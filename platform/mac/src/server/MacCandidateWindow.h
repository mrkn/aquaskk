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

#ifndef MacCandidateWindowMacOSX_h
#define MacCandidateWindowMacOSX_h

#include "SKKCandidateWindow.h"

@class CandidateWindowController;
class SKKFrontEnd;

class MacCandidateWindow : public SKKCandidateWindow {
    bool active_;
    SKKFrontEnd* frontend_;
    NSMutableArray* candidates_;
    NSRange page_;
    int cursor_;
    int cellCount_;
    CandidateWindowController* controller_;

    void reloadUserDefaults();
    void prepareWindow();

public:
    MacCandidateWindow(SKKFrontEnd* frontend);
    virtual ~MacCandidateWindow();

    virtual void Setup(SKKCandidateIterator begin, SKKCandidateIterator end, std::vector<int>& pages);
    virtual void Show(SKKCandidateIterator begin, SKKCandidateIterator end, int cursor, int page_pos, int page_max);
    virtual void Hide();
    virtual void Activate();
    virtual void Deactivate();
    virtual int LabelIndex(char label);
};

#endif
