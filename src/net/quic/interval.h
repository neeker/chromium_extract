// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// An Interval<T> is a data structure used to represent a contiguous, mutable
// range over an ordered type T. Supported operations include testing a value to
// see whether it is included in the interval, comparing two intervals, and
// performing their union, intersection, and difference. For the purposes of
// this library, an "ordered type" is any type that induces a total order on its
// values via its less-than operator (operator<()). Examples of such types are
// basic arithmetic types like int and double as well as class types like
// string.
//
// An Interval<T> is represented using the usual C++ STL convention, namely as
// the half-open interval [min, max). A point p is considered to be contained in
// the interval iff p >= min && p < max. One consequence of this definition is
// that for any non-empty interval, min is contained in the interval but max is
// not. There is no canonical representation for the empty interval; rather, any
// interval where max <= min is regarded as empty. As a consequence, two empty
// intervals will still compare as equal despite possibly having different
// underlying min() or max() values. Also beware of the terminology used here:
// the library uses the terms "min" and "max" rather than "begin" and "end" as
// is conventional for the STL.
//
// T is required to be default- and copy-constructable, to have an assignment
// operator, and the full complement of comparison operators (<, <=, ==, !=, >=,
// >).  A difference operator (operator-()) is required if Interval<T>::Length
// is used.
//
// For equality comparisons, Interval<T> supports an Equals() method and an
// operator==() which delegates to it. Two intervals are considered equal if
// either they are both empty or if their corresponding min and max fields
// compare equal. For ordered comparisons, Interval<T> also provides the
// comparator Interval<T>::Less and an operator<() which delegates to it.
// Unfortunately this comparator is currently buggy because its behavior is
// inconsistent with Equals(): two empty ranges with different representations
// may be regarded as equivalent by Equals() but regarded as different by
// the comparator. Bug 9240050 has been created to address this.
//
// This class is thread-compatible if T is thread-compatible. (See
// go/thread-compatible).
//
// Examples:
//   Interval<int> r1(0, 100);  // The interval [0, 100).
//   EXPECT_TRUE(r1.Contains(0));
//   EXPECT_TRUE(r1.Contains(50));
//   EXPECT_FALSE(r1.Contains(100));  // 100 is just outside the interval.
//
//   Interval<int> r2(50, 150);  // The interval [50, 150).
//   EXPECT_TRUE(r1.Intersects(r2));
//   EXPECT_FALSE(r1.Contains(r2));
//   EXPECT_TRUE(r1.IntersectWith(r2));  // Mutates r1.
//   EXPECT_EQ(Interval<int>(50, 100), r1);  // r1 is now [50, 100).
//
//   Interval<int> r3(1000, 2000);  // The interval [1000, 2000).
//   EXPECT_TRUE(r1.IntersectWith(r3));  // Mutates r1.
//   EXPECT_TRUE(r1.Empty());  // Now r1 is empty.
//   EXPECT_FALSE(r1.Contains(r1.min()));  // e.g. doesn't contain its own min.

#ifndef NET_QUIC_INTERVAL_H_
#define NET_QUIC_INTERVAL_H_

#include <stddef.h>
#include <algorithm>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace net {

template <typename T>
class Interval {
 private:
// TODO(rtenneti): Implement after suupport for std::decay.
#if 0
  // Type trait for deriving the return type for Interval::Length.  If
  // operator-() is not defined for T, then the return type is void.  This makes
  // the signature for Length compile so that the class can be used for such T,
  // but code that calls Length would still generate a compilation error.
  template <typename U>
  class DiffTypeOrVoid {
   private:
    template <typename V>
    static auto f(const V* v) -> decltype(*v - *v);
    template <typename V>
    static void f(...);

   public:
    using type = typename std::decay<decltype(f<U>(0))>::type;
  };
#endif

 public:
  // Compatibility alias.
  using Less = std::less<Interval>;

  // Construct an Interval representing an empty interval.
  Interval() : min_(), max_() {}

