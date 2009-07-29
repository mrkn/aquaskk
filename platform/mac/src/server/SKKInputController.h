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

#ifndef SKKInputController_h
#define SKKInputController_h

#import <InputMethodKit/InputMethodKit.h>
#include "SKKServerProxy.h"
#include "SKKInputMenu.h"

class SKKInputSessionParameter;
class SKKInputSession;
class SKKLayoutManager;
class SKKFrontEnd;
class SKKInputModeListener;

@interface SKKInputController : IMKInputController {
    id client_;
    BOOL activated_;
    NSUserDefaults* defaults_;
    SKKServerProxy* proxy_;
    SKKInputMenu* menu_;
    SKKInputMode unifiedInputMode_;

    SKKInputSessionParameter* param_;
    SKKInputSession* session_;
    SKKLayoutManager* layout_;
    SKKFrontEnd* frontend_;
    SKKInputModeListener* modeMenu_;
    SKKInputModeListener* modeWindow_;
}
@end

#endif
