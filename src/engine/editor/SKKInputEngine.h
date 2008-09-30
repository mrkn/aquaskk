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

#ifndef SKKInputEngine_h
#define SKKInputEngine_h

#include "SKKBaseEditor.h"
#include "SKKContextBuffer.h"
#include "SKKInputMode.h"
#include "SKKInputQueue.h"
#include "SKKComposingEditor.h"
#include "SKKOkuriEditor.h"
#include "SKKCandidateEditor.h"
#include "SKKEntryRemoveEditor.h"
#include "SKKCompleter.h"
#include "SKKSelector.h"
#include <vector>

class SKKRegistrationObserver {
public:
    virtual ~SKKRegistrationObserver() {}

    enum Event { None, Begin, Finish, Abort };
    virtual void SKKRegistrationUpdate(Event event) = 0;
};

class SKKInputEngineOption;
class SKKClipboard;
class SKKFrontEnd;

class SKKInputEngine : public SKKInputQueueObserver,
                       public SKKCompleterBuddy,
                       public SKKSelectorBuddy {
    typedef std::vector<SKKBaseEditor*> EditorStack;

    SKKFrontEnd* frontend_;
    SKKBaseEditor* bottom_;
    SKKInputEngineOption* option_;
    SKKRegistrationObserver* registrationObserver_;
    SKKInputModeSelector inputModeSelector_;
    SKKClipboard* clipboard_;
    bool modified_;
    EditorStack mainStack_;
    EditorStack subStack_;
    EditorStack* active_;
    bool needsInitializeOkuri_;

    SKKInputQueue inputQueue_;
    SKKContextBuffer contextBuffer_;
    std::string word_;

    SKKComposingEditor composingEditor_;
    SKKOkuriEditor okuriEditor_;
    SKKCandidateEditor candidateEditor_;
    SKKEntryRemoveEditor entryRemoveEditor_;

    SKKInputEngine();
    SKKInputEngine(const SKKInputEngine&);
    SKKInputEngine& operator=(const SKKInputEngine&);

    SKKBaseEditor* top() const;
    SKKInputMode inputMode() const;
    void terminate();
    void cancel();
    void enableMainEditor();
    void enableSubEditor(SKKBaseEditor* editor);
    void updateContextBuffer();

    virtual void SKKInputQueueUpdate(const std::string& fixed, const std::string& queue);

    // 見出し語の取得
    virtual const std::string SKKCompleterQueryString();

    // 現在の見出し語の通知
    virtual void SKKCompleterUpdate(const std::string& entry);

    // SKKSelector::Execute() 時に呼び出される
    virtual const SKKEntry SKKSelectorQueryEntry();

    // SKKSelector で現在選択中の候補が変更された場合に呼び出される
    virtual void SKKSelectorUpdate(const SKKCandidate& candidate);

public:
    SKKInputEngine(SKKInputEngineOption* option,
                   SKKRegistrationObserver* registrationObserver,
                   SKKFrontEnd* frontend,
                   SKKInputModeWindow* inputModeWindow,
                   SKKClipboard* clipboard,
                   SKKBaseEditor* bottom);

    // 入力モード
    void SelectInputMode(SKKInputMode mode);

    // 状態変更
    void SetStatePrimary();
    void SetStateComposing();
    void SetStateOkuri();
    void SetStateSelectCandidate();
    void SetStateEntryRemove();

    // 入力
    void HandleChar(char code, bool direct);
    void HandleBackSpace();
    void HandleDelete();
    void HandleCursorLeft();
    void HandleCursorRight();
    void HandleCursorUp();
    void HandleCursorDown();
    void HandlePaste();
    void HandlePing();

    void Commit();
    void Insert(const std::string& str);
    void Reset(bool absolutely = false);

    // トグル変換
    void ToggleKana();
    void ToggleJisx0201Kana();

    // 出力
    void Output();

    // 前回の Output 以降に変更されているかどうか？
    bool IsModified() const;

    // ローマ字かな変換が発生するか？
    bool CanConvert(char code) const;

    // 送りが完成したか？
    bool IsOkuriComplete() const;

    // 再帰的辞書登録
    void BeginRegistration();
    void FinishRegistration();
    void AbortRegistration();

    const SKKEntry Entry() const;
    const std::string Word() const;
};

#endif
