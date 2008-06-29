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

#include "PreferenceController.h"
#include "SKKServerProxy.h"
#include "SKKConstVars.h"
#include <Carbon/Carbon.h>

@interface PreferenceController (Local)
- (NSArray*)collectKeyboardLayout;
- (NSMenuItem*)menuItemWithInputSource:(TISInputSourceRef)inputSource imageSize:(NSSize)size;
- (void)setupKeyboardLayout;
- (void)updatePopUpButton;
- (void)updateFontButton;
@end

@implementation PreferenceController

- (id)init {
    if(self = [super init]) {
        layoutNames_ = [[NSMutableArray alloc] init];
        preferences_ = [[NSMutableDictionary
                            dictionaryWithContentsOfFile:SKKFilePaths::UserDefaults] retain];
        dictionarySet_ = [[NSMutableArray
                              arrayWithContentsOfFile:SKKFilePaths::DictionarySet] retain];

        NSString* fontName = [preferences_ objectForKey:SKKUserDefaultKeys::candidate_window_font_name];
        NSNumber* fontSize =  [preferences_ objectForKey:SKKUserDefaultKeys::candidate_window_font_size];
        candidateWindowFont_ = [[NSFont fontWithName:fontName size:[fontSize floatValue]] retain];
    }

    return self;
}

- (void)dealloc {
    [candidateWindowFont_ release];
    [dictionarySet_ release];
    [preferences_ release];
    [layoutNames_ release];

    [super dealloc];
}

- (void)awakeFromNib {
    [NSApp activateIgnoringOtherApps:YES];

    [objController_ setContent:preferences_];
    [arrayController_ setContent:dictionarySet_];

    [self setupKeyboardLayout];
    [self updatePopUpButton];
    [self updateFontButton];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender {
    return YES;
}

- (void)applicationWillTerminate:(NSNotification*)aNotification {
    [preferences_ writeToFile:SKKFilePaths::UserDefaults atomically:YES];
    [dictionarySet_ writeToFile:SKKFilePaths::DictionarySet atomically:YES];

    SKKServerProxy* proxy = [[SKKServerProxy alloc] init];

    [proxy reloadUserDefaults];
    [proxy reloadDictionarySet];

    [proxy release];
}

- (void)changeFont:(id)sender {
    [candidateWindowFont_ release];
    candidateWindowFont_ = [[sender convertFont:[NSFont systemFontOfSize:14]] retain];

    // Cocoa Bindings により、ボタンのフォント属性も連動して変更される
    [preferences_ setObject:[candidateWindowFont_ fontName]
                  forKey:SKKUserDefaultKeys::candidate_window_font_name];
    [preferences_ setObject:[NSNumber numberWithFloat:[candidateWindowFont_ pointSize]]
                  forKey:SKKUserDefaultKeys::candidate_window_font_size];

    [self updateFontButton];
}

- (IBAction)showFontPanel:(id)sender {
    NSFontPanel* panel = [NSFontPanel sharedFontPanel];

    [panel setPanelFont:[fontButton_ font] isMultiple:NO];
    [panel makeKeyAndOrderFront:self];
}

- (void)keyboardLayoutDidChange:(id)sender {
    int index = [layoutPopUp_ indexOfSelectedItem];
    NSString* selectedLayout = [layoutNames_ objectAtIndex:index];

    if(selectedLayout) {
        [preferences_ setObject:selectedLayout forKey:SKKUserDefaultKeys::keyboard_layout];
    }
}

@end

@implementation PreferenceController (Local)

static int compareInputSource(id obj1, id obj2, void *context) {
    NSString* lhs = (NSString*)TISGetInputSourceProperty((TISInputSourceRef)obj1, kTISPropertyLocalizedName);
    NSString* rhs = (NSString*)TISGetInputSourceProperty((TISInputSourceRef)obj2, kTISPropertyLocalizedName);

    return [lhs compare:rhs];
}

- (NSArray*)collectKeyboardLayout {
    NSArray* result = 0;

    // 検索条件(ASCII 入力可能なキーボードレイアウト)
    CFMutableDictionaryRef conditions = CFDictionaryCreateMutable(0, 2, 0, 0);
    CFDictionaryAddValue(conditions, kTISPropertyInputSourceType, kTISTypeKeyboardLayout);
    CFDictionaryAddValue(conditions, kTISPropertyInputSourceIsASCIICapable, kCFBooleanTrue);

    // リストして名前でソートする
    if(NSArray* array = (NSArray*)TISCreateInputSourceList(conditions, true)) {
        result = [array sortedArrayUsingFunction:compareInputSource context:0];
        [array release];
    }

    CFRelease(conditions);

    return result;
}

- (NSMenuItem*)menuItemWithInputSource:(TISInputSourceRef)inputSource imageSize:(NSSize)size {
    NSString* title = (NSString*)TISGetInputSourceProperty(inputSource, kTISPropertyLocalizedName);
    IconRef iconref = (IconRef)TISGetInputSourceProperty(inputSource, kTISPropertyIconRef);
    NSImage* image = [[NSImage alloc] initWithIconRef:iconref];
    [image setSize:size];

    NSMenuItem* item = [[NSMenuItem alloc] initWithTitle:title
                                           action:@selector(keyboardLayoutDidChange:) keyEquivalent:@""];
    [item setImage:image];
    [image release];

    return [item autorelease];
}

- (void)setupKeyboardLayout {
    NSArray* array = [self collectKeyboardLayout];
    if(!array) return;

    // PopUpButton のフォントからアイコンのサイズを決めておく
    NSSize size;
    NSFont* font = [layoutPopUp_ font];
    size.height = size.width = [font ascender] - [font descender];

    NSMenu* menu = [[NSMenu alloc] initWithTitle:@""];

    NSEnumerator* enumerator = [array objectEnumerator];
    while(TISInputSourceRef inputSource = (TISInputSourceRef)[enumerator nextObject]) {
        [menu addItem:[self menuItemWithInputSource:inputSource imageSize:size]];

        // "com.apple.keylayout.US" 等の ID 文字列を配列に追加しておく
        [layoutNames_ addObject:(NSString*)TISGetInputSourceProperty(inputSource, kTISPropertyInputSourceID)];
    }

    // PopUpButton にメニューを貼り付ける
    [layoutPopUp_ setMenu:menu];
    [menu release];
}

- (void)updatePopUpButton {
    NSString* selectedLayout = [preferences_ objectForKey:SKKUserDefaultKeys::keyboard_layout];
    int index = [layoutNames_ indexOfObject:selectedLayout];

    if(index == NSNotFound) {
        index = 0;
    }

    [layoutPopUp_ selectItemAtIndex:index];
}

- (void)updateFontButton {
    [fontButton_ setTitle:[NSString stringWithFormat:@"%@ - %2.1f",
                                    [candidateWindowFont_ displayName],
                                    [candidateWindowFont_ pointSize]]];
}

@end
