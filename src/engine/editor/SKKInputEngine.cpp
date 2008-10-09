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

#include "SKKInputEngine.h"
#include "SKKInputEngineOption.h"
#include "SKKInputMode.h"
#include "SKKClipboard.h"
#include "SKKContextBuffer.h"
#include "SKKBackEnd.h"
#include "jconv.h"
#include <iostream>
#include <cctype>

SKKInputEngine::SKKInputEngine(SKKInputEngineOption* option,
                               SKKRegistrationObserver* registrationObserver,
                               SKKFrontEnd* frontend,
                               SKKInputModeWindow* inputModeWindow,
                               SKKClipboard* clipboard,
                               SKKBaseEditor* bottom)
    : frontend_(frontend),
      bottom_(bottom),
      option_(option),
      registrationObserver_(registrationObserver),
      inputModeSelector_(inputModeWindow),
      clipboard_(clipboard),
      modified_(false),
      inputQueue_(this) {
    SetStatePrimary();
}

void SKKInputEngine::SelectInputMode(SKKInputMode mode) {
    inputModeSelector_.Select(mode);
    inputQueue_.SelectInputMode(mode);

    // 'q' などの入力モード切り替えに使用した文字は処理済みとする
    modified_ = true;
}

void SKKInputEngine::SetStatePrimary() {
    enableMainEditor();

    top()->Clear();
}

void SKKInputEngine::SetStateComposing() {
    // 直接入力モードから遷移してきた時だけ初期化する
    if(top() == bottom_) {
        composingEditor_.Clear();
    }

    // Undo が有効なら、見出し語をセットしておく
    if(!undo_.empty()) {
        composingEditor_.SetEntry(undo_);
        undo_.clear();
    }

    enableMainEditor();

    active_->push_back(&composingEditor_);
}

void SKKInputEngine::SetStateOkuri() {
    enableMainEditor();

    active_->push_back(&composingEditor_);
    active_->push_back(&okuriEditor_);

    // 次回 HandleChar で初期化が必要
    needsInitializeOkuri_ = true;
}

void SKKInputEngine::SetStateSelectCandidate() {
    enableSubEditor(&candidateEditor_);
}

void SKKInputEngine::SetStateEntryRemove() {
    entryRemoveEditor_.Initialize(Entry(), contextBuffer_.Candidate());

    enableSubEditor(&entryRemoveEditor_);
}

void SKKInputEngine::HandleChar(char code, bool direct) {
    if(needsInitializeOkuri_) {
        needsInitializeOkuri_ = false;

        okuriEditor_.Initialize(code);
    }

    if(direct) {
        inputQueue_.SelectInputMode(AsciiInputMode);
    }
    
    inputQueue_.AddChar(code);

    if(direct) {
        inputQueue_.SelectInputMode(inputMode());
    }
}

void SKKInputEngine::HandleBackSpace() {
    if(inputQueue_.IsEmpty()) {
        top()->Input(SKKBaseEditor::BackSpace);
    } else {
        inputQueue_.RemoveChar();
    }
}

void SKKInputEngine::HandleDelete() {
    top()->Input(SKKBaseEditor::Delete);
}

void SKKInputEngine::HandleCursorLeft() {
    top()->Input(SKKBaseEditor::CursorLeft);
}

void SKKInputEngine::HandleCursorRight() {
    top()->Input(SKKBaseEditor::CursorRight);
}

void SKKInputEngine::HandleCursorUp() {
    top()->Input(SKKBaseEditor::CursorUp);
}

void SKKInputEngine::HandleCursorDown() {
    top()->Input(SKKBaseEditor::CursorDown);
}

void SKKInputEngine::HandlePaste() {
    Insert(clipboard_->PasteString());
}

void SKKInputEngine::HandlePing() {
    inputModeSelector_.Activate();
}

void SKKInputEngine::Commit() {
    terminate();

    word_.clear();

    // Top のフィルターから Commit していき、最終的な単語を取得する
    for(EditorStack::reverse_iterator iter = active_->rbegin(); iter != active_->rend(); ++ iter) {
        (*iter)->Commit(word_);
    }

    contextBuffer_.Clear();

    enableMainEditor();
}

void SKKInputEngine::Insert(const std::string& str) {
    enableMainEditor();

    top()->Input(str, "");

    // 強制的に出力する
    Output();
}

void SKKInputEngine::Reset(bool absolutely) {
    cancel();

    top()->Flush();

    // 完全にリセットする場合は、IsModified() を偽にする
    if(absolutely) {
        modified_ = false;
    }
}

void SKKInputEngine::ToggleKana() {
    terminate();

    std::string entry(Entry().EntryString());
    std::string result;

    switch(inputMode()) {
    case HirakanaInputMode:
	jconv::hirakana_to_katakana(entry, result);
	break;

    case KatakanaInputMode:
	jconv::katakana_to_hirakana(entry, result);
	break;

    case Jisx0201KanaInputMode:
	jconv::jisx0201_kana_to_katakana(entry, result);
	break;

    default:
	break;
    }

    SKKBackEnd::theInstance().Register(entry, SKKCandidate());

    Insert(result);
}

