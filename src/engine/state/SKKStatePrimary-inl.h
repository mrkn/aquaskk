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
// level 1：直接入力
// ======================================================================
State SKKState::Primary(const Event& event) {
    switch(event) {
    case INIT_EVENT:
	return State::Initial(&SKKState::KanaInput);

    case ENTRY_EVENT:
        editor_->SetStatePrimary();
        return 0;

    case SKK_ENTER:
	editor_->FinishRegistration();
	return 0;

    case SKK_CANCEL:
	editor_->AbortRegistration();
	return 0;

    case SKK_JMODE:
        editor_->Commit();
        return 0;

    case SKK_PASTE:
	editor_->HandlePaste();
	return 0;

    case SKK_PING:
        editor_->HandlePing();
        return 0;

    case SKK_BACKSPACE:
        editor_->HandleBackSpace();
        return 0;

    case SKK_DELETE:
        editor_->HandleDelete();
        return 0;

    case SKK_LEFT:
        editor_->HandleCursorLeft();
        return 0;

    case SKK_RIGHT:
        editor_->HandleCursorRight();
        return 0;

    case SKK_UP:
        editor_->HandleCursorUp();
        return 0;

    case SKK_DOWN:
        editor_->HandleCursorDown();
        return 0;
    }
    
    return &SKKState::TopState;
}

// ======================================================================
// level 2 (sub of Primary)：かな入力
// ======================================================================
State SKKState::KanaInput(const Event& event) {
    const SKKEvent& param = event.Param();

    switch(event) {
    case INIT_EVENT:
	return State::ShallowHistory(&SKKState::Hirakana);

    case EXIT_EVENT:
	return State::SaveHistory();

    case SKK_CHAR:
	if(!editor_->CanConvert(param.code)) {
	    if(param.IsSwitchToAscii()) {
		return State::Transition(&SKKState::Ascii);
	    }

	    if(param.IsSwitchToJisx0208Latin()) {
		return State::Transition(&SKKState::Jisx0208Latin);
	    }

	    if(param.IsEnterAbbrev()) {
		return State::Transition(&SKKState::AsciiEntry);
	    }

	    if(param.IsEnterJapanese()) {
		return State::Transition(&SKKState::KanaEntry);
	    }
	}

	if(param.IsUpperCases()) {
	    return State::Forward(&SKKState::KanaEntry);
	}

        // キー修飾がない場合のみローマ字かな変換を実施する
        if(param.IsPlain()) {
            editor_->HandleChar(param.code, param.IsDirect());
        } else {
            // *** FIXME ***
            // 未確定文字列がある状態で Ctrl-P などが押された場合、
            // 次回入力時にゴミとなるのでクリアしておく
            // この問題はもう少しスマートに解決したい...
            editor_->Reset();
        }

        return 0;
    }

    return &SKKState::Primary;
}

// ======================================================================
// level 3 (sub of KanaInput)：ひらかな
// ======================================================================
State SKKState::Hirakana(const Event& event) {
    const SKKEvent& param = event.Param();

    switch(event) {
    case ENTRY_EVENT:
        editor_->SelectInputMode(HirakanaInputMode);
	return 0;

    case SKK_CHAR:
	if(param.IsToggleKana()) {
	    return State::Transition(&SKKState::Katakana);
	}

	if(param.IsToggleJisx0201Kana()) {
	    return State::Transition(&SKKState::Jisx0201Kana);
	}
    }

    return &SKKState::KanaInput;
}

// ======================================================================
// level 3 (sub of KanaInput)：カタカナ
// ======================================================================
State SKKState::Katakana(const Event& event) {
    const SKKEvent& param = event.Param();

    switch(event) {
    case ENTRY_EVENT:
        editor_->SelectInputMode(KatakanaInputMode);
	return 0;

    case SKK_CHAR:
	if(param.IsToggleKana()) {
	    return State::Transition(&SKKState::Hirakana);
	}

	if(param.IsToggleJisx0201Kana()) {
	    return State::Transition(&SKKState::Jisx0201Kana);
	}
    }

    return &SKKState::KanaInput;
}

// ======================================================================
// level 3 (sub of KanaInput)：半角カタカナ
// ======================================================================
State SKKState::Jisx0201Kana(const Event& event) {
    const SKKEvent& param = event.Param();

    switch(event) {
    case ENTRY_EVENT:
        editor_->SelectInputMode(Jisx0201KanaInputMode);
	return 0;

    case SKK_CHAR:
	if(param.IsToggleKana() || param.IsToggleJisx0201Kana()) {
	    return State::Transition(&SKKState::Hirakana);
	}
    }

    return &SKKState::KanaInput;
}

// ======================================================================
// level 2 (sub of Primary)：Latin 入力
// ======================================================================
State SKKState::LatinInput(const Event& event) {
    const SKKEvent& param = event.Param();

    switch(event) {
    case SKK_JMODE:
	return State::Transition(&SKKState::Hirakana);

    case SKK_CHAR:
        editor_->HandleChar(param.code, param.IsDirect());
	return 0;
    }

    return &SKKState::Primary;
}

// ======================================================================
// level 2 (sub of LatinInput)：ASCII
// ======================================================================
State SKKState::Ascii(const Event& event) {
    switch(event) {
    case ENTRY_EVENT:
        editor_->SelectInputMode(AsciiInputMode);
	return 0;
    }

    return &SKKState::LatinInput;
}

// ======================================================================
// level 2 (sub of LatinInput)：全角英数
// ======================================================================
State SKKState::Jisx0208Latin(const Event& event) {
    switch(event) {
    case ENTRY_EVENT:
        editor_->SelectInputMode(Jisx0208LatinInputMode);
	return 0;
    }

    return &SKKState::LatinInput;
}
