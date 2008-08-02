/* -*- ObjC -*-

  MacOS X implementation of the SKK input method.

  Copyright (C) 2008 Tomotaka SUWA <t.suwa@mac.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include "SKKConstVars.h"
#include "ObjCUtil.h"

namespace SKKUserDefaultKeys {
#define DECLARE_UserDefaultsKey(key) NSString* key = @ #key

    DECLARE_UserDefaultsKey(suppress_newline_on_commit);
    DECLARE_UserDefaultsKey(use_numeric_conversion);
    DECLARE_UserDefaultsKey(enable_extended_completion);
    DECLARE_UserDefaultsKey(keyboard_layout);
    DECLARE_UserDefaultsKey(fix_intermediate_conversion);
    DECLARE_UserDefaultsKey(max_count_of_inline_candidates);
    DECLARE_UserDefaultsKey(candidate_window_labels);
    DECLARE_UserDefaultsKey(candidate_window_font_name);
    DECLARE_UserDefaultsKey(candidate_window_font_size);
    DECLARE_UserDefaultsKey(enable_skkserv);
    DECLARE_UserDefaultsKey(skkserv_localonly);
    DECLARE_UserDefaultsKey(skkserv_port);
    DECLARE_UserDefaultsKey(openlab_host);
    DECLARE_UserDefaultsKey(openlab_path);
    DECLARE_UserDefaultsKey(user_dictionary_path);

#undef DECLARE_UserDefaultsKey
}

namespace SKKFilePaths {
    static NSString* pathForApplicationSupport() {
        ObjC::RAIIPool pool;
        static NSString* path = [[NSString stringWithFormat:@"%@/Library/Application Support/AquaSKK",
                                           NSHomeDirectory()] retain];

        return path;
    }

    static NSString* pathForDictionarySet() {
        ObjC::RAIIPool pool;
        static NSString* path = [[NSString stringWithFormat:@"%@/DictionarySet.plist",
                                           pathForApplicationSupport()] retain];

        return path;
    }

    static NSString* pathForUserDefaults() {
        ObjC::RAIIPool pool;
        const char* plist = "Library/Preferences/jp.sourceforge.inputmethod.aquaskk.plist";
        static NSString* path = [[NSString stringWithFormat:@"%@/%s",
                                           NSHomeDirectory(), plist] retain];

        return path;
    }

    NSString* ApplicationSupportFolder = pathForApplicationSupport();
    NSString* DictionarySet = pathForDictionarySet();
    NSString* UserDefaults = pathForUserDefaults();
}
