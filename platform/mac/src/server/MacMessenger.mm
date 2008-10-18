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

#include "MacMessenger.h"
#include "MessengerWindow.h"
#include "SKKFrontEnd.h"
#include "ObjCUtil.h"

MacMessenger::MacMessenger(SKKFrontEnd* frontend) : frontend_(frontend) {}

void MacMessenger::SendMessage(const std::string& msg) {
    ObjC::RAIIPool pool;

    MessengerWindow* window = [MessengerWindow sharedWindow];

    NSString* str = [NSString stringWithUTF8String:msg.c_str()];

    std::pair<int, int> pos = frontend_->WindowPosition();
    int windowLevel = frontend_->WindowLevel();

    [window showMessage:str at:NSMakePoint(pos.first, pos.second) level:windowLevel];
}