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

#include "MacInputModeWindow.h"
#include "SKKFrontEnd.h"
#include "SKKConstVars.h"
#include "InputModeWindow.h"
#include <iostream>
#include <vector>

// MacInputModeWindow::Activate() から呼ばれるユーティリティ群
namespace {
    // 左下原点を左上原点に変換する
    CGPoint FlipPoint(int x, int y) {
        NSRect screen = [[NSScreen mainScreen] frame];

        return CGPointMake(x, NSHeight(screen) - y);
    }

    int ActiveProcessID() {
        NSDictionary* info = [[NSWorkspace sharedWorkspace] activeApplication];
        NSNumber* pid = [info objectForKey:@"NSApplicationProcessIdentifier"];

        return [pid intValue];
    }

    typedef std::vector<CGRect> CGRectContainer;

    // プロセス ID に関連したウィンドウ矩形群の取得
    CGRectContainer CreateWindowBoundsListOf(int pid) {
        CGRectContainer result;
        NSArray* array = (NSArray*)CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly,
                                                              kCGNullWindowID);
        NSEnumerator* enumerator = [array objectEnumerator];

        while(NSDictionary* window = [enumerator nextObject]) {
            // 引数のプロセス ID でフィルタ
            NSNumber* owner = [window objectForKey:(NSString*)kCGWindowOwnerPID];
            if([owner intValue] != pid) continue;

            // デスクトップ全面を覆う Finder のウィンドウは除外
            NSNumber* level = [window objectForKey:(NSString*)kCGWindowLayer];
            if([level intValue] == kCGMinimumWindowLevel) continue;

            CGRect rect;
            NSDictionary* bounds = [window objectForKey:(NSString*)kCGWindowBounds];
            if(CGRectMakeWithDictionaryRepresentation((CFDictionaryRef)bounds, &rect)) {
                result.push_back(rect);
            }
        }

        [array release];

        return result;
    }
}

// ============================================================

MacInputModeWindow::MacInputModeWindow(SKKFrontEnd* frontend)
  : active_(false)
  , frontend_(frontend)
  , mode_(HirakanaInputMode) {
    window_ = [InputModeWindow sharedWindow];
    [window_ changeMode:mode_];
}

void MacInputModeWindow::SelectInputMode(SKKInputMode mode) {
    if(mode_ != mode) {
        mode_ = mode;

        if(active_) Activate();
    }
}

void MacInputModeWindow::Activate() {
    active_ = true;

    if(!enabled()) return;

    std::pair<int, int> position = frontend_->WindowPosition();

    CGPoint cursor = FlipPoint(position.first, position.second);
    CGRectContainer list = CreateWindowBoundsListOf(ActiveProcessID());

    // カーソル位置がウィンドウ矩形に含まれていなければ無視する
    int count = std::count_if(list.begin(), list.end(),
                              std::bind2nd(std::ptr_fun(CGRectContainsPoint), cursor));
    if(!count) return;

    [window_ changeMode:mode_];
    [window_ show:NSMakePoint(position.first, position.second) level:frontend_->WindowLevel()];
}

void MacInputModeWindow::Deactivate() {
    [window_ hide];

    active_ = false;
}

// ------------------------------------------------------------

bool MacInputModeWindow::enabled() const {
    NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];

    [defaults synchronize];
    
    return [defaults boolForKey:SKKUserDefaultKeys::show_input_mode_icon] == YES;
}
