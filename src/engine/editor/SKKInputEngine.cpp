/* -*- C++ -*-

  MacOS X implementation of the SKK input method.

  Copyright (C) 2008-2009 Tomotaka SUWA <t.suwa@mac.com>

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
#include "SKKInputContext.h"
#include "SKKClipboard.h"
#include "SKKBackEnd.h"
#include "SKKRegisterEditor.h"
#include <iostream>
#include <cctype>
#include <cstdarg>

// ----------------------------------------------------------------------

// RAII による SKKInputContext の同期管理
//
// SKKInputEngine::SetState* メソッドで使用する。
class SKKInputEngine::Synchronizer {
    SKKInputEngine* engine_;

public:
    Synchronizer(SKKInputEngine* engine) : engine_(engine) {
        // 直近の状態を SKKInputContext に反映する
        engine_->SyncInputContext();
    }

    ~Synchronizer() {
        // Top エディタを初期化する
        engine_->top()->ReadContext();

        // 最新の状態を SKKInputContext に反映する
        engine_->SyncInputContext();
    }
};

// ----------------------------------------------------------------------

SKKInputEngine::SKKInputEngine(SKKInputEnvironment* env)
    : env_(env), inputQueue_(this)
    , composingEditor_(env->InputContext())
    , okuriEditor_(env->InputContext(), this)
    , candidateEditor_(env->InputContext())
    , entryRemoveEditor_(env->InputContext()) {
    SetStatePrimary();
}

void SKKInputEngine::SelectInputMode(SKKInputMode mode) {
    env_->InputModeSelector()->Select(mode);
    inputQueue_.SelectInputMode(mode);

    env_->InputContext()->event_handled = true;
}

void SKKInputEngine::RefreshInputMode() {
    env_->InputModeSelector()->Refresh();
}

void SKKInputEngine::SetStatePrimary() {
    Synchronizer sync(this);

    initialize();
}

void SKKInputEngine::SetStateComposing() {
    Synchronizer sync(this);

    initialize();
    push(&composingEditor_);
}

void SKKInputEngine::SetStateOkuri() {
    Synchronizer sync(this);

    initialize();
    push(&composingEditor_);
    push(&okuriEditor_);
}

void SKKInputEngine::SetStateSelectCandidate() {
    Synchronizer sync(this);

    initialize();
    push(&candidateEditor_);
}

void SKKInputEngine::SetStateEntryRemove() {
    Synchronizer sync(this);

    initialize();
    push(&entryRemoveEditor_);
}

void SKKInputEngine::HandleChar(char code, bool direct) {
    inputQueue_.AddChar(code, direct);
}

void SKKInputEngine::HandleBackSpace() {
    if(inputQueue_.IsEmpty()) {
        invoke(SKKBaseEditor::BackSpace);
    } else {
        inputQueue_.RemoveChar();
    }
}

void SKKInputEngine::HandleDelete() {
    invoke(SKKBaseEditor::Delete);
}

void SKKInputEngine::HandleCursorLeft() {
    invoke(SKKBaseEditor::CursorLeft);
}

void SKKInputEngine::HandleCursorRight() {
    invoke(SKKBaseEditor::CursorRight);
}

void SKKInputEngine::HandleCursorUp() {
    invoke(SKKBaseEditor::CursorUp);
}

void SKKInputEngine::HandleCursorDown() {
    invoke(SKKBaseEditor::CursorDown);
}

void SKKInputEngine::HandlePaste() {
    SKKClipboard* clipboard = env_->InputSessionParameter()->Clipboard();

    top()->Input(clipboard->PasteString());
}

void SKKInputEngine::HandlePing() {
    env_->InputModeSelector()->Show();
}

void SKKInputEngine::Commit() {
    terminate();

    word_.clear();

    // Top のフィルターから Commit していき、最終的な単語を取得する
    std::vector<SKKBaseEditor*>::reverse_iterator iter;
    for(iter = stack_.rbegin(); iter != stack_.rend(); ++ iter) {
        (*iter)->Commit(word_);
    }
}

void SKKInputEngine::Cancel() {
    terminate();

    env_->InputContext()->event_handled = false;
}

void SKKInputEngine::Register(const std::string& word) {
    insert(word);
}

void SKKInputEngine::ToggleKana() {
    terminate();

    SKKEntry& entry = env_->InputContext()->entry;

    study(entry, SKKCandidate());

    insert(entry.ToggleKana(inputMode()));
}

void SKKInputEngine::ToggleJisx0201Kana() {
    terminate();

    SKKEntry& entry = env_->InputContext()->entry;
    
    study(entry, SKKCandidate());

    insert(entry.ToggleJisx0201Kana(inputMode()));
}

void SKKInputEngine::SyncInputContext() {
    SKKInputContext* context = env_->InputContext();

    std::for_each(stack_.begin(), stack_.end(), std::mem_fun(&SKKBaseEditor::WriteContext));

    // 非確定文字があれば挿入(ex. "ky" など)
    if(env_->InputEngineOption()->DisplayShortestMatchOfKanaConversions()) {
        if(!inputState_.intermediate.empty()) {
            context->output.Compose(inputState_.intermediate);
            return;
        }
    }

    context->output.Compose(inputState_.queue);

    env_->InputModeSelector()->Notify();
}

bool SKKInputEngine::CanConvert(char code) const {
    return inputQueue_.CanConvert(code);
}

bool SKKInputEngine::IsOkuriComplete() const {
    return env_->InputContext()->entry.IsOkuriAri() && inputQueue_.IsEmpty();
}

void SKKInputEngine::BeginRegistration() {
    SKKBaseEditor* editor = new SKKRegisterEditor(env_->InputContext());

    env_->RegistrationObserver()->SKKRegistrationBegin(editor);
}

void SKKInputEngine::FinishRegistration() {
    Commit();

    SKKEntry entry = env_->InputContext()->entry;

    if(!entry.IsEmpty()) {
        study(entry, SKKCandidate(word_, false));
    }

    env_->RegistrationObserver()->SKKRegistrationFinish(word_ + entry.OkuriString());
}

void SKKInputEngine::AbortRegistration() {
    if(!inputQueue_.IsEmpty()) {
        terminate();
        return;
    }

    env_->RegistrationObserver()->SKKRegistrationCancel();
}

// ----------------------------------------------------------------------

SKKBaseEditor* SKKInputEngine::top() const {
    return stack_.back();
}

SKKInputMode SKKInputEngine::inputMode() const {
    return *(env_->InputModeSelector());
}

void SKKInputEngine::initialize() {
    stack_.clear();
    stack_.push_back(env_->BaseEditor());

    env_->InputContext()->dynamic_completion = false;
    env_->InputContext()->annotator = false;
}

void SKKInputEngine::push(SKKBaseEditor* editor) {
    stack_.push_back(editor);
}

void SKKInputEngine::terminate() {
    // ローマ字かな変換を打ち切る
    if(env_->InputEngineOption()->FixIntermediateConversion()) {
        inputQueue_.Terminate();
    } else {
        inputQueue_.Clear();
    }
}

void SKKInputEngine::invoke(SKKBaseEditor::Event event) {
    if(!inputQueue_.IsEmpty()) {
        inputQueue_.Clear();
        env_->InputContext()->event_handled = false;
    } else {
        top()->Input(event);
    }
}

void SKKInputEngine::study(const SKKEntry& entry, const SKKCandidate& candidate) {
    SKKBackEnd::theInstance().Register(entry, candidate);
}

void SKKInputEngine::insert(const std::string& str) {
    env_->BaseEditor()->Input(str, "", 0);
}

// ----------------------------------------------------------------------

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

    SKKEntry entry = env_->InputContext()->entry.Normalize(inputMode());

    env_->InputContext()->entry = entry;

    return entry;
}

void SKKInputEngine::SKKSelectorUpdate(const SKKCandidate& candidate) {
    candidateEditor_.SetCandidate(candidate);
}

void SKKInputEngine::SKKOkuriListenerAppendEntry(const std::string& fixed) {
    composingEditor_.Input(fixed, "", 0);
}
