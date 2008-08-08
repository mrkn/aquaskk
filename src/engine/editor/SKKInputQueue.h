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

#ifndef SKKInputQueue_h
#define SKKInputQueue_h

#include <string>
#include "SKKInputMode.h"

class SKKInputQueueObserver {
public:
    virtual ~SKKInputQueueObserver() {}

    virtual void SKKInputQueueUpdate(const std::string& fixed) = 0;
    virtual void SKKInputQueueUpdate(const std::string& fixed, const std::string& queue) = 0;
};

class SKKInputQueue {
    SKKInputQueueObserver* observer_;
    SKKInputMode mode_;
    std::string queue_;

    void convert(char code, std::string& fixed, bool terminate = false);
    void notify(const std::string& fixed = "");

public:
    SKKInputQueue(SKKInputQueueObserver* observer);

    // 入力モードを変更する
    void SelectInputMode(SKKInputMode mode);

    // 文字の追加と削除
    void AddChar(char code);
    void RemoveChar();

    void Terminate();
    void Clear();

    bool IsEmpty() const;

    // 変換可能かどうか
    bool CanConvert(char code) const;
};

#endif
