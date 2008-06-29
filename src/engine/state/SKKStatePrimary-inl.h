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
    const SKKEvent& param = event.Param();

    switch(event) {
    case INIT_EVENT:
	return State::Initial(&SKKState::KanaInput);

    case SKK_JMODE:
        editor_->Commit();
        return 0;

    case SKK_ENTER:
	editor_->FinishRegistration();
	return 0;

    case SKK_CANCEL:
	editor_->AbortRegistration();
	return 0;

    case SKK_PASTE:
	editor_->Paste();
	return 0;

    case SKK_PING:
        editor_->ActivateInputMode();
        return 0;

    case SKK_BACKSPACE:
    case SKK_DELETE:
    case SKK_LEFT:
    case SKK_RIGHT:
    case SKK_UP:
    case SKK_DOWN:
        editor_->Input(param);
        return 0;

    case SKK_CHAR:
	if(!editor_->WillConvert(param)) {
	    if(param.IsSwitchToAscii()) {
		return State::Transition(&SKKState::Ascii);
	    }

	    if(param.IsSwitchToJisx0208Latin()) {
		return State::Transition(&SKKState::Jisx0208Latin);
	    }

	    if(param.IsEnterAbbrev()) {
                editor_->PushEditor(&composingEditor_);
		return State::Transition(&SKKState::AsciiEntry);
	    }

	    if(param.IsEnterJapanese()) {
                editor_->PushEditor(&composingEditor_);
		return State::Transition(&SKKState::KanaEntry);
	    }
	}

	if(param.IsUpperCases()) {
            editor_->PushEditor(&composingEditor_);
	    return State::Forward(&SKKState::KanaEntry);
	}

	if(param.IsPlain()) {
            editor_->Input(param);
        } else {
            editor_->Reset();
        }

	return 0;
    }
    
    return &SKKState::TopState;
}

// ======================================================================
// level 2 (sub of Primary)：かな入力
// ======================================================================
State SKKState::KanaInput(const Event& event) {
    switch(event) {
    case INIT_EVENT:
	return State::ShallowHistory(&SKKState::Hirakana);

    case EXIT_EVENT:
	return State::SaveHistory();
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

    case EXIT_EVENT:
        editor_->Cancel();
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

    case EXIT_EVENT:
        editor_->Cancel();
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

    case EXIT_EVENT:
        editor_->Cancel();
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
    case EXIT_EVENT:
        editor_->Cancel();
        return 0;

    case SKK_JMODE:
	return State::Transition(&SKKState::Hirakana);

    case SKK_CHAR:
        editor_->Input(param);
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
