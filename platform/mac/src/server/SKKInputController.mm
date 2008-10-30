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
#include "SKKInputSession.h"
#include "SKKPreProcessor.h"
#include "SKKConstVars.h"
#include "MacInputSessionParameter.h"
#include "MacInputModeMenu.h"
#include "MacInputModeWindow.h"
#include "CompletionWindow.h"
#include "AnnotationWindow.h"
#include "SKKFrontEnd.h"
#include "SKKBackEnd.h"

@interface SKKInputController (Local)

- (void)initializeKeyboardLayout;
- (const char*)selectedString;
- (BOOL)privateMode;
- (void)setPrivateMode:(BOOL)flag;

@end

@implementation SKKInputController

- (id)initWithServer:(id)server delegate:(id)delegate client:(id)client {
    self = [super initWithServer:server delegate:delegate client:client];
    if(self) {
        client_ = client;
        defaults_ = [NSUserDefaults standardUserDefaults];
        proxy_ = [[SKKServerProxy alloc] init];
        menu_ = [[SKKInputMenu alloc] initWithClient:client];
        param_ = new MacInputSessionParameter(client_);
        frontend_ = param_->FrontEnd();

        SKKInputModeSelector* master = new SKKInputModeSelector();

        modeMenu_ = new MacInputModeMenu(menu_);
        modeWindow_ = new MacInputModeWindow(frontend_);

        master->AddListener(modeMenu_);
        master->AddListener(modeWindow_);

        session_ = new SKKInputSession(param_, master);
    }

    return self;
}

- (void)dealloc {
    delete session_;
    delete param_;
    delete modeWindow_;
    delete modeMenu_;

    [menu_ release];
    [proxy_ release];
    [super dealloc];
}

// IMKServerInput
- (BOOL)handleEvent:(NSEvent*)event client:(id)sender {
    SKKEvent param = SKKPreProcessor::theInstance().Execute(event);

    param.selected_text = [self selectedString];

    return session_->HandleEvent(param);
}

- (void)commitComposition:(id)sender {
    session_->Clear();
    frontend_->Clear();

    [[AnnotationWindow sharedWindow] hide];
}

// IMKStateSetting
- (void)activateServer:(id)sender {
    activated_ = YES;

    [self initializeKeyboardLayout];

    session_->Activate();
}

- (void)deactivateServer:(id)sender {
    session_->Deactivate();

    [[CompletionWindow sharedWindow] hide];
    [[AnnotationWindow sharedWindow] hide];
}

- (void)setValue:(id)value forTag:(long)tag client:(id)sender {
    // 入力モードを統一する場合は、Leopard デフォルトの挙動に従い、入力
    // メニューのモードで内部の入力モードを更新する
    if([defaults_ boolForKey:SKKUserDefaultKeys::use_unified_input_mode]) {
        SKKEvent param;

        // ex) "com.apple.inputmethod.Roman" => SKK_ASCII_MODE
        param.id = [menu_ eventId:value];
        session_->HandleEvent(param);

        return;
    }

    // 個別の入力モードを保持するなら、アクティブ化直後だけ、入力メニュー
    // を内部の入力モードで更新する
    if(activated_) {
        [menu_ updateMenu:[menu_ currentInputMode]];
        activated_ = NO;
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

- (const char*)selectedString {
    NSRange range = [client_ selectedRange];
    NSAttributedString* text = [client_ attributedSubstringFromRange:range];

    if(text) {
        return [[text string] UTF8String];
    }

    return "";
}

- (BOOL)privateMode {
    return [defaults_ boolForKey:SKKUserDefaultKeys::enable_private_mode];
}

- (void)setPrivateMode:(BOOL)flag {
    [defaults_ setBool:flag forKey:SKKUserDefaultKeys::enable_private_mode];
}

@end
