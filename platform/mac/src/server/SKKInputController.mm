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

@interface SKKInputController (Local)

- (void)initializeKeyboardLayout;
- (void)setInputModeIfNeeded;
- (void)updateCursorPosition;

@end

@implementation SKKInputController

- (id)initWithServer:(id)server delegate:(id)delegate client:(id)client {
    self = [super initWithServer:server delegate:delegate client:client];
    if(self) {
        // 直前のセッションの入力モードを保存しておく
        initialInputMode_ = [[InputModeWindowController sharedController] currentInputMode];
        initialized_ = NO;

        client_ = client;
        proxy_ = [[SKKServerProxy alloc] init];
        param_ = new MacInputSessionParameter(client_);
	session_ = new SKKInputSession(param_);
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
    NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];

    SKKEvent param = SKKPreProcessor::theInstance().Execute(event);

    BOOL result = session_->HandleEvent(param);

    if([defaults boolForKey:SKKUserDefaultKeys::show_input_mode_cursor] == YES) {
        [self performSelector:@selector(updateCursorPosition) object:nil afterDelay:0.005];
    }

    return result || param.force_handled;
}

- (void)commitComposition:(id)sender {
    SKKEvent event;

    event.id = SKK_JMODE;

    session_->HandleEvent(event);
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
    NSMenu* inputMenu = [[[NSMenu alloc] initWithTitle:@"AquaSKK"] autorelease];

    [inputMenu addItemWithTitle:[NSString stringWithUTF8String:"環境設定"]
               action:@selector(showPreferences:) keyEquivalent:@""];

    [inputMenu addItemWithTitle:[NSString stringWithUTF8String:"設定ファイルの再読み込み"]
               action:@selector(reloadComponents:) keyEquivalent:@""];

    [inputMenu addItemWithTitle:[NSString stringWithUTF8String:"AquaSKK ヘルプ"]
               action:@selector(showHelp:) keyEquivalent:@""];

    [inputMenu addItem:[NSMenuItem separatorItem]];

    [inputMenu addItemWithTitle:[NSString stringWithUTF8String:"Web::プロジェクトホーム"]
               action:@selector(webHome:) keyEquivalent:@""];

    [inputMenu addItemWithTitle:[NSString stringWithUTF8String:"Web::便利な機能、Tips"]
               action:@selector(webTips:) keyEquivalent:@""];

    [inputMenu addItemWithTitle:[NSString stringWithUTF8String:"Web::FAQ"]
               action:@selector(webFAQ:) keyEquivalent:@""];

    return inputMenu;
}

// handling menu items
- (void)showPreferences:(id)sender {
    NSString* path = [NSString stringWithFormat:@"%@/AquaSKKPreferences.app",
                               [[NSBundle mainBundle] sharedSupportPath]];

    [[NSWorkspace sharedWorkspace] launchApplication:path];
}

- (void)reloadComponents:(id)sender {
    [proxy_ reloadComponents];
}

- (void)showHelp:(id)sender {
    NSLog(@"showHelp");
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
    NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
    
    NSString* keyboardLayout = [defaults stringForKey:SKKUserDefaultKeys::keyboard_layout];
    [client_ overrideKeyboardWithKeyboardNamed:keyboardLayout];
}

- (void)setInputModeIfNeeded {
    NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];

    if([defaults boolForKey:SKKUserDefaultKeys::use_unified_input_mode]) {
        SKKEvent event;
        SKKInputMode currentInputMode;

        // 起動時は直前のセッションの入力モードを復元する
        if(!initialized_) {
            currentInputMode = initialInputMode_;
            initialized_ = YES;
        } else {
            currentInputMode = [[InputModeWindowController sharedController] currentInputMode];
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

- (void)updateCursorPosition {
    std::pair<int, int> pos = param_->FrontEnd()->WindowPosition();
    int level = param_->FrontEnd()->WindowLevel();
    SKKInputMode currentInputMode = [[InputModeWindowController sharedController] currentInputMode];

    [[InputModeCursor sharedCursor] changeMode:currentInputMode];
    [[InputModeCursor sharedCursor] show:NSMakePoint(pos.first, pos.second) level:level];
}

@end