  // Construct an Interval representing the interval [min, max). If min < max,
  // the constructed object will represent the non-empty interval containing all
  // values from min up to (but not including) max. On the other hand, if min >=
  // max, the constructed object will represent the empty interval.
  Interval(const T& min, const T& max) : min_(min), max_(max) {}

#if defined(OS_WIN)
  (const T& min)() const { return min_; }
#else
  const T& min() const { return min_; }
#endif
#if defined(OS_WIN)
  (const T& max)() const { return max_; }
#else
  const T& max() const { return max_; }
#endif
  void SetMin(const T& t) { min_ = t; }
  void SetMax(const T& t) { max_ = t; }

  void Set(const T& min, const T& max) {
    SetMin(min);
    SetMax(max);
  }

  void Clear() { *this = {}; }
  void CopyFrom(const Interval& i) { *this = i; }
  bool Equals(const Interval& i) const { return *this == i; }
#if defined(OS_WIN)
  bool Empty() const { return (min)() >= (max)(); }
#else
  bool Empty() const { return min() >= max(); }
#endif

  // Returns the length of this interval. The value returned is zero if
  // IsEmpty() is true; otherwise the value returned is max() - min().
  const T Length() const { return (min_ >= max_ ? min_ : max_) - min_; }

  // Returns true iff t >= min() && t < max().
#if defined(OS_WIN)
  bool Contains(const T& t) const { return (min)() <= t && (max)() > t; }
#else
  bool Contains(const T& t) const { return min() <= t && max() > t; }
#endif
  // Returns true iff *this and i are non-empty, and *this includes i. "*this
  // includes i" means that for all t, if i.Contains(t) then this->Contains(t).
  // Note the unintuitive consequence of this definition: this method always
  // returns false when i is the empty interval.
  bool Contains(const Interval& i) const {
#if defined(OS_WIN)
    return !Empty() && !i.Empty() && (min)() <= i.(min)() && (max)() >= i.(max)();
#else
    return !Empty() && !i.Empty() && min() <= i.min() && max() >= i.max();
#endif
  }

  // Returns true iff there exists some point t for which this->Contains(t) &&
  // i.Contains(t) evaluates to true, i.e. if the intersection is non-empty.
  bool Intersects(const Interval& i) const {
#if defined(OS_WIN)
    return !Empty() && !i.Empty() && (min)() < (i.max)() && (max)() > (i.min)();
#else
    return !Empty() && !i.Empty() && min() < i.max() && max() > i.min();
#endif
  }

  // Returns true iff there exists some point t for which this->Contains(t) &&
  // i.Contains(t) evaluates to true, i.e. if the intersection is non-empty.
  // Furthermore, if the intersection is non-empty and the intersection pointer
  // is not null, this method stores the calculated intersection in
  // *intersection.
  bool Intersects(const Interval& i, Interval* out) const;

  // Sets *this to be the intersection of itself with i. Returns true iff
  // *this was modified.
  bool IntersectWith(const Interval& i);

  // Calculates the smallest interval containing both *this i, and updates *this
  // to represent that interval, and returns true iff *this was modified.
  bool SpanningUnion(const Interval& i);

  // Determines the difference between two intervals by finding all points that
  // are contained in *this but not in i, coalesces those points into the
  // largest possible contiguous intervals, and appends those intervals to the
  // *difference vector. Intuitively this can be thought of as "erasing" i from
  // *this. This will either completely erase *this (leaving nothing behind),
  // partially erase some of *this from the left or right side (leaving some
  // residual behind), or erase a hole in the middle of *this (leaving behind an
  // interval on either side). Therefore, 0, 1, or 2 intervals will be appended
  // to *difference. The method returns true iff the intersection of *this and i
  // is non-empty. The caller owns the vector and the Interval* pointers
  // inside it. The difference vector is required to be non-null.
  bool Difference(const Interval& i, std::vector<Interval*>* difference) const;

  // Determines the difference between two intervals as in
  // Difference(Interval&, vector*), but stores the results directly in out
  // parameters rather than dynamically allocating an Interval* and appending
  // it to a vector. If two results are generated, the one with the smaller
  // value of min() will be stored in *lo and the other in *hi. Otherwise (if
  // fewer than two results are generated), unused arguments will be set to the
  // empty interval (it is possible that *lo will be empty and *hi non-empty).
  // The method returns true iff the intersection of *this and i is non-empty.
  bool Difference(const Interval& i, Interval* lo, Interval* hi) const;

