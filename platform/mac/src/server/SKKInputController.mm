/* -*- ObjC -*-

  MacOS X implementation of the SKK input method.

  Copyright (C) 2007-2010 Tomotaka SUWA <tomotaka.suwa@gmail.com>

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

#include "SKKInputController.h"
#include "SKKLayoutManager.h"
#include "SKKInputSession.h"
#include "SKKBackEnd.h"

#include "SKKPreProcessor.h"
#include "SKKConstVars.h"

#include "MacInputSessionParameter.h"
#include "MacInputModeMenu.h"
#include "MacInputModeWindow.h"

@interface SKKInputController (Local)

- (void)initializeKeyboardLayout;
- (BOOL)privateMode;
- (void)setPrivateMode:(BOOL)flag;
- (BOOL)directMode;
- (void)setDirectMode:(BOOL)flag;
- (void)debug:(NSString*)message;

@end

@implementation SKKInputController

- (id)initWithServer:(id)server delegate:(id)delegate client:(id)client {
    self = [super initWithServer:server delegate:delegate client:client];
    if(self) {
        client_ = client;
        activated_ = NO;
        defaults_ = [NSUserDefaults standardUserDefaults];
        proxy_ = [[SKKServerProxy alloc] init];
        menu_ = [[SKKInputMenu alloc] initWithClient:client];

        layout_ = new SKKLayoutManager(client_);
        session_ = new SKKInputSession(new MacInputSessionParameter(client_, layout_));
        modeIcon_ = new MacInputModeWindow(layout_);

        session_->AddInputModeListener(new MacInputModeMenu(menu_));
        session_->AddInputModeListener(modeIcon_);
    }

    return self;
}

- (void)dealloc {
    delete session_;
    delete layout_;

    [menu_ release];
    [proxy_ release];
    [super dealloc];
}

// IMKServerInput
- (BOOL)handleEvent:(NSEvent*)event client:(id)sender {
    if([self directMode]) return NO;

    SKKEvent param = SKKPreProcessor::theInstance().Execute(event);

    return session_->HandleEvent(param);
}

- (void)commitComposition:(id)sender {
    if([self directMode]) return;

    [self debug:@"commitComposition"];

    session_->Commit();
}

// IMKStateSetting
- (void)activateServer:(id)sender {
    if([self directMode]) return;

    [self debug:@"activateServer"];

    [self initializeKeyboardLayout];

    activated_ = YES;

    session_->Activate();
}

- (void)deactivateServer:(id)sender {
    if([self directMode]) return;

    [self debug:@"deactivateServer"];

    session_->Deactivate();
}

- (void)setValue:(id)value forTag:(long)tag client:(id)sender {
    if([self directMode]) return;

    if(tag != kTextServiceInputModePropertyTag) return;

    [self debug:@"setValue"];

    bool individual = ([defaults_ boolForKey:SKKUserDefaultKeys::use_individual_input_mode] == YES);

    // 個別の入力モードが有効な状態では強制的に入力モードを統一する
    // 「AquaSKK 統合」以外では、文書毎に独立した入力モードを保持できない
    if([menu_ eventId:value] != 0) {
        individual = false;
    }

    if(activated_) {
        activated_ = NO;

        if(individual) {
            NSString* identifier = [menu_ modeIdentifier:[menu_ currentInputMode]];
            SKKEvent param;

            param.id = [menu_ eventId:identifier];
            session_->HandleEvent(param);

            modeIcon_->SelectInputMode([menu_ currentInputMode]);
        } else {
            NSString* identifier = [menu_ modeIdentifier:[menu_ unifiedInputMode]];
            SKKEvent param;

            param.id = [menu_ eventId:identifier];
            session_->HandleEvent(param);
        }

        return;
    }

    SKKEvent param;

    // ex) "com.apple.inputmethod.Roman" => SKK_ASCII_MODE
    param.id = [menu_ eventId:(NSString*)value];
    if(param.id != InvalidInputMode) {
        session_->HandleEvent(param);
    }
}

// IMKInputController
- (NSMenu*)menu {
    struct {
        const char* title;
        SEL handler;
        SEL state;
    } items[] = {
        { "環境設定",                 @selector(showPreferences:),   0 },
        { "直接入力モード",           @selector(toggleDirectMode:),  @selector(directMode) },
        { "プライベートモード",       @selector(togglePrivateMode:), @selector(privateMode) },
        { "設定ファイルの再読み込み", @selector(reloadComponents:),  0 },
#ifdef SKK_DEBUG
        { "デバッグ情報",             @selector(showDebugInfo:),     0 },
#endif
        { "separator",                0,                             0 },
        { "Web::日本語を快適に",      @selector(webHome:),           0 },
        { "Web::SourceForge.JP",      @selector(webSourceForge:),    0 },
        { "Web::Wiki",                @selector(webWiki:),           0 },
        { 0,                          0,                             0 }
    };

    NSMenu* inputMenu = [[[NSMenu alloc] initWithTitle:@"AquaSKK"] autorelease];

    for(int i = 0; items[i].title != 0; ++ i) {
        NSString* title = [NSString stringWithUTF8String:items[i].title];
        SEL handler = items[i].handler;
        NSMenuItem* item;

        if(handler != 0) {
            item = [[NSMenuItem alloc] initWithTitle:title
                                              action:handler
                                       keyEquivalent:@""];
            [item autorelease];
        } else {
            item = [NSMenuItem separatorItem];
        }
        
        if(items[i].state != 0) {
            [item setState:(NSInteger)[self performSelector:items[i].state]];
        }

        [inputMenu addItem:item];
    }

    return inputMenu;
}

// handling menu items
- (void)showPreferences:(id)sender {
    NSString* path = [NSString stringWithFormat:@"%@/AquaSKKPreferences.app",
                               [[NSBundle mainBundle] sharedSupportPath]];

    [[NSWorkspace sharedWorkspace] launchApplication:path];
}

- (void)togglePrivateMode:(id)sender {
    [self setPrivateMode:![self privateMode]];

    SKKBackEnd::theInstance().EnablePrivateMode([self privateMode]);
}

- (void)toggleDirectMode:(id)sender {
    [self setDirectMode:![self directMode]];
}

- (void)reloadComponents:(id)sender {
    [proxy_ reloadComponents];
}

- (void)showDebugInfo:(id)sender {
    NSMutableString* info = [[NSMutableString alloc] initWithCapacity:0];
    NSRect rect;

    [info appendFormat:@"bundleId = %@\n", [client_ bundleIdentifier]];
    [info appendFormat:@"attributes = %@\n",
          [client_ attributesForCharacterIndex:0 lineHeightRectangle:&rect]];
    [info appendFormat:@"inline rect = %@\n", NSStringFromRect(rect)];
    [info appendFormat:@"selected range = %@\n",NSStringFromRange([client_ selectedRange])];
    [info appendFormat:@"marked range = %@\n", NSStringFromRange([client_ markedRange])];
    [info appendFormat:@"supports unicode = %@\n",
          ([client_ supportsUnicode] == 1 ? @"YES" : @"NO")];
    [info appendFormat:@"window level = %d\n", [client_ windowLevel]];
    [info appendFormat:@"length = %d\n", [client_ length]];
    [info appendFormat:@"valid attributes = %@\n", [client_ validAttributesForMarkedText]];

    NSAlert* alert = [[NSAlert alloc] init];
    [alert addButtonWithTitle:@"OK"];
    [alert setMessageText:@"デバッグ情報"];
    [alert setInformativeText:info];
    [alert setAlertStyle:NSInformationalAlertStyle];
    [alert setIcon:[NSImage imageNamed:NSImageNameInfo]];
    [[alert window] setLevel:kCGPopUpMenuWindowLevel];
    [[alert window] setTitle:@"AquaSKK"];

    [alert beginSheetModalForWindow:0 modalDelegate:self didEndSelector:0 contextInfo:0];

    NSPasteboard* pb = [NSPasteboard generalPasteboard];

    [pb declareTypes:[NSArray arrayWithObjects:NSStringPboardType, nil] owner:self];
    [pb setString:info forType:NSStringPboardType];

    [info release];
}

- (void)openURL:(NSString*)url {
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:url]];
}

- (void)webHome:(id)sender {
    [self openURL:@"http://aquaskk.sourceforge.jp/"];
}

- (void)webSourceForge:(id)sender {
    [self openURL:@"http://sourceforge.jp/projects/aquaskk/"];
}

- (void)webWiki:(id)sender {
    [self openURL:@"http://sourceforge.jp/projects/aquaskk/wiki/FrontPage"];
}

@end

@implementation SKKInputController (Local)

- (void)initializeKeyboardLayout {
    NSString* keyboardLayout = [defaults_ stringForKey:SKKUserDefaultKeys::keyboard_layout];
    [client_ overrideKeyboardWithKeyboardNamed:keyboardLayout];
}

- (BOOL)privateMode {
    return [defaults_ boolForKey:SKKUserDefaultKeys::enable_private_mode];
}

- (void)setPrivateMode:(BOOL)flag {
    [defaults_ setBool:flag forKey:SKKUserDefaultKeys::enable_private_mode];
}

- (BOOL)directMode {
    NSArray* clients = [defaults_ arrayForKey:SKKUserDefaultKeys::direct_clients];

    return [clients containsObject:[client_ bundleIdentifier]] == YES;
}

- (void)setDirectMode:(BOOL)flag {
    NSArray* current = [defaults_ arrayForKey:SKKUserDefaultKeys::direct_clients];
    NSMutableArray* result = [NSMutableArray arrayWithArray:current];
    NSString* client = [client_ bundleIdentifier];

    if(flag) {
        [result addObject:client];
    } else {
        [result removeObject:client];
    }

    [defaults_ setObject:result forKey:SKKUserDefaultKeys::direct_clients];
}

- (void)debug:(NSString*)str {
#ifdef SKK_DEBUG
    NSLog(@"%@: %@", [client_ bundleIdentifier], str);
#endif
}

@end
