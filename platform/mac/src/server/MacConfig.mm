/* -*- ObjC -*-

  MacOS X implementation of the SKK input method.

  Copyright (C) 2009 Tomotaka SUWA <t.suwa@mac.com>

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

#include "MacConfig.h"
#include "SKKConstVars.h"

MacConfig::MacConfig() {
    defaults_ = [NSUserDefaults standardUserDefaults];
}

bool MacConfig::FixIntermediateConversion() {
    return [defaults_ boolForKey:SKKUserDefaultKeys::fix_intermediate_conversion] == YES;
}

bool MacConfig::EnableDynamicCompletion() {
    return [defaults_ boolForKey:SKKUserDefaultKeys::enable_dynamic_completion] == YES;
}

int MacConfig::DynamicCompletionRange() {
    return [defaults_ integerForKey:SKKUserDefaultKeys::dynamic_completion_range];
}

bool MacConfig::EnableAnnotation() {
    return [defaults_ boolForKey:SKKUserDefaultKeys::enable_annotation] == YES;
}

bool MacConfig::DisplayShortestMatchOfKanaConversions() {
    return [defaults_ boolForKey:SKKUserDefaultKeys::display_shortest_match_of_kana_conversions] == YES;
}

bool MacConfig::SuppressNewlineOnCommit() {
    return [defaults_ boolForKey:SKKUserDefaultKeys::suppress_newline_on_commit] == YES;
}

int MacConfig::MaxCountOfInlineCandidates() {
    return [defaults_ integerForKey:SKKUserDefaultKeys::max_count_of_inline_candidates];
}

bool MacConfig::HandleRecursiveEntryAsOkuri() {
    return [defaults_ boolForKey:SKKUserDefaultKeys::handle_recursive_entry_as_okuri];
}
