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

#include "InputModeWindowController.h"

@interface InputModeWindowController (Local)
- (void)prepareIcon;
- (void)prepareLayer;
- (void)prepareAnimation;
- (void)updateFrame;
@end

@implementation InputModeWindowController

+ (InputModeWindowController*)sharedController {
    static InputModeWindowController* obj =  [[InputModeWindowController alloc] init];

    return obj;
}

- (id)init {
    if(self = [super initWithWindowNibName:@"InputModeWindow"]) {
        [self prepareIcon];
        [self prepareLayer];
        [self prepareAnimation];
    }

    return self;
}

- (void)dealloc {
    [modeIcons_ release];
    [animation_ release];

    [super dealloc];
}

- (void)changeMode:(SKKInputMode)mode {
    inputMode_ = mode;

    [self updateFrame];

    NSImage* image = [modeIcons_ objectForKey:[NSNumber numberWithInt:mode]];
    NSBitmapImageRep* rep = [NSBitmapImageRep imageRepWithData:[image TIFFRepresentation]]; 

    [CATransaction begin];
    [CATransaction setValue:[NSNumber numberWithFloat:0.0]
                   forKey:kCATransactionAnimationDuration];

    rootLayer_.contents = (id)[rep CGImage];

    [CATransaction commit];
}

- (void)show:(NSPoint)topleft level:(int)level {
    [self updateFrame];

    [[self window] setFrameTopLeftPoint:topleft];
    [[self window] setLevel:level];
    [self showWindow:nil];

    [rootLayer_ addAnimation:animation_ forKey:@"fadeOut"];
}

- (void)hide {
    [[self window] orderOut:nil];
}

@end

@implementation InputModeWindowController (Local)

- (void)prepareIcon {
    struct {
        int mode;
        NSString* name;
    } icon[] = {
        { HirakanaInputMode,		@"AquaSKK-Hirakana.png" },
        { KatakanaInputMode,		@"AquaSKK-Katakana.png" },
        { Jisx0201KanaInputMode,	@"AquaSKK-Jisx0201Kana.png" },
        { AsciiInputMode,		@"AquaSKK-Ascii.png" },
        { Jisx0208LatinInputMode,	@"AquaSKK-Jisx0208Latin.png" },
        { 0,				0 }
    };

    NSMutableDictionary* icons = [[NSMutableDictionary alloc] initWithCapacity:0];

    for(int i = 0; icon[i].name != 0; ++ i) {
        [icons setObject:[NSImage imageNamed:icon[i].name]
               forKey:[NSNumber numberWithInt:icon[i].mode]];
    }

    inputMode_ = HirakanaInputMode;
    modeIcons_ = [[NSDictionary alloc] initWithDictionary:icons];

    [icons release];
}
        
- (void)prepareLayer {
    rootLayer_ = [CALayer layer];
    rootLayer_.opacity = 0.0;

    NSView* view = [[self window] contentView];

    [view setLayer:rootLayer_];
    [view setWantsLayer:YES];
}

- (void)prepareAnimation {
    animation_ = [[CABasicAnimation animationWithKeyPath:@"opacity"] retain];

    animation_.duration = 2.0;
    animation_.fromValue = [NSNumber numberWithFloat:1.0];
    animation_.toValue = [NSNumber numberWithFloat:0];
    animation_.timingFunction = [CAMediaTimingFunction functionWithControlPoints:0.5 :0.0 :0.5 :0.0];
}

- (void)updateFrame {
    NSImage* icon = [modeIcons_ objectForKey:[NSNumber numberWithInt:inputMode_]];

    NSRect rect;
    rect.origin = NSZeroPoint;
    rect.size = [icon size];

    // ppc では、背景を clearColor にした NSWindow の矩形サイズが、いつ
    // のまにか 0*0 になってしまうことがある(QuarzDebug による調査)ため、
    // 表示する直前にウィンドウの矩形を設定し直す
    [[self window] setFrame:rect display:NO];
}

@end
