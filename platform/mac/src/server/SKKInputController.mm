/* -*- ObjC -*-

  MacOS X implementation of the SKK input method.

  Copyright (C) 2007-2008 Tomotaka SUWA <t.suwa@mac.com>

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

        session_->AddInputModeListener(new MacInputModeMenu(menu_));
        session_->AddInputModeListener(new MacInputModeWindow(layout_));
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
    SKKEvent param = SKKPreProcessor::theInstance().Execute(event);

    return session_->HandleEvent(param);
}

- (void)commitComposition:(id)sender {
    [self debug:@"commitComposition"];

    session_->Clear();
}

// IMKStateSetting
- (void)activateServer:(id)sender {
    [self debug:@"activateServer"];

    [self initializeKeyboardLayout];

    activated_ = YES;

    session_->Activate();
}

- (void)deactivateServer:(id)sender {
    [self debug:@"deactivateServer"];

    session_->Deactivate();
}

- (void)setValue:(id)value forTag:(long)tag client:(id)sender {
    if(tag != kTextServiceInputModePropertyTag) return;

    [self debug:@"setValue"];

    bool individual = ([defaults_ boolForKey:SKKUserDefaultKeys::use_individual_input_mode] == YES);

    // 個別の入力モードが有効な状態では強制的に入力モードを統一する
    // 「AquaSKK 統合」以外では、文書毎に独立した入力モードを保持できない
    if([menu_ eventId:value] != 0) {
        individual = false;
    }

    if(activated_) {
        if(individual) {
            NSString* identifier = [menu_ modeIdentifier:[menu_ currentInputMode]];
            SKKEvent param;

            param.id = [menu_ eventId:identifier];
            session_->HandleEvent(param);

            [menu_ updateMenu:[menu_ currentInputMode]];
        } else {
            NSString* identifier = [menu_ modeIdentifier:[menu_ unifiedInputMode]];
            SKKEvent param;

            param.id = [menu_ eventId:identifier];
            session_->HandleEvent(param);
        }

        activated_ = NO;
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
    enum MenuTypes { Normal, Selector, PrivateMode };
    struct {
        MenuTypes type;
        const char* title;
        SEL selector;
    } items[] = {
        { Normal,	"環境設定",			@selector(showPreferences:) },
        { PrivateMode,	"プライベートモード",		@selector(togglePrivateMode:) },
        { Normal,	"設定ファイルの再読み込み",	@selector(reloadComponents:) },
#ifdef SKK_DEBUG
        { Normal,	"デバッグ情報",			@selector(showDebugInfo:) },
#endif
        { Normal,	"AquaSKK ヘルプ",		@selector(showHelp:) },
        { Selector,	"__selector__",			0 },
        { Normal,	"Web::プロジェクトホーム",	@selector(webHome:) },
        { Normal,	"Web::便利な機能、Tips",	@selector(webTips:) },
        { Normal,	"Web::FAQ",			@selector(webFAQ:) },
        { Normal,	0,				0 }
    };

    NSMenu* inputMenu = [[[NSMenu alloc] initWithTitle:@"AquaSKK"] autorelease];

    for(int i = 0; items[i].title != 0; ++ i) {
        NSMenuItem* item;
        const char* title = items[i].title;
        SEL selector = items[i].selector;

        switch(items[i].type) {
        case Normal:
        case PrivateMode:
            item = [[NSMenuItem alloc] initWithTitle:[NSString stringWithUTF8String:title]
                                       action:selector keyEquivalent:@""];
            [item autorelease];

            if(items[i].type == PrivateMode) {
                [item setState:[self privateMode] ? NSOnState : NSOffState];
            }
            break;

        case Selector:
            item = [NSMenuItem separatorItem];
            break;
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

- (void)showHelp:(id)sender {
    NSHelpManager* manager = [NSHelpManager sharedHelpManager];

    [manager openHelpAnchor:@"mail" inBook:nil];
}

- (void)openURL:(NSString*)url {
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:url]];
}

- (void)webHome:(id)sender {
    [self openURL:@"http://aquaskk.sourceforge.jp/"];
}

- (void)webTips:(id)sender {
    [self openURL:@"http://aquaskk.sourceforge.jp/inside_aquaskk/"];
}

- (void)webFAQ:(id)sender {
    [self openURL:@"http://sourceforge.jp/projects/aquaskk/wiki/FAQ"];
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

- (void)debug:(NSString*)str {
#ifdef SKK_DEBUG
    NSLog(@"%@: %@", [client_ bundleIdentifier], str);
#endif
}

@end
