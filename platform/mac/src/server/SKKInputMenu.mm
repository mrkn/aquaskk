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

#include "SKKInputMenu.h"
#include "SKKEvent.h"

@implementation SKKInputMenu

namespace {
    const struct {
        int event_id;
        SKKInputMode mode;
        NSString* identifier;
    } table[] = {
        { SKK_HIRAKANA_MODE, 		HirakanaInputMode,
          @"com.apple.inputmethod.Japanese.Hiragana" },
        { SKK_KATAKANA_MODE, 		KatakanaInputMode,
          @"com.apple.inputmethod.Japanese.Katakana" },
        { SKK_JISX0201KANA_MODE,	Jisx0201KanaInputMode,
          @"com.apple.inputmethod.Japanese.HalfWidthKana" },
        { SKK_JISX0208LATIN_MODE,	Jisx0208LatinInputMode,
          @"com.apple.inputmethod.Japanese.FullWidthRoman" },
        { SKK_ASCII_MODE,		AsciiInputMode,
          @"com.apple.inputmethod.Roman" },
        { -1,				InvalidInputMode,
          0 }
    };
};

- (id)initWithClient:(id)client {
    if(self = [super init]) {
        client_ = client;
    }

    return self;
}

- (void)updateMenu:(SKKInputMode)mode {
    NSString* identifer = [self modeIdentifier:mode];

    if(identifer) {
        [client_ selectInputMode:identifer];
    }
}

- (int)eventId:(NSString*)identifier {
    for(int i = 0; table[i].identifier != 0; ++ i) {
        if([identifier compare:table[i].identifier] == 0) {
            return table[i].event_id;
        }
    }

    return 0;
}

- (NSString*)modeIdentifier:(SKKInputMode)mode {
    for(int i = 0; table[i].identifier != 0; ++ i) {
        if(table[i].mode == mode) {
            return table[i].identifier;
        }
    }

    return 0;
}

@end    
