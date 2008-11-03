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
    notify(convert(code), code);
}

void SKKInputQueue::RemoveChar() {
    if(IsEmpty()) return;

    queue_.erase(queue_.size() - 1);

    notify();
}

void SKKInputQueue::Terminate() {
    if(IsEmpty()) return;

    std::string fixed = terminate();

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
    SKKRomanKanaConversionResult result;
    std::string tmp_queue(queue_);

    switch(mode_) {
    case HirakanaInputMode:
    case KatakanaInputMode:
    case Jisx0201KanaInputMode:
	// ローマ字 → かな変換
	tmp_queue += std::tolower(code);
	return converter.Convert(mode_, tmp_queue, result);

    case Jisx0208LatinInputMode:
    case AsciiInputMode:
        break;

    case InvalidInputMode:
        assert(false);
    }

    return false;
}

// ------------------------------------------------------------

std::string SKKInputQueue::convert(char code) {
    SKKRomanKanaConverter& converter = SKKRomanKanaConverter::theInstance();
    SKKRomanKanaConversionResult result;

    switch(mode_) {
    case AsciiInputMode:
        result.output += code;
	break;

    case HirakanaInputMode:
    case KatakanaInputMode:
    case Jisx0201KanaInputMode:
	// ローマ字 → かな変換
	queue_ += std::tolower(code);
	converter.Convert(mode_, queue_, result);
	queue_ = result.next;
	break;

    case Jisx0208LatinInputMode:
	// ASCII → 全角英数変換
	queue_ += code;
	jconv::ascii_to_jisx0208_latin(queue_, result.output);
	queue_.clear();
	break;

    case InvalidInputMode:
        assert(false);
    }

    return result.output;
}

std::string SKKInputQueue::terminate() {
    SKKRomanKanaConverter& converter = SKKRomanKanaConverter::theInstance();
    SKKRomanKanaConversionResult result;

    switch(mode_) {
    case HirakanaInputMode:
    case KatakanaInputMode:
    case Jisx0201KanaInputMode:
	// ローマ字 → かな変換
	converter.Convert(mode_, queue_, result);
	break;

    case Jisx0208LatinInputMode:
    case AsciiInputMode:
        break;

    case InvalidInputMode:
        assert(false);
    }

    return result.intermediate;
}

void SKKInputQueue::notify(const std::string& fixed, char code) {
    observer_->SKKInputQueueUpdate(fixed, queue_, code);
}
