# Copyright (c) 2011 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'targets': [
    {
      'target_name': 'chromium_extract',
      'type': 'none',
      'dependencies': [
        # This file is intended to be locally modified. List the targets you use
        # regularly. The generated some.sln will contains projects for only
        # those targets and the targets they are transitively dependent on. This
        # can result in a solution that loads and unloads faster in Visual
        # Studio.
        #
        # Tip: Create a dummy CL to hold your local edits to this file, so they
        # don't accidentally get added to another CL that you are editing.
        #
        # Example:
        #×®ÊµÏÖ¿â
        '../testing/gmock.gyp:gmock',
        #µ¥Ôª²âÊÔ¿â
        '../testing/gtest.gyp:gtest',
        #ÓïÑÔ»ù´¡¿â
        '../base/base.gyp:base',
        #ÓïÑÔ»ù´¡¿â(i18nÀ©Õ¹)
        '../base/base.gyp:base_i18n',
        '../net/net.gyp:net',
        '../net/net.gyp:http_server',
        '../sql/sql.gyp:sql',
        '../ui/base/ui_base.gyp:ui_base',
        '../ui/views/views.gyp:views',
        '../ui/views/examples/examples.gyp:views_examples_exe',
        '../ui/aura/aura.gyp:aura',
        '../ui/aura_extra/aura_extra.gyp:aura_extra',
        '../ui/gfx/gfx.gyp:gfx',
        '../ui/app_list/resources/app_list_resources.gyp:app_list_resources'
      ],
    },
  ],
}
