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
- (void)prepareLayer;
- (void)prepareAnimation;
- (void)prepareWindow;
- (void)updateFrame;
- (void)setImage:(id)image;
@end

@implementation InputModeWindowController

+ (InputModeWindowController*)sharedController {
    static InputModeWindowController* obj =  [[InputModeWindowController alloc] init];

    return obj;
}

- (id)init {
    if(self = [super initWithWindowNibName:@"InputModeWindow"]) {
        inputMode_ = HirakanaInputMode;
        modeIcons_ = 0;

        [self prepareLayer];
        [self prepareAnimation];
        [self prepareWindow];
    }

    return self;
}

- (void)dealloc {
    [modeIcons_ release];
    [animation_ release];

    [super dealloc];
}

- (void)setModeIcons:(NSDictionary*)icons {
    [icons retain];

    if(modeIcons_) {
        [modeIcons_ release];
    }

    modeIcons_ = icons;
}

- (void)changeMode:(SKKInputMode)mode {
    inputMode_ = mode;

    [self updateFrame];

    NSImage* image = [modeIcons_ objectForKey:[NSNumber numberWithInt:mode]];
    NSBitmapImageRep* rep = [NSBitmapImageRep imageRepWithData:[image TIFFRepresentation]]; 

    [self setImage:(id)[rep CGImage]];
}

- (SKKInputMode)currentInputMode {
    return inputMode_;
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

- (void)prepareWindow {
    NSWindow* window = [self window];

    [window setBackgroundColor:[NSColor clearColor]];
    [window setOpaque:NO];
    [window setIgnoresMouseEvents:YES];
}

- (void)updateFrame {
    NSRect rect = [[self window] frame];
    NSSize iconSize = rect.size;
    NSImage* icon = [modeIcons_ objectForKey:[NSNumber numberWithInt:inputMode_]];
    NSArray* reps = [icon representations];

    if([reps count]) {
        // NSImage の - (NSSize)size ではなく、ピクセルサイズを取得する
        NSImageRep* image = [reps objectAtIndex:0];
        iconSize = NSMakeSize(image.pixelsWide, image.pixelsHigh);
    }

    // ppc では、背景を clearColor にした NSWindow の矩形サイズが、いつ
    // のまにか 0*0 になってしまうことがある(QuarzDebug による調査)ため、
    // 表示する直前にウィンドウの矩形を設定し直す
    if(!NSEqualSizes(rect.size, iconSize)) {
        // 画像サイズに応じて原点をオフセットする
        rect.origin.y += rect.size.height - iconSize.height;
        rect.size = iconSize;

        // 矩形設定時の画像ちらつきを防ぐ
        [self setImage:0];

        [[self window] setFrame:rect display:YES];
    }
}

- (void)setImage:(id)image {
    [CATransaction begin];
    [CATransaction setValue:[NSNumber numberWithFloat:0.0]
                   forKey:kCATransactionAnimationDuration];

    rootLayer_.contents = image;

    [CATransaction commit];
}

@end
