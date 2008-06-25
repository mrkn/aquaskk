/* -*- C++ -*-

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

#ifndef SKKInputEngine_h
#define SKKInputEngine_h

#include "SKKEditorStack.h"
#include "SKKStateMachine.h"

class SKKInputSessionParameter;

class SKKInputEngine {
    SKKInputSessionParameter* param_;
    std::auto_ptr<SKKBaseEditor> bottom_;
    SKKEditorStack editor_;
    SKKStateMachine state_;

    SKKInputEngine();
    SKKInputEngine(const SKKInputEngine&);
    SKKInputEngine& operator=(const SKKInputEngine&);

public:
    SKKInputEngine(SKKRegistrationObserver* registrationObserver,
                   SKKInputSessionParameter* param,
                   SKKBaseEditor* bottom);

    void Dispatch(const SKKEvent& event);

    void Commit(const std::string& word);
    void Cancel();

    bool Emit();

    const SKKEntry Entry() const;
    const std::string Word() const;
};

#endif
