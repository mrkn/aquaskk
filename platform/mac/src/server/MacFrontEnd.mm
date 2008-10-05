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

#include <iostream>
#include "MacFrontEnd.h"
#include "utf8util.h"

MacFrontEnd::MacFrontEnd(id client) : client_(client) {}

void MacFrontEnd::InsertString(const std::string& str) {
    NSString* string = [NSString stringWithUTF8String:str.c_str()];
    NSRange notfound = NSMakeRange(NSNotFound, NSNotFound);

    [client_ insertText:string replacementRange:notfound];
}

void MacFrontEnd::ComposeString(const std::string& str, int cursorOffset) {
    NSMutableAttributedString* string = [NSMutableAttributedString alloc];
    [string initWithString:[NSString stringWithUTF8String:str.c_str()]];

    NSRange cursorPos = NSMakeRange([string length] + cursorOffset, 0);
    NSRange notfound = NSMakeRange(NSNotFound, NSNotFound);

    // *** FIXME ***
    // Carbon アプリで見出し語を入力すると、なぜか文字のベースラインが下にずれる
    // 一旦 "▽" だけ入力すると回避できるが、正解かどうかは不明
    if(utf8::length(str) == 2 && str.find("▽") == 0) {
        [client_ setMarkedText:@"▽" selectionRange:notfound replacementRange:notfound];
    }

    [string addAttribute:NSCursorAttributeName value:[NSCursor IBeamCursor] range:cursorPos];

    [client_ setMarkedText:string selectionRange:cursorPos replacementRange:notfound];

    [string release];
}

void MacFrontEnd::Clear() {
    ComposeString("");
}

std::pair<int, int> MacFrontEnd::WindowPosition() const {
    NSRect rect;

    [client_ attributesForCharacterIndex:0 lineHeightRectangle:&rect];

#ifdef SKK_DEBUG
    BOOL markedRange = NO;
    NSInteger pos = [client_ characterIndexForPoint:rect.origin
                             tracking:kIMKNearestBoundaryMode inMarkedRange:&markedRange];

    NSLog(@"pos=%d, marked=%d, origin=(%f, %f), size=(%f, %f)",
          pos, markedRange, rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
#endif

    return std::make_pair<int, int>(rect.origin.x, rect.origin.y);
}

int MacFrontEnd::WindowLevel() const {
    return [client_ windowLevel];
}
