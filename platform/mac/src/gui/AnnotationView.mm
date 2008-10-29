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

#include "AnnotationView.h"

@implementation AnnotationView

- (id)init {
    self = [super initWithFrame:NSMakeRect(0, 0, 254, 62)];
    if(self) {
        NSTextTab* tab = [[NSTextTab alloc] initWithType:NSLeftTabStopType location:24.0];
        listStyle_ = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
        [listStyle_ addTabStop:tab];
        [listStyle_ setHeadIndent:[tab location]];
        [tab release];

        textView_ = [[NSTextView alloc] initWithFrame:[self frame]];
        [textView_ setEditable:NO];
        [textView_ setBackgroundColor:[NSColor colorWithDeviceRed:1.0 green:1.0 blue:0.9 alpha:1.0]];

        scrollView_ = [[NSScrollView alloc] initWithFrame:[textView_ frame]];
        [scrollView_ setHasVerticalScroller:YES];
        [[scrollView_ verticalScroller] setControlSize:NSSmallControlSize];
        [scrollView_ setAutohidesScrollers:YES];
        [scrollView_ setDocumentView:textView_];
        [scrollView_ setAutoresizingMask:NSViewMinXMargin|NSViewMaxXMargin|NSViewMinYMargin|NSViewMaxYMargin];

        [self addSubview:scrollView_];
        [self setFrame:NSMakeRect(0, 0, 256, 64)];
    }

    return self;
}

- (void)dealloc {
    [scrollView_ release];
    [textView_ release];
    [listStyle_ release];

    [super dealloc];
}

- (void)setAnnotation:(NSString*)string {
    NSArray* array = [string componentsSeparatedByString:@"\n"];

    if(array) {
        NSString* first = [array objectAtIndex:0];
        [textView_ setString:@""];

        // *** FIXME *** 先頭行の長さを調べる
        if([first length] != 1) {
            // oops!
            NSString* rep = [string stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%C", 0xe027]
                                    withString:@"[文]"];
            rep = [rep stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%C", 0xe021]
                                    withString:@"[ー]"];
            rep = [rep stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%C", 0xe022]
                                    withString:@"[二]"];
            rep = [rep stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%C", 0xe023]
                                    withString:@"[三]"];
            rep = [rep stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%C", 0xe024]
                                    withString:@"[四]"];
            rep = [rep stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%C", 0xe025]
                                    withString:@"[五]"];
            rep = [rep stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%C", 0xe026]
                                    withString:@"[六]"];
            [textView_ setString:rep];
        } else {
            // リストタイプだと想定する
            NSMutableArray* tmp = [NSMutableArray arrayWithCapacity:0];

            for(int i = 0; i + 1 < [array count]; i += 2) {
                [tmp addObject:[NSString stringWithFormat:@"%@\t%@",
                                         [array objectAtIndex:i],
                                         [array objectAtIndex:i + 1]]];
            }

            NSMutableAttributedString* attr = [[NSMutableAttributedString alloc]
                                                  initWithString:[tmp componentsJoinedByString:@"\n"]];
            [attr addAttribute:NSParagraphStyleAttributeName value:listStyle_ range:NSMakeRange(0, [attr length])];

            [textView_ setEditable:YES];
            [textView_ insertText:attr];
            [textView_ setEditable:NO];

            [attr release];
        }
    }
}

- (BOOL)hasAnnotation {
    return [[textView_ string] length] ? YES : NO;
}

- (void)drawRect:(NSRect)rect {
    [[NSColor windowFrameColor] setStroke];
    [NSBezierPath strokeRect:[self frame]];
}

@end
