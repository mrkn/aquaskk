/* -*- C++ -*-

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

#ifndef SKKBaseEditor_h
#define SKKBaseEditor_h

#include <string>

class SKKContextBuffer;

class SKKBaseEditor {
public:
    virtual ~SKKBaseEditor() {}

    enum Event {
        BackSpace,
        Delete,
        CursorLeft,
        CursorRight,
        CursorUp,
        CursorDown
    };

    // 入力処理(ASCII 専用)
    virtual void Input(const std::string& ascii) {}

    // 入力処理(fixed=確定文字列, input=入力文字列, code=入力文字)
    virtual void Input(const std::string& fixed, const std::string& input, char code) {}

    // 入力処理(event=イベント)
    virtual void Input(Event event) {}

    // クリアー
    virtual void Clear() = 0;

    // 出力処理
    virtual void Output(SKKContextBuffer& buffer) const = 0;

    // 確定処理
    virtual void Commit(std::string& queue) = 0;

    // 出力完了処理
    virtual void Flush() {}

    // 前回の Flush 以降に変更されているかどうか？
    virtual bool IsModified() const { return true; }
};

#endif
