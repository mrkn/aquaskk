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

#ifndef SKKEditorStack_h
#define SKKEditorStack_h

#include "SKKBaseEditor.h"
#include "SKKContextBuffer.h"
#include "SKKInputMode.h"
#include <vector>

class SKKRegistrationObserver {
public:
    virtual ~SKKRegistrationObserver() {}

    enum Event { None, Begin, Finish, Abort };
    virtual void SKKRegistrationUpdate(Event event) = 0;
};

class SKKClipboard;
class SKKFrontEnd;
class SKKEvent;

class SKKEditorStack {
    typedef std::vector<SKKBaseEditor*> EditorStack;

    SKKRegistrationObserver* registrationObserver_;
    SKKInputModeSelector inputModeSelector_;
    SKKClipboard* clipboard_;
    bool modified_;
    EditorStack composing_;
    EditorStack sub_;
    EditorStack* active_;

    std::string input_;
    std::string word_;
    SKKContextBuffer buffer_;

    SKKEditorStack();
    SKKEditorStack(const SKKEditorStack&);
    SKKEditorStack& operator=(const SKKEditorStack&);

    SKKBaseEditor* top() const;
    void resetStack();
    void handleInput(char code);
    void collect();

public:
    SKKEditorStack(SKKRegistrationObserver* registrationObserver,
                   SKKInputModeWindow* inputModeWindow,
                   SKKClipboard* clipboard,
                   SKKBaseEditor* bottom);

    // 入力モード
    void SelectInputMode(SKKInputMode mode);
    SKKInputMode InputMode() const;
    void ActivateInputMode();

    // 入力
    void Input(const SKKEvent& event);
    void Paste(const std::string& str);
    void Paste();
    void Commit();
    void Cancel();
    void Reset();

    // トグル変換
    void ToggleKana();
    void ToggleJisx0201Kana();

    // 出力
    void Output(SKKFrontEnd* frontend);

    // バッファが空かどうか？
    bool IsEmpty() const;

    // 前回の Output 以降に変更されているかどうか？
    bool IsModified() const;

    // ローマ字かな変換が発生するか？
    bool WillConvert(const SKKEvent& event) const;
    
    // スタック操作
    void PushEditor(SKKBaseEditor* editor);
    void PopEditor();

    void EnableSubEditor(SKKBaseEditor* editor);
    void DisableSubEditor();

    // 再帰的辞書登録
    void BeginRegistration();
    void FinishRegistration();
    void AbortRegistration();

    void FixEntry();

    const SKKEntry Entry() const;
    const SKKCandidate Candidate() const;
    const std::string Word() const;
};

#endif
