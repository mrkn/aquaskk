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

    [string addAttribute:NSCursorAttributeName value:[NSCursor IBeamCursor] range:cursorPos];

    [client_ setMarkedText:string selectionRange:cursorPos replacementRange:notfound];

    [string release];
}

std::pair<int, int> MacFrontEnd::WindowPosition() const {
    NSRect rect;

    [client_ attributesForCharacterIndex:0 lineHeightRectangle:&rect];

    return std::make_pair<int, int>(rect.origin.x, rect.origin.y);
}

int MacFrontEnd::WindowLevel() const {
    return [client_ windowLevel];
}
