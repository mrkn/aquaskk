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

#include "SKKInputQueue.h"
#include "SKKRomanKanaConverter.h"
#include "jconv.h"

SKKInputQueue::SKKInputQueue(SKKInputQueueObserver* observer)
    : observer_(observer), mode_(HirakanaInputMode) {}

void SKKInputQueue::SelectInputMode(SKKInputMode mode) {
    mode_ = mode;

    Clear();
}

void SKKInputQueue::AddChar(char code) {
    std::string fixed;

    convert(code, fixed);

    notify(fixed, code);
}

void SKKInputQueue::RemoveChar() {
    if(IsEmpty()) return;

    queue_.erase(queue_.size() - 1);

    notify();
}

void SKKInputQueue::Terminate() {
    if(IsEmpty()) return;

    std::string fixed;

    convert(5, fixed, true);

    Clear();

    notify(fixed);
}

void SKKInputQueue::Clear() {
    queue_.clear();
}

bool SKKInputQueue::IsEmpty() const {
    return queue_.empty();
}

const std::string& SKKInputQueue::QueueString() const {
    return queue_;
}

bool SKKInputQueue::CanConvert(char code) const {
    SKKRomanKanaConverter& converter = SKKRomanKanaConverter::theInstance();
    std::string tmp_queue(queue_);
    std::string fixed;
    std::string next;

    switch(mode_) {
    case HirakanaInputMode:
    case KatakanaInputMode:
    case Jisx0201KanaInputMode:
	// ローマ字 → かな変換
	tmp_queue += std::tolower(code);
	return converter.Execute(mode_, tmp_queue, fixed, next);

    default:
        return false;
    }
}

// ------------------------------------------------------------

void SKKInputQueue::convert(char code, std::string& fixed, bool terminate) {
    SKKRomanKanaConverter& converter = SKKRomanKanaConverter::theInstance();
    std::string next;

    fixed.clear();

    switch(mode_) {
    case AsciiInputMode:
        fixed += code;
	break;

    case HirakanaInputMode:
    case KatakanaInputMode:
    case Jisx0201KanaInputMode:
	// ローマ字 → かな変換
	queue_ += std::tolower(code);
	converter.Execute(mode_, queue_, fixed, next);
	queue_ = next;

        if(terminate) {
            fixed.erase(fixed.size() - 1);
        }
	break;

    case Jisx0208LatinInputMode:
	// ASCII → 全角英数変換
	queue_ += code;
	jconv::ascii_to_jisx0208_latin(queue_, fixed);
	queue_.clear();
	break;

    default:
        break;
    }
}

void SKKInputQueue::notify(const std::string& fixed, char code) {
    observer_->SKKInputQueueUpdate(fixed, queue_, code);
}
