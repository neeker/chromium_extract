chromium基础库抽取包
=========================

## 基础库说明

### base

基础的工具类库，解决一些需要不得不处理的语言、编译器或操作系统
细节而导致的比必要的重复代码工作。

同时也替代一些C++本身不提供的或STL兼容性处理得不好的实现。

总之用了它你很爽！

### net

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