void SKKInputEngine::ToggleJisx0201Kana() {
    terminate();

    std::string entry(Entry().EntryString());
    std::string result;

    switch(inputMode()) {
    case HirakanaInputMode:
	jconv::hirakana_to_jisx0201_kana(entry, result);
	break;
	
    case KatakanaInputMode:
	jconv::katakana_to_jisx0201_kana(entry, result);
	break;

    case Jisx0201KanaInputMode:
	jconv::jisx0201_kana_to_hirakana(entry, result);
	break;

    case AsciiInputMode:
	jconv::ascii_to_jisx0208_latin(entry, result);
	break;

    default:
	break;
    }

    SKKBackEnd::theInstance().Register(entry, SKKCandidate());

    Insert(result);
}

SKKInputEngine::UndoResult SKKInputEngine::Undo(const std::string& candidate) {
    if(candidate.empty()) return UndoFailed;

    // 逆引き
    undo_ = SKKBackEnd::theInstance().ReverseLookup(candidate);

    if(undo_.empty()) {
        return UndoFailed;
    }

    // 表示不可能な文字が含まれるか？
    if(std::find_if(undo_.begin(), undo_.end(),
                    std::not1(std::ptr_fun(isprint))) != undo_.end()) {
        return UndoKanaEntry;
    }

    return UndoAsciiEntry;
}

void SKKInputEngine::Output() {
    // 内部バッファが更新されている時だけ出力する
    if(IsModified()) {
        updateContextBuffer();
        contextBuffer_.Output(frontend_);

        // 直接入力モードのカーソル移動等では内部バッファが変更されず、
        // empty 状態が保たれる
        //
        // こういったケースを弾かないと、クライアント側で文字列選択状態
        // だった場合に empty 内部バッファで選択文字列が消されてしまう
    }

    // 全てのエディターを Flush する
    std::for_each(active_->begin(), active_->end(), std::mem_fun(&SKKBaseEditor::Flush));

    inputModeSelector_.Notify();

    modified_ = false;
}

bool SKKInputEngine::IsModified() const {
    return modified_ || top()->IsModified();
}

bool SKKInputEngine::CanConvert(char code) const {
    return inputQueue_.CanConvert(code);
}

bool SKKInputEngine::IsOkuriComplete() const {
    return okuriEditor_.IsOkuriComplete() && inputQueue_.IsEmpty();
}

void SKKInputEngine::BeginRegistration() {
    registrationObserver_->SKKRegistrationUpdate(SKKRegistrationObserver::Begin);
}

void SKKInputEngine::FinishRegistration() {
    Commit();

    registrationObserver_->SKKRegistrationUpdate(SKKRegistrationObserver::Finish);
}

void SKKInputEngine::AbortRegistration() {
    cancel();

    registrationObserver_->SKKRegistrationUpdate(SKKRegistrationObserver::Abort);
}

const SKKEntry SKKInputEngine::Entry() const {
    return contextBuffer_.Entry();
}

const std::string SKKInputEngine::Word() const {
    return word_;
}

// ------------------------------------------------------------

SKKBaseEditor* SKKInputEngine::top() const {
    return active_->back();
}

SKKInputMode SKKInputEngine::inputMode() const {
    return inputModeSelector_;
}

void SKKInputEngine::terminate() {
    if(option_->FixIntermediateConversion()) {
        inputQueue_.Terminate();
    } else {
        inputQueue_.Clear();
    }

    updateContextBuffer();
}

void SKKInputEngine::cancel() {
    if(!inputQueue_.IsEmpty()) {
        modified_ = true;
    }

    inputQueue_.Clear();

    top()->Clear();
}

void SKKInputEngine::enableMainEditor() {
    active_ = &mainStack_;

    active_->clear();
    active_->push_back(bottom_);

    needsInitializeOkuri_ = false;
}

void SKKInputEngine::enableSubEditor(SKKBaseEditor* editor) {
    active_ = &subStack_;

    active_->clear();
    active_->push_back(bottom_);
    active_->push_back(editor);
}

void SKKInputEngine::updateContextBuffer() {
    contextBuffer_.Clear();

    for(EditorStack::iterator iter = active_->begin(); iter != active_->end(); ++ iter) {
        (*iter)->Output(contextBuffer_);
    }

    // 非確定文字があれば挿入(ex. "ky" など)
    contextBuffer_.Compose(inputQueue_.QueueString());
}

// ------------------------------------------------------------

void SKKInputEngine::SKKInputQueueUpdate(const std::string& fixed, const std::string& queue) {
    top()->Input(fixed, queue);
}

const std::string SKKInputEngine::SKKCompleterQueryString() {
    terminate();

    return composingEditor_.QueryString();
}

void SKKInputEngine::SKKCompleterUpdate(const std::string& entry) {
    composingEditor_.SetEntry(entry);
}

const SKKEntry SKKInputEngine::SKKSelectorQueryEntry() {
    terminate();

    SKKEntry entry(Entry());
    std::string query(entry.EntryString());

    // 入力モードがカタカナ/半角カナなら、見出し語をひらかなに正規化する
    switch(inputMode()) {
    case KatakanaInputMode:
        jconv::katakana_to_hirakana(query, query);
        break;

    case Jisx0201KanaInputMode:
        jconv::jisx0201_kana_to_hirakana(query, query);
        break;
    }

    entry.SetEntry(query);

    candidateEditor_.Initialize(entry);

    return entry;
}

void SKKInputEngine::SKKSelectorUpdate(const SKKCandidate& candidate) {
    candidateEditor_.SetCandidate(candidate);
}
