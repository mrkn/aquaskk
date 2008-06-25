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

#include "SKKEditorStack.h"
#include "SKKInputMode.h"
#include "SKKClipboard.h"
#include "SKKContextBuffer.h"
#include "SKKCandidateEditor.h"
#include "SKKEvent.h"
#include "SKKRomanKanaConverter.h"
#include "SKKBackEnd.h"
#include "utf8util.h"
#include "jconv.h"
#include <cassert>

SKKEditorStack::SKKEditorStack(SKKRegistrationObserver* registrationObserver,
                               SKKInputModeWindow* inputModeWindow,
                               SKKClipboard* clipboard,
                               SKKBaseEditor* bottom)
    : registrationObserver_(registrationObserver),
      inputModeSelector_(inputModeWindow),
      clipboard_(clipboard),
      modified_(false) {
    active_ = &composing_;
    PushEditor(bottom);
}

void SKKEditorStack::SelectInputMode(SKKInputMode mode) {
    inputModeSelector_.Select(mode);
    input_.clear();
}

SKKInputMode SKKEditorStack::InputMode() const {
    return inputModeSelector_;
}

void SKKEditorStack::ActivateInputMode() {
    modified_ = true;
    inputModeSelector_.Activate();
}

void SKKEditorStack::Input(const SKKEvent& event) {
    switch(event.id) {
    case SKK_CHAR:
        handleInput(event.code);
        break;

    case SKK_BACKSPACE:
        if(input_.empty()) {
            top()->Input(SKKBaseEditor::BackSpace);
        } else {
            utf8::pop(input_);
            top()->Input("", input_);
        }
        break;

    case SKK_DELETE:
        top()->Input(SKKBaseEditor::Delete);
        break;

    case SKK_LEFT:
        top()->Input(SKKBaseEditor::CursorLeft);
        break;

    case SKK_RIGHT:
        top()->Input(SKKBaseEditor::CursorRight);
        break;

    case SKK_UP:
        top()->Input(SKKBaseEditor::CursorUp);
        break;

    case SKK_DOWN:
        top()->Input(SKKBaseEditor::CursorDown);
        break;

    default:
        return;
    }

    // SKKContextBuffer を更新する
    collect();
}

void SKKEditorStack::Paste(const std::string& str) {
    resetStack();

    top()->Input(str, "");

    collect();
}

void SKKEditorStack::Paste() {
    Paste(clipboard_->PasteString());
}

void SKKEditorStack::Commit() {
    std::string queue;

    // Top のフィルターから Commit していき、最終的な単語を取得する
    for(EditorStack::reverse_iterator iter = active_->rbegin(); iter != active_->rend(); ++ iter) {
        (*iter)->Commit(queue);
    }

    word_ = queue;

    resetStack();
}

void SKKEditorStack::Cancel() {
    top()->Clear();
    input_.clear();
}

void SKKEditorStack::Reset() {
    Cancel();
    top()->Flush();
}

void SKKEditorStack::ToggleKana() {
    FixEntry();
    collect();

    std::string entry(Entry().EntryString());
    std::string result;

    switch(InputMode()) {
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

    Paste(result);
}

void SKKEditorStack::ToggleJisx0201Kana() {
    FixEntry();
    collect();

    std::string entry(Entry().EntryString());
    std::string result;

    switch(InputMode()) {
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

    Paste(result);
}

void SKKEditorStack::Output(SKKFrontEnd* frontend) {
    // SKKContextBuffer を更新する
    collect();

    buffer_.Output(frontend);

    // 全てのフィルターを Flush する
    for(EditorStack::iterator iter = active_->begin(); iter != active_->end(); ++ iter) {
        (*iter)->Flush();
    }

    inputModeSelector_.Notify();
    modified_ = false;
}

bool SKKEditorStack::IsEmpty() const {
    return false;
}

bool SKKEditorStack::IsModified() const {
    return modified_ || top()->IsModified();
}

bool SKKEditorStack::WillConvert(const SKKEvent& event) const {
    SKKRomanKanaConverter& converter = SKKRomanKanaConverter::theInstance();
    std::string input(input_);
    std::string fixed;
    std::string next;

    switch(InputMode()) {
    case HirakanaInputMode:
    case KatakanaInputMode:
    case Jisx0201KanaInputMode:
	// ローマ字 → かな変換
	input += std::tolower(event.code);
	return converter.Execute(InputMode(), input, fixed, next);

    default:
        return false;
    }
}

void SKKEditorStack::PushEditor(SKKBaseEditor* editor) {
    active_->push_back(editor);
    active_->back()->Clear();
    collect();
}

void SKKEditorStack::PopEditor() {
    if(active_->size() == 1) return;

    active_->back()->Clear();
    active_->pop_back();
    active_->back()->Input("", "");
}

void SKKEditorStack::EnableSubEditor(SKKBaseEditor* editor) {
    active_ = &sub_;

    sub_.clear();
    sub_.push_back(composing_[0]);
    sub_.push_back(editor);
}

void SKKEditorStack::DisableSubEditor() {
    active_ = &composing_;
}

void SKKEditorStack::BeginRegistration() {
    registrationObserver_->SKKRegistrationUpdate(SKKRegistrationObserver::Begin);
}

void SKKEditorStack::FinishRegistration() {
    Commit();
    registrationObserver_->SKKRegistrationUpdate(SKKRegistrationObserver::Finish);
}

void SKKEditorStack::AbortRegistration() {
    Cancel();
    registrationObserver_->SKKRegistrationUpdate(SKKRegistrationObserver::Abort);
}

void SKKEditorStack::FixEntry() {
    if(input_.empty()) return;

    Input(SKKEvent(SKK_CHAR, 0));
    Input(SKKEvent(SKK_BACKSPACE, 0));
}

const SKKEntry SKKEditorStack::Entry() const {
    return buffer_.Entry();
}

const SKKCandidate SKKEditorStack::Candidate() const {
    return buffer_.Candidate();
}

const std::string SKKEditorStack::Word() const {
    return word_;
}

SKKBaseEditor* SKKEditorStack::top() const {
    assert(active_->back());
    return active_->back();
}

void SKKEditorStack::resetStack() {
    DisableSubEditor();

    SKKBaseEditor* bottom = composing_[0];
    composing_.clear();
    composing_.push_back(bottom);
}

void SKKEditorStack::handleInput(char code) {
    SKKRomanKanaConverter& converter = SKKRomanKanaConverter::theInstance();
    std::string fixed;
    std::string next;

    switch(InputMode()) {
    case AsciiInputMode:
	top()->Input(std::string(1, code));
	return;

    case HirakanaInputMode:
    case KatakanaInputMode:
    case Jisx0201KanaInputMode:
	// ローマ字 → かな変換
	input_ += std::tolower(code);
	converter.Execute(InputMode(), input_, fixed, next);
	input_ = next;
	break;

    case Jisx0208LatinInputMode:
	// ASCII → 全角英数変換
	input_ += code;
	jconv::ascii_to_jisx0208_latin(input_, fixed);
	input_.clear();
	break;

    default:
        break;
    }

    top()->Input(fixed, input_);
}

void SKKEditorStack::collect() {
    // Top のフィルターだけは active 引数を true にする
    SKKContextBuffer tmp;
    for(EditorStack::iterator iter = active_->begin(); iter != active_->end(); ++ iter) {
        (*iter)->Output(tmp, *iter == top());
    }

    buffer_ = tmp;
}
