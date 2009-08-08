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
#include "SKKClipboard.h"
#include "SKKAnnotator.h"
#include "SKKContextBuffer.h"
#include "SKKBackEnd.h"
#include "SKKFrontEnd.h"
#include "SKKRegisterEditor.h"
#include "jconv.h"
#include <iostream>
#include <cctype>

SKKInputEngine::SKKInputEngine(SKKInputEnvironment* env)
    : env_(env) , handled_(false) , inputQueue_(this) , okuriEditor_(this) {
    SetStatePrimary();
}

void SKKInputEngine::SelectInputMode(SKKInputMode mode) {
    handled_ = true;

    env_->InputModeSelector()->Select(mode);
    inputQueue_.SelectInputMode(mode);
}

void SKKInputEngine::RefreshInputMode() {
    env_->InputModeSelector()->Refresh();
}

void SKKInputEngine::SetStatePrimary() {
    enableMainEditor();

    insert(restore_);
    restore_.clear();
}

void SKKInputEngine::SetStateComposing() {
    // 直接入力モードから遷移してきた時だけ初期化する
    if(top() == env_->BaseEditor()) {
        composingEditor_.Clear();
    }

    // Undo が有効なら、見出し語をセットしておく
    if(!undo_.empty()) {
        composingEditor_.SetEntry(undo_);
        undo_.clear();
    }

    // ダイナミック補完オプション設定
    SKKInputEngineOption* option = env_->InputEngineOption();
    composingEditor_.EnableDynamicCompletion(option->EnableDynamicCompletion());
    composingEditor_.SetDynamicCompletionRange(option->DynamicCompletionRange());

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

    inputQueue_.AddChar(code, direct);
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
    top()->Input(env_->InputSessionParameter()->Clipboard()->PasteString());
}

void SKKInputEngine::HandlePing() {
    env_->InputModeSelector()->Show();
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
    restore_ = env_->InputSessionParameter()->FrontEnd()->SelectedString();

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
    SKKInputEngineOption* option = env_->InputEngineOption();
    SKKInputSessionParameter* param = env_->InputSessionParameter();
    SKKDynamicCompletor* completer = 0;
    SKKAnnotator* annotator = 0;


    if(option->EnableDynamicCompletion()) {
        completer = param->DynamicCompletor();
    }

    if(option->EnableAnnotation()) {
        annotator = param->Annotator();
    }

    updateContextBuffer();
    outputQueue_.Add(contextBuffer_);
    outputQueue_.Output(param->FrontEnd(), completer, annotator);

    // 全てのエディターを Flush する
    std::for_each(active_->begin(), active_->end(), std::mem_fun(&SKKBaseEditor::Flush));

    env_->InputModeSelector()->Notify();

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
    env_->RegistrationObserver()->SKKRegistrationBegin(new SKKRegisterEditor(Entry()));
}

void SKKInputEngine::FinishRegistration() {
    Commit();

    env_->RegistrationObserver()->SKKRegistrationFinish(word_);
}

void SKKInputEngine::AbortRegistration() {
    if(!inputQueue_.IsEmpty()) {
        terminate();
        return;
    }

    cancel();

    env_->RegistrationObserver()->SKKRegistrationCancel();
}

const SKKEntry SKKInputEngine::Entry() const {
    return contextBuffer_.Entry();
}

// ------------------------------------------------------------

SKKBaseEditor* SKKInputEngine::top() const {
    return active_->back();
}

SKKInputMode SKKInputEngine::inputMode() const {
    return *(env_->InputModeSelector());
}

void SKKInputEngine::terminate() {
    if(env_->InputEngineOption()->FixIntermediateConversion()) {
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
    active_->push_back(env_->BaseEditor());

    needsInitializeOkuri_ = false;
}

void SKKInputEngine::enableSubEditor(SKKBaseEditor* editor) {
    active_ = &subStack_;

    active_->clear();
    active_->push_back(env_->BaseEditor());
    active_->push_back(editor);
}

void SKKInputEngine::updateContextBuffer() {
    contextBuffer_.Clear();

    for(EditorStack::iterator iter = active_->begin(); iter != active_->end(); ++ iter) {
        (*iter)->Output(contextBuffer_);
    }

    // 非確定文字があれば挿入(ex. "ky" など)
    if(env_->InputEngineOption()->DisplayShortestMatchOfKanaConversions()) {
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
    SKKEntry entry = SKKSelectorQueryEntry();

    return entry.EntryString();
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
