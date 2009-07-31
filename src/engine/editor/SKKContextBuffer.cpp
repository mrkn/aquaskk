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

#include "SKKContextBuffer.h"
#include "SKKFrontEnd.h"
#include "SKKDynamicCompletor.h"
#include "SKKAnnotator.h"
#include "utf8util.h"
#include <iostream>

SKKContextBuffer::SKKContextBuffer() : cursor_(0), completion_cursor_(0) {}

void SKKContextBuffer::Fix(const std::string& str) {
    fixed_ += str;
}

void SKKContextBuffer::Compose(const std::string& str, int cursor) {
    utf8::push(composing_, str, cursor_);
    cursor_ += cursor;
}

void SKKContextBuffer::Compose(const std::string& str, const std::string& completion, int cursor) {
    completion_cursor_ = utf8::length(composing_) + cursor_;
    Compose(str, cursor);

    completion_ = completion;
}

bool SKKContextBuffer::IsComposing() const {
    return !composing_.empty();
}

void SKKContextBuffer::Output(SKKFrontEnd* frontend, SKKDynamicCompletor* completor, SKKAnnotator* annotator) {
    frontend->InsertString(fixed_);

    frontend->ComposeString(composing_, cursor_);

    if(completor) {
        completor->Update(completion_, completion_cursor_);

        if(!completion_.empty()) {
            completor->Show();
        } else {
            completor->Hide();
        }
    }

    if(annotator) {
        if(!candidate_.IsEmpty()) {
            annotator->Update(candidate_, utf8::left(composing_, cursor_));
            annotator->Show();
        } else {
            annotator->Hide();
        }
    }
}

void SKKContextBuffer::SetEntry(const SKKEntry& entry) {
    entry_ = entry;
}

void SKKContextBuffer::SetCandidate(const SKKCandidate& candidate) {
    candidate_ = candidate;
}

void SKKContextBuffer::Clear() {
    fixed_.clear();
    composing_.clear();
    completion_.clear();
    cursor_ = 0;

    SetEntry(SKKEntry());
    SetCandidate(SKKCandidate());
}

const SKKEntry& SKKContextBuffer::Entry() const {
    return entry_;
}

const SKKCandidate& SKKContextBuffer::Candidate() const {
    return candidate_;
}

void SKKContextBuffer::Dump() const {
    std::string tmp(composing_);

    utf8::push(tmp, "[I]", cursor_);
    
    std::cerr << "fixed(" << fixed_ << "), composing(" << tmp << ")" << std::endl;
}

// ----------------------------------------------------------------------
 
bool operator==(const SKKContextBuffer& left, const SKKContextBuffer& right) {
    return left.fixed_ == right.fixed_
        && left.composing_ == right.composing_
        && left.completion_ == right.completion_
        && left.cursor_ == right.cursor_
        && left.completion_cursor_ == right.completion_cursor_
        && left.entry_ == right.entry_
        && left.candidate_ == right.candidate_;
}

bool operator!=(const SKKContextBuffer& left, const SKKContextBuffer& right) {
    return !(left == right);
}
