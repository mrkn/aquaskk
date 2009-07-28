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
    NSString* string = @"";

    if(!str.empty()) {
        string = [NSString stringWithUTF8String:str.c_str()];
    }

    [client_ insertText:string replacementRange:notFound()];
}

void MacFrontEnd::ComposeString(const std::string& str, int cursorOffset) {
    composing_ = str;

    NSMutableAttributedString* marked = createMarkedText(str, cursorOffset);
    NSRange cursorPos = NSMakeRange([marked length] + cursorOffset, 0);

    // *** FIXME ***
    // Carbon アプリで見出し語を入力すると、なぜか文字のベースラインが下にずれる
    // 一旦 "▽" だけ入力すると回避できるが、正解かどうかは不明
    if(utf8::length(str) == 2 && str.find("▽") == 0) {
        [client_ setMarkedText:@"▽" selectionRange:notFound() replacementRange:notFound()];
    }

    [client_ setMarkedText:marked selectionRange:cursorPos replacementRange:notFound()];

    [marked release];
}

void MacFrontEnd::Clear() {
    // 未確定文字列がある時だけクリアーする
    // Photoshop Elements の問題を回避
    if(!composing_.empty()) {
        ComposeString("");
    }
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

std::string MacFrontEnd::SelectedString() {
    NSRange range = [client_ selectedRange];
    NSAttributedString* text = [client_ attributedSubstringFromRange:range];

    if(text) {
        return [[text string] UTF8String];
    }

    return "";
}

// ------------------------------------------------------------

NSRange MacFrontEnd::notFound() const {
    return NSMakeRange(NSNotFound, NSNotFound);
}

NSMutableAttributedString* MacFrontEnd::createMarkedText(const std::string& str, int cursorOffset) {
    NSString* source = [NSString stringWithUTF8String:str.c_str()];
    NSMutableAttributedString* marked = [[NSMutableAttributedString alloc] initWithString:source];

    [marked addAttribute:NSCursorAttributeName
            value:[NSCursor IBeamCursor] range:NSMakeRange([marked length] + cursorOffset, 0)];

    [marked addAttribute:NSUnderlineStyleAttributeName
            value:[NSNumber numberWithInt:NSUnderlineStyleThick] range:NSMakeRange(0, [marked length])];

    return marked;
}
