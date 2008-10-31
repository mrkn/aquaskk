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

@interface AnnotationView (Local)

- (NSAttributedString*)createHeader:(NSString*)string;
- (void)setDefinitiveAnnotation:(NSString*)string;
- (void)setOptionalAnnotation:(NSString*)string;
- (NSString*)normalizeString:(NSString*)string;
- (void)insertString:(NSString*)string withStyle:(NSParagraphStyle*)style;

@end

@implementation AnnotationView

- (id)init {
    self = [super initWithFrame:NSMakeRect(0, 0, 254, 62)];

    if(self) {
        NSTextTab* tab = [[NSTextTab alloc] initWithType:NSLeftTabStopType location:28.0];
        listStyle_ = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
        [listStyle_ addTabStop:tab];
        [listStyle_ setFirstLineHeadIndent:10.0];
        [listStyle_ setHeadIndent:[tab location]];
        [tab release];

        defaultStyle_ = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
        [defaultStyle_ setFirstLineHeadIndent:10.0];
        [defaultStyle_ setHeadIndent:10.0];

        textView_ = [[NSTextView alloc] initWithFrame:[self frame]];
        [textView_ setEditable:NO];
        [textView_ setBackgroundColor:[NSColor colorWithDeviceRed:1.0 green:1.0 blue:0.94 alpha:1.0]];
        [textView_ setTextContainerInset:NSMakeSize(0, 2)];

        scrollView_ = [[NSScrollView alloc] initWithFrame:[self frame]];
        [scrollView_ setHasVerticalScroller:YES];
        [[scrollView_ verticalScroller] setControlSize:NSSmallControlSize];
        [scrollView_ setDocumentView:textView_];
        [scrollView_ setAutoresizingMask:NSViewMinXMargin|NSViewMaxXMargin|NSViewMinYMargin|NSViewMaxYMargin];

        [self addSubview:scrollView_];
        [self setFrame:NSMakeRect(0, 0, 256, 64)];

        [textView_ setFrameSize:[scrollView_ contentSize]];

        header1_ = [self createHeader:@"意味・語源"];
        header2_ = [self createHeader:@"SKK アノテーション"];
    }

    return self;
}

- (void)dealloc {
    [scrollView_ release];
    [textView_ release];
    [defaultStyle_ release];
    [listStyle_ release];
    [header1_ release];
    [header2_ release];

    [super dealloc];
}

- (void)setAnnotation:(NSString*)definition optional:(NSString*)annotation {
    [textView_ setString:@""];
    [textView_ setEditable:YES];

    [self setDefinitiveAnnotation:definition];
    [self setOptionalAnnotation:annotation];

    [textView_ setEditable:NO];

    NSPoint top;

    if([textView_ isFlipped]) {
        top = NSMakePoint(0.0, 0.0);
    } else {
        top = NSMakePoint(0.0, NSMaxY([textView_ frame]) - NSHeight([textView_  bounds]));
    }

    [textView_ scrollPoint:top];
}

- (BOOL)hasAnnotation {
    return [[textView_ string] length] ? YES : NO;
}

- (void)drawRect:(NSRect)rect {
    [[NSColor windowFrameColor] setStroke];
    [NSBezierPath strokeRect:[self frame]];
}

@end

@implementation AnnotationView (Local)

- (NSAttributedString*)createHeader:(NSString*)string {
    NSRange range = NSMakeRange(0, [string length]);
    NSMutableAttributedString* header = [[NSMutableAttributedString alloc] initWithString:string];
    NSMutableParagraphStyle* style = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
    [style setLineSpacing:4.0];

    [header addAttribute:NSFontAttributeName value:[NSFont boldSystemFontOfSize:0] range:range];
    [header addAttribute:NSForegroundColorAttributeName value:[NSColor grayColor] range:range];
    [header addAttribute:NSParagraphStyleAttributeName value:style range:range];

    [style release];

    return header;
}

- (void)setDefinitiveAnnotation:(NSString*)string {
    if(!string) return;

    NSArray* array = [string componentsSeparatedByString:@"\n"];

    if(array) {
        [textView_ insertText:header1_];
        [textView_ insertText:@"\n"];

        for(int i = 0; i < [array count]; ++ i) {
            NSString* line = [array objectAtIndex:i];

            if([line length] == 0) continue;

            if([line length] == 1 && i + 1 < [array count]) {
                NSString* item = [NSString stringWithFormat:@"%@\t%@",
                                           [array objectAtIndex:i],
                                           [array objectAtIndex:i + 1]];

                [self insertString:[self normalizeString:item] withStyle:listStyle_];
                ++ i;
            } else {
                [self insertString:[self normalizeString:line] withStyle:defaultStyle_];
            }

            [textView_ insertText:@"\n"];
        }
    }
}

- (void)setOptionalAnnotation:(NSString*)string {
    if(!string || [string length] == 0) return;

    [textView_ insertText:header2_];
    [textView_ insertText:@"\n"];
    [self insertString:string withStyle:defaultStyle_];
}

- (NSString*)normalizeString:(NSString*)string {
    const static struct {
        unsigned short unichar;
        NSString* string;
    } table[] = {
        { 0xe021, @"[ー]" },
        { 0xe022, @"[二]" },
        { 0xe023, @"[三]" },
        { 0xe024, @"[四]" },
        { 0xe025, @"[五]" },
        { 0xe026, @"[六]" },
        { 0xe027, @"[文]" },
        { 0,      0x00 }
    };

    NSString* tmp = string;
    for(int i = 0; table[i].string != 0x00; ++ i) {
        NSString* from = [NSString stringWithFormat:@"%C", table[i].unichar];
        NSString* to = table[i].string;
        tmp = [tmp stringByReplacingOccurrencesOfString:from withString:to];
    }

    return tmp;
}

- (void)insertString:(NSString*)string withStyle:(NSParagraphStyle*)style {
    NSMutableAttributedString* attr = [[NSMutableAttributedString alloc] initWithString:string];
    NSRange range = NSMakeRange(0, [attr length]);

    [attr addAttribute:NSParagraphStyleAttributeName value:style range:range];
    [textView_ insertText:attr];

    [attr release];
}

@end
