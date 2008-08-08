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
// level 1：単語削除
// ======================================================================
State SKKState::EntryRemove(const Event& event) {
    const SKKEvent& param = event.Param();

    switch(event) {
    case ENTRY_EVENT:
        inputMode_ = editor_->InputMode();

        editor_->SetStateEntryRemove();
        editor_->SelectInputMode(AsciiInputMode);
	return 0;

    case EXIT_EVENT:
        editor_->SelectInputMode(inputMode_);
        return 0;

    case SKK_ENTER:
        if(editor_->Entry().EntryString() == "yes") {
            editor_->Commit();
            return State::Transition(&SKKState::KanaInput);
        }

        // yes 以外なら SKK_CANCEL と同じ処理↓

    case SKK_CANCEL:
        return State::Transition(&SKKState::SelectCandidate);

    case SKK_BACKSPACE:
        editor_->HandleBackSpace();
        return 0;

    case SKK_CHAR:
        if(param.IsPlain()) {
            editor_->HandleChar(param.code);
            return 0;
        }
    }

    return &SKKState::TopState;
}
