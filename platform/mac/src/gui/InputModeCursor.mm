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

#include "InputModeCursor.h"

@implementation InputModeCursor

+ (InputModeCursor*)sharedCursor {
    static InputModeCursor* obj =  [[InputModeCursor alloc] init];

    return obj;
}

- (id)init {
    self = [super initWithWindowNibName:@"InputModeCursor"];

    if(self) {
        [[self window] setFrame:NSMakeRect(0, 0, 8, 4) display:YES];
        [[self window] setAlphaValue:0.8];
        active_ = NO;
    }

    return self;
}

- (void)changeMode:(SKKInputMode)mode {
    NSColor* color;

    switch(mode) {
    case HirakanaInputMode:
        color = [NSColor orangeColor];
        break;

    case KatakanaInputMode:
        color = [NSColor colorWithDeviceRed:0.2 green:0.4 blue:0.0 alpha:1.0];
        break;

    case Jisx0201KanaInputMode:
        color = [NSColor purpleColor];
        break;

    case AsciiInputMode:
        color = [NSColor grayColor];
        break;

    case Jisx0208LatinInputMode:
        color = [NSColor colorWithDeviceRed:0.9 green:0.9 blue:0.0 alpha:1.0];
        break;
    }

    [[self window] setBackgroundColor:color];
}

- (void)show:(NSPoint)topleft level:(int)level {
    [[self window] setFrameOrigin:topleft];
    [[self window] setLevel:level];

    [self showWindow:nil];

    active_ = YES;
}

- (void)hide {
    [[self window] orderOut:nil];

    active_ = NO;
}

@end
