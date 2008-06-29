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

// ======================================================================
// level 1：構築
// ======================================================================
State SKKState::Composing(const Event& event) {
    switch(event) {
    case EXIT_EVENT:
        // 履歴を保存する
        return State::SaveHistory();

    case SKK_PING:
        editor_->ActivateInputMode();
        return 0;
    }

    return &SKKState::TopState;
}

// ======================================================================
// level 2：見出し語編集
// ======================================================================
State SKKState::Edit(const Event& event) {
    const SKKEvent& param = event.Param();

    switch(event) {
    case SKK_ENTER:
    case SKK_JMODE:
        editor_->Commit();

        // 改行するかどうか？(egg-like-new-line)
        if(event == SKK_ENTER && !configuration_->SuppressNewlineOnCommit()) {
            return State::Forward(&SKKState::KanaInput);
        }

        return State::Transition(&SKKState::KanaInput);

    case SKK_CANCEL:
        editor_->PopEditor();
        return State::Transition(&SKKState::KanaInput);

    case SKK_TAB:
        if(completer_.Execute()) {
            return State::Transition(&SKKState::EntryCompletion);
        }
        return 0;

    case SKK_BACKSPACE:
        editor_->Input(param);

        if(!editor_->IsModified()) {
            editor_->PopEditor();
            return State::Transition(&SKKState::KanaInput);
        }

        return 0;

    case SKK_DELETE:
    case SKK_LEFT:
    case SKK_RIGHT:
    case SKK_UP:
    case SKK_DOWN:
        editor_->Input(param);
        return 0;

    case SKK_CHAR:
        if(param.IsNextCandidate()) {
            if(editor_->Entry().IsEmpty()) {
                editor_->PopEditor();
                return State::Transition(&SKKState::KanaInput);
            }

            editor_->FixEntry();
            candidateEditor_.Initialize(editor_->Entry());

            if(selector_.Execute(configuration_->MaxCountOfInlineCandidates())) {
                return State::Transition(&SKKState::SelectCandidate);
            }

            return State::Transition(&SKKState::RecursiveRegister);
        }

        return 0;
    }

    return &SKKState::Composing;
}

// ======================================================================
// level 3 (sub of Edit)：見出し語入力
// ======================================================================
State SKKState::EntryInput(const Event& event) {
    // 履歴を保存するだけ
    switch(event) {
    case EXIT_EVENT:
        return State::SaveHistory();
    }

    return &SKKState::Edit;
}

// ======================================================================
// level 4 (sub of EntryInput)：日本語
// ======================================================================
State SKKState::KanaEntry(const Event& event) {
    const SKKEvent& param = event.Param();

    switch(event) {
    case SKK_CHAR:
        // 変換
        if(param.IsNextCandidate()) break;

        if(!editor_->Entry().IsEmpty()) {
            // トグル変換 #1
            if(param.IsToggleKana()) {
                editor_->ToggleKana();
                return State::Transition(&SKKState::KanaInput);
            }

            // トグル変換 #2
            if(param.IsToggleJisx0201Kana()) {
                editor_->ToggleJisx0201Kana();
                return State::Transition(&SKKState::KanaInput);
            }

            // 送りあり
            if(param.IsUpperCases()) {
                okuriEditor_.Initialize(param.code);
                return State::Forward(&SKKState::OkuriInput);
            }
        }

        if(!editor_->WillConvert(param)) {
            if(param.IsSwitchToAscii()) {
                editor_->Commit();
                return State::Transition(&SKKState::Ascii);
            }
            
            if(param.IsSwitchToJisx0208Latin()) {
                editor_->Commit();
                return State::Transition(&SKKState::Jisx0208Latin);
            }

            if(param.IsEnterJapanese()) {
                editor_->Commit();
                editor_->PushEditor(&composingEditor_);
                return State::Transition(&SKKState::KanaEntry);
            }
        }

        if(param.IsPlain()) {
            editor_->Input(param);
        }

        return 0;
    }

    return &SKKState::EntryInput;
}

// ======================================================================
// level 4 (sub of EntryInput)：省略表記
// ======================================================================
State SKKState::AsciiEntry(const Event& event) {
    const SKKEvent& param = event.Param();

    switch(event) {
    case ENTRY_EVENT:
        editor_->SelectInputMode(AsciiInputMode);
        return 0;

    case SKK_CHAR:
        if(param.IsNextCandidate()) break;

        if(!editor_->Entry().IsEmpty()) {
            if(param.IsToggleJisx0201Kana()) {
                editor_->ToggleJisx0201Kana();
                return State::Transition(&SKKState::KanaInput);
            }
        }

        if(param.IsPlain()) {
            editor_->Input(param);
        }

        return 0;
    }

    return &SKKState::EntryInput;
}