  friend bool operator==(const Interval& a, const Interval& b) {
    bool ae = a.Empty();
    bool be = b.Empty();
    if (ae && be)
      return true;  // All empties are equal.
    if (ae != be)
      return false;  // Empty cannot equal nonempty.
#if defined(OS_WIN)
    return (a.min)() == (b.min)() && (a.max)() == (b.max)();
#else
    return a.min() == b.min() && a.max() == b.max();
#endif
  }

  friend bool operator!=(const Interval& a, const Interval& b) {
    return !(a == b);
  }

  // Defines a comparator which can be used to induce an order on Intervals, so
  // that, for example, they can be stored in an ordered container such as
  // std::set. The ordering is arbitrary, but does provide the guarantee that,
  // for non-empty intervals X and Y, if X contains Y, then X <= Y.
  // TODO(kosak): The current implementation of this comparator has a problem
  // because the ordering it induces is inconsistent with that of Equals(). In
  // particular, this comparator does not properly consider all empty intervals
  // equivalent. Bug b/9240050 has been created to track this.
  friend bool operator<(const Interval& a, const Interval& b) {
#if defined(OS_WIN)
    return (a.min)() < (b.min)() || ((a.min)() == (b.min)() && (a.max)() > (b.max)());
#else
    return a.min() < b.min() || (a.min() == b.min() && a.max() > b.max());
#endif
  }

  friend std::ostream& operator<<(std::ostream& out, const Interval& i) {
#if defined(OS_WIN)
    return out << "[" << (i.min)() << ", " << (i.max)() << ")";
#else
    return out << "[" << i.min() << ", " << i.max() << ")";
#endif
  }

 private:
  T min_;  // Inclusive lower bound.
  T max_;  // Exclusive upper bound.
};

//==============================================================================
// Implementation details: Clients can stop reading here.

template <typename T>
bool Interval<T>::Intersects(const Interval& i, Interval* out) const {
  if (!Intersects(i))
    return false;
  if (out != nullptr) {
#if defined(OS_WIN)
    *out = Interval((std::max)((min)(), (i.min)()), (std::min)((max)(), (i.max)()));
#else
    *out = Interval(std::max(min(), i.min()), std::min(max(), i.max()));
#endif
  }
  return true;
}

template <typename T>
bool Interval<T>::IntersectWith(const Interval& i) {
  if (Empty())
    return false;
  bool modified = false;
#if defined(OS_WIN)
  if ((i.min)() > (min)()) {
    SetMin((i.min)());
    modified = true;
  }
  if ((i.max)() < (max)()) {
    SetMax((i.max)());
    modified = true;
  }
#else
  if (i.min() > min()) {
    SetMin(i.min());
    modified = true;
  }
  if (i.max() < max()) {
    SetMax(i.max());
    modified = true;
  }
#endif
  return modified;
}

