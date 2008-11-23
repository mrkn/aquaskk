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
#include "SKKConstVars.h"
#include "utf8util.h"

#include <InputMethodKit/InputMethodKit.h>
#include <CoreServices/CoreServices.h>
#include <iostream>

MacAnnotator::MacAnnotator(id client) : client_(client), definition_(nil), optional_(nil) {
    window_ = [AnnotationWindow sharedWindow];
}

void MacAnnotator::Update(const SKKCandidate& candidate, const std::string& buffer) {
    candidate_ = candidate;
    buffer_ = buffer;

    release(definition_);
    release(optional_);

    NSString* str = [NSString stringWithUTF8String:candidate_.Variant().c_str()];
    CFRange range = CFRangeMake(0, [str length]);
    definition_ = (NSString*)DCSCopyTextDefinition(0, (CFStringRef)str, range);

    if(!candidate_.Annotation().empty()) {
        optional_ = [NSString stringWithUTF8String:candidate_.Annotation().c_str()];
        [optional_ retain];
    }
}

// ------------------------------------------------------------

void MacAnnotator::release(NSString*& str) {
    if(str) {
        [str release];
    }

    str = nil;
}

void MacAnnotator::SKKWidgetShow() {
    NSRect rect;
    NSDictionary* dict = [client_ attributesForCharacterIndex:0 lineHeightRectangle:&rect];
    NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];

    [window_ setAnnotation:definition_ optional:optional_];

    if(!definition_ && !optional_) {
        SKKWidgetHide();
    }

    if([defaults boolForKey:SKKUserDefaultKeys::put_candidate_window_upward] == YES) {
        [window_ show:rect.origin level:[client_ windowLevel] topleft:YES];
    } else {
        NSFont* font = [dict objectForKey:@"NSFont"];
        if(!font) {
            NSLog(@"MacAnnotator::Show(): can't get font");
            return;
        }

        NSDictionary* attributes = [NSDictionary dictionaryWithObject:font forKey:NSFontAttributeName];
        NSAttributedString* str = [[NSAttributedString alloc]
                                      initWithString:[NSString stringWithUTF8String:buffer_.c_str()]
                                      attributes:attributes];
        rect.origin.x += [str size].width + 2;
        rect.origin.y += 2;
        [str release];

        [window_ show:rect.origin level:[client_ windowLevel] topleft:NO];
    }
}

void MacAnnotator::SKKWidgetHide() {
    [window_ hide];
}
