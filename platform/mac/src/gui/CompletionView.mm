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

#include "CompletionView.h"

@interface CompletionView (Local)

- (NSRect)completionRect;

@end

@implementation CompletionView

- (id)init {
    self = [super initWithFrame:NSMakeRect(0, 0, 0, 0)];
    if(self) {
        completion_ = nil;

        NSMutableDictionary* dictionary = [NSMutableDictionary dictionaryWithCapacity:0];

        [dictionary setObject:[NSFont systemFontOfSize:0.0] forKey:NSFontAttributeName];

        attributes_ = [dictionary retain];
    }
    return self;
}

- (void)dealloc {
    if(completion_) {
        [completion_ release];
    }

    [attributes_ release];

    [super dealloc];
}

- (void)setCompletion:(NSString*)completion {
    if(completion_) {
        [completion_ release];
    }

    completion_ = [completion retain];
    
    [self setFrame:[self completionRect]];
    [self setNeedsDisplay:YES];
}

- (void)drawRect:(NSRect)rect {
    NSRect frame = [self completionRect];

    [[NSColor controlColor] setFill];
    [[NSColor controlShadowColor] setStroke];

    NSRectFill(frame);
    [NSBezierPath strokeRect:frame];

    [completion_ drawAtPoint:NSMakePoint(3, 4) withAttributes:attributes_];
}

@end

@implementation CompletionView (Local)

- (NSRect)completionRect {
    NSRect rect;

    rect.origin = NSMakePoint(0, 0);
    rect.size = [completion_ sizeWithAttributes:attributes_];
    rect.size.width += 8;
    rect.size.height += 8;

    return rect;
}

@end
