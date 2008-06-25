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

#ifndef SKKCompleter_h
#define SKKCompleter_h

#include <string>
#include <vector>

// 補完サポートクラス
struct SKKCompleterBuddy {
    virtual ~SKKCompleterBuddy() {}

    // 見出し語の取得
    virtual const std::string SKKCompleterQueryString() const = 0;

    // 現在の見出し語の通知
    virtual void SKKCompleterUpdate(const std::string& entry) = 0;
};

class SKKCompleter {
    SKKCompleterBuddy* buddy_;
    std::vector<std::string> completions_;
    int pos_;

    bool complete();
    void notify();
    int minPosition() const;
    int maxPosition() const;

public:
    SKKCompleter(SKKCompleterBuddy* buddy);

    // 見出し語の補完
    bool Execute();

    // 移動
    void Next();
    void Prev();
};

#endif

