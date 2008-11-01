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

void MacAnnotator::UpdateAnnotation(const SKKCandidate& candidate, const std::string& buffer) {
    candidate_ = candidate;
    buffer_ = buffer;

    NSString* str = [NSString stringWithUTF8String:candidate_.Variant().c_str()];
    NSString* annotation = [NSString stringWithUTF8String:candidate.Annotation().c_str()];
    CFRange range = CFRangeMake(0, [str length]);

    NSString* definition = (NSString*)DCSCopyTextDefinition(0, (CFStringRef)str, range);

    [window_ setAnnotation:definition optional:annotation];

    [definition release];
}

void MacAnnotator::Show() {
    NSRect rect;
    NSDictionary* dict = [client_ attributesForCharacterIndex:0 lineHeightRectangle:&rect];

    NSFont* font = [dict objectForKey:@"NSFont"];
    if(!font) {
        NSLog(@"NSFont can't get!!!");
        return;
    }

    NSDictionary* attributes = [NSDictionary dictionaryWithObject:font forKey:NSFontAttributeName];
    NSAttributedString* str = [[NSAttributedString alloc]
                                  initWithString:[NSString stringWithUTF8String:buffer_.c_str()]
                                  attributes:attributes];
    rect.origin.x += [str size].width + 4;
    rect.origin.y += 4;
    [str release];

    [window_ show:rect.origin level:[client_ windowLevel]];
}

void MacAnnotator::Hide() {
    [window_ hide];
}