template <typename T>
bool Interval<T>::SpanningUnion(const Interval& i) {
  if (i.Empty())
    return false;
  if (Empty()) {
    *this = i;
    return true;
  }
  bool modified = false;
#if defined(OS_WIN)
  if ((i.min)() < (min)()) {
#else
  if (i.min() < min()) {
#endif
#if defined(OS_WIN)
    SetMin((i.min)());
#else
    SetMin(i.min());
#endif
    modified = true;
  }
#if defined(OS_WIN)
  if ((i.max)() > (max)()) {
    SetMax((i.max)());
#else
  if (i.max() > max()) {
    SetMax(i.max());
#endif
    modified = true;
  }
  return modified;
}

template <typename T>
bool Interval<T>::Difference(const Interval& i,
                             std::vector<Interval*>* difference) const {
  if (Empty()) {
    // <empty> - <i> = <empty>
    return false;
  }
  if (i.Empty()) {
    // <this> - <empty> = <this>
    difference->push_back(new Interval(*this));
    return false;
  }
#if defined(OS_WIN)
  if ((min)() < (i.max)() && (min)() >= (i.min)() && (max)() > (i.max)()) {
    //            [------ this ------)
    // [------ i ------)
    //                 [-- result ---)
    difference->push_back(new Interval((i.max)(), (max)()));
    return true;
  }
  if ((max)() > (i.min)() && (max)() <= (i.max)() && (min)() < (i.min)()) {
    // [------ this ------)
    //            [------ i ------)
    // [- result -)
    difference->push_back(new Interval((min)(), (i.min)()));
    return true;
  }
  if ((min)() < (i.min)() && (max)() > (i.max)()) {
    // [------- this --------)
    //      [---- i ----)
    // [ R1 )           [ R2 )
    // There are two results: R1 and R2.
    difference->push_back(new Interval((min)(), (i.min)()));
    difference->push_back(new Interval((i.max)(), (max)()));
    return true;
  }
  if ((min)() >= (i.min)() && (max)() <= (i.max)()) {
    //   [--- this ---)
    // [------ i --------)
    // Intersection is <this>, so difference yields the empty interval.
    // Nothing is appended to *difference.
    return true;
  }
#else
  if (min() < i.max() && min() >= i.min() && max() > i.max()) {
    //            [------ this ------)
    // [------ i ------)
    //                 [-- result ---)
    difference->push_back(new Interval(i.max(), max()));
    return true;
  }
  if (max() > i.min() && max() <= i.max() && min() < i.min()) {
    // [------ this ------)
    //            [------ i ------)
    // [- result -)
    difference->push_back(new Interval(min(), i.min()));
    return true;
  }
  if (min() < i.min() && max() > i.max()) {
    // [------- this --------)
    //      [---- i ----)
    // [ R1 )           [ R2 )
    // There are two results: R1 and R2.
    difference->push_back(new Interval(min(), i.min()));
    difference->push_back(new Interval(i.max(), max()));
    return true;
  }
  if (min() >= i.min() && max() <= i.max()) {
    //   [--- this ---)
    // [------ i --------)
    // Intersection is <this>, so difference yields the empty interval.
    // Nothing is appended to *difference.
    return true;
  }
#endif
  // No intersection. Append <this>.
  difference->push_back(new Interval(*this));
  return false;
}

template <typename T>
bool Interval<T>::Difference(const Interval& i,
                             Interval* lo,
                             Interval* hi) const {
  // Initialize *lo and *hi to empty
  *lo = {};
  *hi = {};
  if (Empty())
    return false;
  if (i.Empty()) {
    *lo = *this;
    return false;
  }
#if defined(OS_WIN)
  if ((min)() < (i.max)() && (min)() >= (i.min)() && (max)() > (i.max)()) {
    //            [------ this ------)
    // [------ i ------)
    //                 [-- result ---)
    *hi = Interval((i.max)(), (max)());
    return true;
  }
  if ((max)() > (i.min)() && (max)() <= (i.max)() && (min)() < (i.min)()) {
    // [------ this ------)
    //            [------ i ------)
    // [- result -)
    *lo = Interval((min)(), (i.min)());
    return true;
  }
  if ((min)() < (i.min)() && (max)() > (i.max)()) {
    // [------- this --------)
    //      [---- i ----)
    // [ R1 )           [ R2 )
    // There are two results: R1 and R2.
    *lo = Interval((min)(), (i.min)());
    *hi = Interval((i.max)(), (max)());
    return true;
  }
  if ((min)() >= (i.min)() && (max)() <= (i.max)()) {
    //   [--- this ---)
    // [------ i --------)
    // Intersection is <this>, so difference yields the empty interval.
    return true;
  }
#else
  if (min() < i.max() && min() >= i.min() && max() > i.max()) {
    //            [------ this ------)
    // [------ i ------)
    //                 [-- result ---)
    *hi = Interval(i.max(), max());
    return true;
  }
  if (max() > i.min() && max() <= i.max() && min() < i.min()) {
    // [------ this ------)
    //            [------ i ------)
    // [- result -)
    *lo = Interval(min(), i.min());
    return true;
  }
  if (min() < i.min() && max() > i.max()) {
    // [------- this --------)
    //      [---- i ----)
    // [ R1 )           [ R2 )
    // There are two results: R1 and R2.
    *lo = Interval(min(), i.min());
    *hi = Interval(i.max(), max());
    return true;
  }
  if (min() >= i.min() && max() <= i.max()) {
    //   [--- this ---)
    // [------ i --------)
    // Intersection is <this>, so difference yields the empty interval.
    return true;
  }
#endif
  *lo = *this;  // No intersection.
  return false;
}

}  // namespace net

#endif  // NET_QUIC_INTERVAL_H_
