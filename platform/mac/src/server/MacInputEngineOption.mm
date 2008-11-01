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

#include "MacInputEngineOption.h"
#include "SKKConstVars.h"

MacInputEngineOption::MacInputEngineOption() {
    defaults_ = [NSUserDefaults standardUserDefaults];
}

bool MacInputEngineOption::FixIntermediateConversion() {
    return [defaults_ boolForKey:SKKUserDefaultKeys::fix_intermediate_conversion] == YES;
}

bool MacInputEngineOption::EnableDynamicCompletion() {
    return [defaults_ boolForKey:SKKUserDefaultKeys::enable_dynamic_completion] == YES;
}

int MacInputEngineOption::DynamicCompletionRange() {
    return [defaults_ integerForKey:SKKUserDefaultKeys::dynamic_completion_range];
}

bool MacInputEngineOption::EnableAnnotation() {
    return [defaults_ boolForKey:SKKUserDefaultKeys::enable_annotation] == YES;
}
