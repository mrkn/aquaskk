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

#include "CompletionWindow.h"

MacFrontEnd::MacFrontEnd(id client) : client_(client) {}

void MacFrontEnd::InsertString(const std::string& str) {
    NSString* string = [NSString stringWithUTF8String:str.c_str()];

    [client_ insertText:string replacementRange:notFound()];
}

void MacFrontEnd::ComposeString(const std::string& str, int cursorOffset) {
    NSMutableAttributedString* marked = createMarkedText(str, cursorOffset);
    NSRange cursorPos = NSMakeRange([marked length] + cursorOffset, 0);

    // *** FIXME ***
    // Carbon アプリで見出し語を入力すると、なぜか文字のベースラインが下にずれる
    // 一旦 "▽" だけ入力すると回避できるが、正解かどうかは不明
    if(utf8::length(str) == 2 && str.find("▽") == 0) {
        [client_ setMarkedText:@"▽" selectionRange:notFound() replacementRange:notFound()];
    }

    [client_ setMarkedText:marked selectionRange:cursorPos replacementRange:notFound()];

    [[CompletionWindow sharedWindow] hide];

    [marked release];
}

void MacFrontEnd::ShowCompletion(const std::string& completion, int cursorOffset) {
    CompletionWindow* window = [CompletionWindow sharedWindow];
    NSString* compString = [NSString stringWithUTF8String:completion.c_str()];
    NSRect rect;

    [client_ attributesForCharacterIndex:cursorOffset + 1 lineHeightRectangle:&rect];

    [window showCompletion:compString at:rect.origin level:WindowLevel()];
}

void MacFrontEnd::HideCompletion() {
    [[CompletionWindow sharedWindow] hide];
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

// ------------------------------------------------------------

NSRange MacFrontEnd::notFound() const {
    return NSMakeRange(NSNotFound, NSNotFound);
}

NSMutableAttributedString* MacFrontEnd::createMarkedText(const std::string& str, int cursorOffset) {
    NSString* source = [NSString stringWithUTF8String:str.c_str()];
    NSMutableAttributedString* marked = [[NSMutableAttributedString alloc] initWithString:source];

    //[marked beginEditing];
    [marked addAttribute:NSCursorAttributeName
            value:[NSCursor IBeamCursor] range:NSMakeRange([marked length] + cursorOffset, 0)];

#if 0
    [marked addAttribute:NSForegroundColorAttributeName
            value:[NSColor redColor] range:NSMakeRange(0, [marked length])];
#endif

#if 0
    [marked addAttribute:NSMarkedClauseSegmentAttributeName
            value:[NSNumber numberWithInt:1] range:NSMakeRange(0, [marked length])];
#endif

#if 1
    [marked addAttribute:NSUnderlineColorAttributeName
            value:[NSColor blackColor] range:NSMakeRange(0, [marked length])];
#endif

#if 1
    [marked addAttribute:NSUnderlineStyleAttributeName
            value:[NSNumber numberWithInt:NSUnderlineStyleDouble] range:NSMakeRange(0, [marked length])];
#endif

#if 0
    [marked addAttribute:NSUnderlineStyleAttributeName
            value:[NSNumber numberWithInt:NSUnderlinePatternDot]
            range:NSMakeRange(0, [marked length])];
#endif

#if 1
    [marked addAttribute:NSKernAttributeName
            value:[NSNumber numberWithFloat:1.0] range:NSMakeRange([marked length], 0)];
#endif

    //[marked endEditing];

    return marked;
}
