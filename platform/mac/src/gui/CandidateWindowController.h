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

#ifndef CandidateWindowController_h
#define CandidateWindowController_h

#include <Cocoa/Cocoa.h>

@class CandidateView;

@interface CandidateWindowController : NSWindowController {
    IBOutlet CandidateView* view_;
    NSString* labels_;
}

+ (CandidateWindowController*)sharedController;
- (void)prepareWithFont:(NSFont*)font labels:(NSString*)labels;
- (void)setCandidates:(NSArray*)candidates selectedIndex:(int)cursor;
- (void)setPage:(NSRange)page;
- (void)show;
- (void)hide;
- (int)indexOfLabel:(char)label;
- (id)createCandidateCell;

@end

#endif
