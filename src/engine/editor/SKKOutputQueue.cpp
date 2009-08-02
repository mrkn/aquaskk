/* -*- C++ -*-

  MacOS X implementation of the SKK input method.

  Copyright (C) 2009 Tomotaka SUWA <t.suwa@mac.com>

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

#include "SKKOutputQueue.h"

void SKKOutputQueue::Add(const SKKContextBuffer& context) {
    // *** 重要な出力制御 ***
    //
    // No.|prev    |context |制御
    // ---+--------+--------+--------------------
    //  1 |empty   |empty   |何もしない
    // ---+--------+--------+--------------------
    //  2 |!empty  |empty   |context を出力
    // ---+--------+--------+--------------------
    //  3 |empty   |!empty  |context を出力
    // ---+--------+--------+--------------------
    //  4 |!empty  |!empty  |context を出力
    // ---+--------+--------+--------------------
    //
    // 1 のケースを弾かないと、finder や iTunes で不具合が出る 
    //
    if(prev_ == context && context == empty_) return;

    queue_.push_back(context);
}

void SKKOutputQueue::Output(SKKFrontEnd* frontend,
                            SKKDynamicCompletor* completor,
                            SKKAnnotator* annotator) {
    if(queue_.empty()) return;

    for(OutputQueueIterator iter = queue_.begin(); iter != queue_.end(); ++ iter) {
        iter->Output(frontend, completor, annotator);
    }

    prev_ = queue_.back();
    if(!prev_.IsComposing()) {
        prev_.Clear();
    }

    queue_.clear();
}
