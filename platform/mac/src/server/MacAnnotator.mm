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

#include "MacAnnotator.h"
#include "AnnotationWindow.h"
#include "utf8util.h"

#include <InputMethodKit/InputMethodKit.h>
#include <CoreServices/CoreServices.h>
#include <iostream>

MacAnnotator::MacAnnotator(id client) : client_(client) {
    window_ = [AnnotationWindow sharedWindow];
}

void MacAnnotator::Update(const SKKCandidate& candidate) {
    candidate_ = candidate;
    NSString* str = [NSString stringWithUTF8String:candidate_.Variant().c_str()];
    //NSString* anon = [NSString stringWithUTF8String:candidate.Annotation().c_str()];
    CFRange range = CFRangeMake(0, [str length]);

    NSString* result = (NSString*)DCSCopyTextDefinition(0, (CFStringRef)str, range);

    if(result) {
        [window_ setAnnotation:result];
        [result release];
    } else {
        [window_ setAnnotation:@""];
    }
}

void MacAnnotator::Show(int cursor) {
    NSRect rect;
    NSDictionary* dict = [client_ attributesForCharacterIndex:cursor - 1 lineHeightRectangle:&rect];

    NSLog(@"anon dict=%@", dict);

    NSFont* font = [dict objectForKey:@"NSFont"];
    NSDictionary* attributes = [NSDictionary dictionaryWithObject:font forKey:NSFontAttributeName];

    std::string last = utf8::right(candidate_.Variant(), -1);

    NSAttributedString* str = [[NSAttributedString alloc]
                                  initWithString:[NSString stringWithUTF8String:last.c_str()]
                                  attributes:attributes];
    rect.origin.x += [str size].width + 4;
    rect.origin.y += 4;
    [str release];

    [window_ show:rect.origin level:[client_ windowLevel]];
}

void MacAnnotator::Hide() {
    [window_ hide];
}