// ======================================================================
// level 3 (sub of Edit)：送り
// ======================================================================
State SKKState::OkuriInput(const Event& event) {
    const SKKEvent& param = event.Param();

    switch(event) {
    case ENTRY_EVENT:
        editor_->PushEditor(&okuriEditor_);
        return 0;

    case EXIT_EVENT:
        editor_->PopEditor();
        return 0;

    case SKK_CANCEL:
        return State::Transition(&SKKState::KanaEntry);

    case SKK_TAB:
    case SKK_DELETE:
    case SKK_LEFT:
    case SKK_RIGHT:
    case SKK_UP:
    case SKK_DOWN:
        editor_->Input(param);
        return 0;

    case SKK_BACKSPACE:
        editor_->Input(param);

        if(!editor_->IsModified()) {
            return State::Transition(&SKKState::KanaEntry);
        }

        return 0;

    case SKK_CHAR:
        if(param.IsPlain()) {
            editor_->Input(param);
        }

        if(param.IsNextCandidate() || okuriEditor_.DidFinish()) {
            editor_->FixEntry();
            candidateEditor_.Initialize(editor_->Entry());

            if(selector_.Execute(configuration_->MaxCountOfInlineCandidates())) {
                return State::Transition(&SKKState::SelectCandidate);
            }

            return State::Transition(&SKKState::RecursiveRegister);
        }

        return 0;
    }

    return &SKKState::Edit;
}

// ======================================================================
// level 3 (sub of Edit)：見出し語補完
// ======================================================================
State SKKState::EntryCompletion(const Event& event) {
    const SKKEvent& param = event.Param();

    // ENTRY 以外のシステムイベントは上位状態にルーティング
    if(event.IsSystem() && event != ENTRY_EVENT) {
        return &SKKState::Edit;
    }

    switch(event) {
    case ENTRY_EVENT:
        return 0;

    case SKK_TAB:
        completer_.Next();
        return 0;

    case SKK_CHAR:
        if(param.IsNextCompletion()) {
            completer_.Next();
            return 0;
        }

        if(param.IsPrevCompletion()) {
            completer_.Prev();
            return 0;
        }

        if(param.IsNextCandidate()) {
            return &SKKState::Edit;
        }
    }

    // 補完キー以外なら履歴に転送する
    return State::DeepForward(&SKKState::EntryInput);
}

// ======================================================================
// level 2：候補選択
// ======================================================================
State SKKState::SelectCandidate(const Event& event) {
    const SKKEvent& param = event.Param();

    switch(event) {
    case ENTRY_EVENT:
        editor_->EnableSubEditor(&candidateEditor_);
        selector_.Show();
        return 0;

    case EXIT_EVENT:
        editor_->DisableSubEditor();
        selector_.Hide();
        return 0;

    case SKK_ENTER:
    case SKK_JMODE:
        editor_->Commit();

        // 改行するかどうか？(egg-like-new-line)
        if(event == SKK_ENTER && !configuration_->SuppressNewlineOnCommit()) {
            return State::Forward(&SKKState::KanaInput);
        }

        return State::Transition(&SKKState::KanaInput);

    case SKK_CANCEL:
        return State::DeepHistory(&SKKState::EntryInput);

    case SKK_LEFT:
        selector_.CursorLeft();
        return 0;

    case SKK_RIGHT:
        selector_.CursorRight();
        return 0;

    case SKK_UP:
        selector_.CursorUp();
        return 0;

    case SKK_DOWN:
        selector_.CursorDown();
        return 0;

    case SKK_CHAR:
        if(param.IsPrevCandidate()) {
            if(!selector_.Prev()) {
                return State::DeepHistory(&SKKState::EntryInput);
            }
            return 0;
        }

        if(param.IsNextCandidate()) {
            if(!selector_.Next()) {
                return State::Transition(&SKKState::RecursiveRegister);
            }
            return 0;
        }

        if(param.IsRemoveTrigger()) {
            return State::Transition(&SKKState::EntryRemove);
        }

        if(selector_.IsInline()) {
            editor_->Commit();
            return State::DeepForward(&SKKState::KanaInput);
        }

        if(selector_.Select(param.code)) {
            editor_->Commit();
            return State::Transition(&SKKState::KanaInput);
        }

        return 0;
    }

    return &SKKState::Composing;
}
