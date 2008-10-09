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
#include "InputModeWindowController.h"
#include "InputModeCursor.h"
#include "SKKFrontEnd.h"
#include "SKKBackEnd.h"

@interface SKKInputController (Local)

- (void)initializeKeyboardLayout;
- (void)setInputModeIfNeeded;
- (const char*)selectedString;
- (void)updateModeCursor:(id)sender;
- (BOOL)privateMode;
- (void)setPrivateMode:(BOOL)flag;
- (SKKInputMode)currentInputMode;

@end

@implementation SKKInputController

- (id)initWithServer:(id)server delegate:(id)delegate client:(id)client {
    self = [super initWithServer:server delegate:delegate client:client];
    if(self) {
        // 直前のセッションの入力モードを保存しておく
        initialInputMode_ = [self currentInputMode];
        initialized_ = NO;

        client_ = client;
        defaults_ = [NSUserDefaults standardUserDefaults];
        proxy_ = [[SKKServerProxy alloc] init];
        param_ = new MacInputSessionParameter(client_);
	session_ = new SKKInputSession(param_);
        frontend_ = param_->FrontEnd();
    }

    return self;
}

- (void)dealloc {
    delete session_;
    delete param_;

    [proxy_ release];
    [super dealloc];
}

// IMKServerInput
- (BOOL)handleEvent:(NSEvent*)event client:(id)sender {
    SKKEvent param = SKKPreProcessor::theInstance().Execute(event);

    param.selected_text = [self selectedString];

    BOOL result = session_->HandleEvent(param);

    [self updateModeCursor:nil];

    return result || param.force_handled;
}

- (void)commitComposition:(id)sender {
#ifdef SKK_DEBUG
    NSLog(@"commitComposition from %@", sender);
#endif

    session_->Clear();
    frontend_->Clear();
}

// IMKStateSetting
- (void)activateServer:(id)sender {
    [self initializeKeyboardLayout];
    [self setInputModeIfNeeded];

    session_->Activate();
}

- (void)deactivateServer:(id)sender {
    session_->Deactivate();

    [[InputModeCursor sharedCursor] hide];
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

- (void)setInputModeIfNeeded {
    if([defaults_ boolForKey:SKKUserDefaultKeys::use_unified_input_mode]) {
        SKKEvent event;
        SKKInputMode currentInputMode;

        // 起動時は直前のセッションの入力モードを復元する
        if(!initialized_) {
            currentInputMode = initialInputMode_;
            initialized_ = YES;
        } else {
            currentInputMode = [self currentInputMode];
        }

        switch(currentInputMode) {
        case AsciiInputMode:
            event.id = SKK_ASCII_MODE;
            break;

        case HirakanaInputMode:
            event.id = SKK_HIRAKANA_MODE;
            break;

        case KatakanaInputMode:
            event.id = SKK_KATAKANA_MODE;
            break;

        case Jisx0201KanaInputMode:
            event.id = SKK_JISX0201KANA_MODE;
            break;

        case Jisx0208LatinInputMode:
            event.id = SKK_JISX0208LATIN_MODE;
            break;
        }

        session_->HandleEvent(event);
    }
}

- (const char*)selectedString {
    NSRange range = [client_ selectedRange];
    NSAttributedString* text = [client_ attributedSubstringFromRange:range];

    if(text) {
        return [[text string] UTF8String];
    }

    return "";
}

- (void)updateModeCursor:(id)sender {
    if([defaults_ boolForKey:SKKUserDefaultKeys::show_input_mode_cursor] != YES) return;

    if(!sender) {
        [self performSelector:@selector(updateModeCursor:) withObject:self afterDelay:0.005];
        return;
    }

    std::pair<int, int> pos = frontend_->WindowPosition();
    int level = frontend_->WindowLevel();
    InputModeCursor* cursor = [InputModeCursor sharedCursor];

    [cursor changeMode:[self currentInputMode]];
    [cursor show:NSMakePoint(pos.first, pos.second) level:level];
}

- (BOOL)privateMode {
    return [defaults_ boolForKey:SKKUserDefaultKeys::enable_private_mode];
}

- (void)setPrivateMode:(BOOL)flag {
    [defaults_ setBool:flag forKey:SKKUserDefaultKeys::enable_private_mode];
}

- (SKKInputMode)currentInputMode {
    return [[InputModeWindowController sharedController] currentInputMode];
}

@end
