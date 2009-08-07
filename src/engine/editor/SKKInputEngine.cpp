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
#include "SKKAnnotator.h"
#include "SKKContextBuffer.h"
#include "SKKBackEnd.h"
#include "SKKFrontEnd.h"
#include "SKKRegisterEditor.h"
#include "jconv.h"
#include <iostream>
#include <cctype>

SKKInputEngine::SKKInputEngine(SKKRegistrationObserver* registrationObserver,
                               SKKInputModeSelector* inputModeSelector,
                               SKKBaseEditor* bottom,
                               SKKInputSessionParameter* param)
    : registrationObserver_(registrationObserver)
    , inputModeSelector_(inputModeSelector)
    , bottom_(bottom)
    , sessionParam_(param)
    , option_(sessionParam_->InputEngineOption())
    , handled_(false)
    , inputQueue_(this)
    , okuriEditor_(this) {
    SetStatePrimary();
}

void SKKInputEngine::SelectInputMode(SKKInputMode mode) {
    handled_ = true;

    inputModeSelector_->Select(mode);
    inputQueue_.SelectInputMode(mode);
}

void SKKInputEngine::RefreshInputMode() {
    inputModeSelector_->Refresh();
}

void SKKInputEngine::SetStatePrimary() {
    enableMainEditor();

    insert(restore_);
    restore_.clear();
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

    // ダイナミック補完オプション設定
    composingEditor_.EnableDynamicCompletion(option_->EnableDynamicCompletion());
    composingEditor_.SetDynamicCompletionRange(option_->DynamicCompletionRange());

    enableMainEditor();

    active_->push_back(&composingEditor_);
}

void SKKInputEngine::SetStateOkuri() {
    enableMainEditor();

    active_->push_back(&composingEditor_);
    active_->push_back(&okuriEditor_);

    top()->Clear();

    // ダイナミック補完をオフにする
    composingEditor_.EnableDynamicCompletion(false);

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
    fire(SKKBaseEditor::Delete);
}

void SKKInputEngine::HandleCursorLeft() {
    fire(SKKBaseEditor::CursorLeft);
}

void SKKInputEngine::HandleCursorRight() {
    fire(SKKBaseEditor::CursorRight);
}

void SKKInputEngine::HandleCursorUp() {
    fire(SKKBaseEditor::CursorUp);
}

void SKKInputEngine::HandleCursorDown() {
    fire(SKKBaseEditor::CursorDown);
}

void SKKInputEngine::HandlePaste() {
    top()->Input(sessionParam_->Clipboard()->PasteString());
}

void SKKInputEngine::HandlePing() {
    inputModeSelector_->Show();
}

void SKKInputEngine::Commit() {
    terminate();

    restore_.clear();
    word_.clear();

    // Top のフィルターから Commit していき、最終的な単語を取得する
    for(EditorStack::reverse_iterator iter = active_->rbegin(); iter != active_->rend(); ++ iter) {
        (*iter)->Commit(word_);
    }

    enableMainEditor();

    contextBuffer_.Clear();
}

void SKKInputEngine::Reset() {
    cancel();

    top()->Flush();
}

void SKKInputEngine::Register(const std::string& word) {
    SKKEntry entry = Entry();

    study(entry, SKKCandidate(word, false));

    insert(word + entry.OkuriString());
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

    study(entry, SKKCandidate());

    insert(result);
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

    study(entry, SKKCandidate());

    insert(result);
}

SKKInputEngine::UndoResult SKKInputEngine::Undo() {
    restore_ = sessionParam_->FrontEnd()->SelectedString();

    // 逆引き
    undo_ = SKKBackEnd::theInstance().ReverseLookup(restore_);

    if(undo_.empty()) {
        restore_.clear();
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
    SKKDynamicCompletor* completer = 0;
    SKKAnnotator* annotator = 0;

    if(option_->EnableDynamicCompletion()) {
        completer = sessionParam_->DynamicCompletor();
    }

    if(option_->EnableAnnotation()) {
        annotator = sessionParam_->Annotator();
    }

    updateContextBuffer();
    outputQueue_.Add(contextBuffer_);
    outputQueue_.Output(sessionParam_->FrontEnd(), completer, annotator);

    // 全てのエディターを Flush する
    std::for_each(active_->begin(), active_->end(), std::mem_fun(&SKKBaseEditor::Flush));

    inputModeSelector_->Notify();

    handled_ = false;
}

bool SKKInputEngine::IsModified() const {
    return top()->IsModified() || handled_;
}

bool SKKInputEngine::IsComposing() const {
    return contextBuffer_.IsComposing();
}

bool SKKInputEngine::CanConvert(char code) const {
    return inputQueue_.CanConvert(code);
}

bool SKKInputEngine::IsOkuriComplete() const {
    return okuriEditor_.IsOkuriComplete() && inputQueue_.IsEmpty();
}

void SKKInputEngine::BeginRegistration() {
    registrationObserver_->SKKRegistrationBegin(new SKKRegisterEditor(Entry()));
}

void SKKInputEngine::FinishRegistration() {
    Commit();

    registrationObserver_->SKKRegistrationFinish(word_);
}

void SKKInputEngine::AbortRegistration() {
    if(!inputQueue_.IsEmpty()) {
        terminate();
        return;
    }

    cancel();
    registrationObserver_->SKKRegistrationCancel();
}

const SKKEntry SKKInputEngine::Entry() const {
    return contextBuffer_.Entry();
}

// ------------------------------------------------------------

SKKBaseEditor* SKKInputEngine::top() const {
    return active_->back();
}

SKKInputMode SKKInputEngine::inputMode() const {
    return *inputModeSelector_;
}

void SKKInputEngine::terminate() {
    if(option_->FixIntermediateConversion()) {
        inputQueue_.Terminate();
    } else {
        inputQueue_.Clear();
    }

    updateContextBuffer();
}

void SKKInputEngine::fire(SKKBaseEditor::Event event) {
    if(!inputQueue_.IsEmpty()) {
        inputQueue_.Clear();
        top()->Flush();
    } else {
        top()->Input(event);
    }
}

void SKKInputEngine::cancel() {
    if(!inputQueue_.IsEmpty()) {
        outputQueue_.Add(SKKContextBuffer());
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
    if(option_->DisplayShortestMatchOfKanaConversions()) {
        if(inputState_.intermediate.empty()) {
            contextBuffer_.Compose(inputState_.queue);
        } else {
            contextBuffer_.Compose(inputState_.intermediate);
        }
    } else {
        contextBuffer_.Compose(inputState_.queue);
    }
}

void SKKInputEngine::study(const SKKEntry& entry, const SKKCandidate& candidate) {
    SKKBackEnd::theInstance().Register(entry, candidate);
}

void SKKInputEngine::insert(const std::string& str) {
    enableMainEditor();

    top()->Input(str, "", 0);

    // 強制的に出力する
    Output();
}

// ------------------------------------------------------------

void SKKInputEngine::SKKInputQueueUpdate(const SKKInputQueueObserver::State& state) {
    inputState_ = state;

    if(inputMode() == AsciiInputMode) {
        top()->Input(state.fixed);
    } else {
        top()->Input(state.fixed, state.queue, state.code);
    }
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

void SKKInputEngine::SKKOkuriListenerAppendEntry(const std::string& fixed) {
    composingEditor_.Input(fixed, "", 0);
}
