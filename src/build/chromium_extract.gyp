# Copyright (c) 2011 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'targets': [
    {
      'target_name': 'chromium_extract',
      'type': 'none',
      'dependencies': [
        #桩实现库（goolge gmock）
        '../testing/gmock.gyp:gmock',
        #单元测试库（google gtest）
        '../testing/gtest.gyp:gtest',
        #内存管理
        '../base/allocator/allocator.gyp:allocator',
        #语言基础库
        '../base/base.gyp:base',
        #语言基础库（i18n）
        '../base/base.gyp:base_i18n',
        #网络库
        '../net/net.gyp:net',
        #http服务器实现库
        '../net/net.gyp:http_server',
        #SQLite（加上了SQLCipher实现）
        '../third_party/sqlite/sqlite.gyp:sqlite',
        #SQLite封装库
        '../sql/sql.gyp:sql',
        #UI基础库
        '../ui/base/ui_base.gyp:ui_base',
        #UI视图组件库
        '../ui/views/views.gyp:views',
        #UI视图组件示例
        '../ui/views/examples/examples.gyp:views_examples_exe',
        #UI皮肤库（google aura）
        '../ui/aura/aura.gyp:aura',
        '../ui/aura_extra/aura_extra.gyp:aura_extra',
        #UI绘制库（google gfx）
        '../ui/gfx/gfx.gyp:gfx',
        #UI资源库
        '../ui/app_list/resources/app_list_resources.gyp:app_list_resources',
        #加密库工具（依赖third_party/boringssl，一个OpenSSL变种，提升了速度）
        '../crypto/crypto.gyp:crypto',
        #boringssl库（openssl替代）
        '../third_party/boringssl/boringssl.gyp:boringssl',
        #打印库
        '../printing/printing.gyp:printing',
        #错误报告库转储处理库（google breakpad）
        '../breakpad/breakpad.gyp:breakpad_handler',
        #错误报告上传库（google breakpad）
        '../breakpad/breakpad.gyp:breakpad_sender',
      ],
    },
  ],
}
