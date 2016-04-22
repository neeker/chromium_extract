chromium基础库抽取包
=========================

## 基础库说明

### base

基础的工具类库，解决一些需要不得不处理的语言、编译器或操作系统
细节而导致的比必要的重复代码工作。

同时也替代一些C++本身不提供的或STL兼容性处理得不好的实现。

总之用了它你很爽！

### net

基础的网络操作工具函数库。

### brotli 

无损压缩算法，压缩数据使用LZ77算法(实现变种)。

### sfntly

是一个字体工具包，可用于使用、编辑和创建基于 SFNT (http://en.wikipedia.org/wiki/Sfnt) 的字体。

### skia

2D向量图形处理函数库，包含字型、座标转换，以及点阵图都有高效能且简洁的表现。

### modp_b64

BASE64编解码库。

### harfbuzz-ng

Harfbuzz是一个OpenType的构图引擎[](http://www.freedesktop.org/wiki/Software/HarfBuzz)。

目前Harfbuzz有两个实现树，早期的那个称之为Harfbuzz，或者old harfbuzz。

而新的目前还处于比较活跃的开发状态的这个代码树，在整个代码的结构上面与前面有着非常大的区别，
为了区分新的这个称之为harfbuzz-ng。

### event

### gfx

Chromium专用的图形和渲染相关的助手代码。

### breakpad

专门用于崩溃后处理的库。

### boringssl

OpenSSL的一个分支（具体还不清楚有什么优化或不同？）

### url_lib

URL处理库

### crypto

加解密相关库。

### sdch

http的sdch压缩客户端实现库。

### sql

基于SQLite3的封装库。

### testing

单元测试相关的工具代码（gtest）。

### v8

V8引擎库。


## 编译指南

### Windows

**环境要求**

1. VS2013
2. python2.7

**执行命令**

    cd src
    python
    python build\gyp_chromium build\chromium_extract.gyp
    ninja -C out\Debug chromium_extract
    ninja -C out\Release chromium_extract
    ninja -C out\Debug_x64 chromium_extract
    ninja -C out\Release_x64 chromium_extract









