/* -*- ObjC -*-

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

#include "MacCandidateWindow.h"
#include "CandidateWindowController.h"
#include "CandidateView.h"
#include "CandidateCell.h"
#include "SKKConstVars.h"
#include "SKKFrontEnd.h"
#include "utf8util.h"

MacCandidateWindow::MacCandidateWindow(SKKFrontEnd* frontend) : active_(false), frontend_(frontend) {
    controller_ = [CandidateWindowController sharedController];
    candidates_ = [[NSMutableArray alloc] initWithCapacity:0];
    reloadUserDefaults();
}

MacCandidateWindow::~MacCandidateWindow() {
    [candidates_ release];
}

void MacCandidateWindow::Setup(SKKCandidateIterator begin, SKKCandidateIterator end, std::vector<int>& pages) {
    reloadUserDefaults();

    std::vector<int> cell_width;

    CandidateCell* cell = [controller_ createCandidateCell];
    int width;

    // 全ての cell の幅を求める
    while(begin != end) {
        std::string candidate(begin->Variant());

        // UTF-8 で二文字以下ならデフォルトサイズを使う(最適化)
        if(utf8::length(candidate) < 3) {
            width = [cell defaultSize].width;
        } else {
            NSString* string = [NSString stringWithUTF8String:candidate.c_str()];

            [cell setString:string withLabel:'A'];

            width = [cell size].width;
        }

        cell_width.push_back(width + [CandidateView cellSpacing]);

        ++ begin;
    }

    unsigned limit = ([cell defaultSize].width + [CandidateView cellSpacing]) * cellCount_;
    int offset = 0;

    // 候補ウィンドウに表示可能な cell の数を求める
    pages.clear();
    do {
        unsigned size = 0;
        int count = 0;
        while(offset < cell_width.size()) {
            if(limit < size + cell_width[offset]) {
                if(size == 0) {
                    ++ offset;
                    count = 1;
                }
                break;
            }
            size += cell_width[offset];
            ++ offset;
            ++ count;
        }

        pages.push_back(count);
        count = 0;
        size = 0;
    } while(offset < cell_width.size());

    [cell release];
}

void MacCandidateWindow::Show(SKKCandidateIterator begin, SKKCandidateIterator end,
				    int cursor, int page_pos, int page_max) {
    [candidates_ removeAllObjects];

    for(SKKCandidateIterator curr = begin; curr != end; ++ curr) {
        std::string candidate(curr->Variant());
	[candidates_ addObject:[NSString stringWithUTF8String:candidate.c_str()]];
    }

    page_ = NSMakeRange(page_pos, page_max);
    cursor_ = cursor;

    active_ = true;

    Activate();
}

void MacCandidateWindow::Hide() {
    Deactivate();

    active_ = false;
}

void MacCandidateWindow::Activate() {
    if(!active_) return;

    prepareWindow();

    [controller_ setCandidates:candidates_ selectedIndex:cursor_];
    [controller_ setPage:page_];
    [controller_ show];
}

void MacCandidateWindow::Deactivate() {
    if(!active_) return;

    [controller_ hide];
}

int MacCandidateWindow::LabelIndex(char label) {
    return [controller_ indexOfLabel:label];
}

void MacCandidateWindow::reloadUserDefaults() {
    NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];

    NSString* fontName = [defaults stringForKey:SKKUserDefaultKeys::candidate_window_font_name];
    float fontSize = [defaults floatForKey:SKKUserDefaultKeys::candidate_window_font_size];

    NSFont* font = [NSFont fontWithName:fontName size:fontSize];

    NSString* labels = [defaults stringForKey:SKKUserDefaultKeys::candidate_window_labels];
    cellCount_ = [labels length];

    [controller_ prepareWithFont:font labels:labels];
}

void MacCandidateWindow::prepareWindow() {
    // ウィンドウレベル
    [[controller_ window] setLevel:frontend_->WindowLevel()];

    // カーソル位置を含むディスプレイの矩形を取得
    std::pair<int, int> position = frontend_->WindowPosition();
    CGDirectDisplayID disp[1];
    CGDisplayCount count;
    CGGetDisplaysWithPoint(CGPointMake(position.first, position.second), 1, disp, &count);
    CGRect screen = CGDisplayBounds(disp[0]);

    screen.size.width += screen.origin.x;

    NSSize window_size = [[controller_ window] frame].size;

    float window_x = position.first;
    float window_y = position.second;

    if(window_x + window_size.width > screen.size.width) {
	window_x = screen.size.width - window_size.width;
    }

    [[controller_ window] setFrameTopLeftPoint:NSMakePoint(window_x, window_y)];
}
