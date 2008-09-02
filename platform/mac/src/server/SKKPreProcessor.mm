/* -*- C++ -*-

   MacOS X implementation of the SKK input method.

   Copyright (C) 2007 Tomotaka SUWA <t.suwa@mac.com>

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

#include <cassert>
#include <iostream>
#include <cctype>
#include <fstream>
#include "SKKPreProcessor.h"
#include "SKKKeyState.h"

SKKPreProcessor::SKKPreProcessor() {}

SKKPreProcessor& SKKPreProcessor::theInstance() {
    static SKKPreProcessor obj;
    return obj;
}

void SKKPreProcessor::Initialize(const std::string& path) {
    keymap_.Initialize(path);
}

SKKEvent SKKPreProcessor::Execute(const NSEvent* event) {
    SKKEvent result;
    int dispchar = *[[event characters] UTF8String];
    int charcode = *[[event charactersIgnoringModifiers] UTF8String];
    int keycode = [event keyCode];
    int mods = 0;
    bool isEisuuOrKanaKey = (keycode == 0x66 || keycode == 0x68);

#ifdef SKK_DEBUG
    NSLog(@"%@", [event description]);
#endif

    // シフト属性が有効なのはデッドキーのみ
    if([event modifierFlags] & NSShiftKeyMask) {
	if(std::isgraph(dispchar)) { // 空白類を除いた英数字記号
	    charcode = dispchar;
	} else {
	    mods += SKKKeyState::SHIFT;
	}
    }

    if([event modifierFlags] & NSControlKeyMask) {
	mods += SKKKeyState::CTRL;
    }

    if([event modifierFlags] & NSAlternateKeyMask) {
	mods += SKKKeyState::ALT;
    }

    if([event modifierFlags] & NSCommandKeyMask) {
	mods += SKKKeyState::META;
    }

    // 英数キー、かなキーの文字コードがスペースのため、0 にする
    if(isEisuuOrKanaKey) {
        charcode = 0x00;
    }

    result = keymap_.Fetch(charcode, keycode, mods);

    // 英数キー、かなキーの場合は強制的に処理済みとする
    if(isEisuuOrKanaKey) {
        result.force_handled = true;
    }

    return result;
}
