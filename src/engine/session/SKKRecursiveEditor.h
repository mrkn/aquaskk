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

#ifndef SKKRecursiveEditor_h
#define SKKRecursiveEditor_h

#include "SKKStateMachine.h"
#include "SKKInputEngine.h"

class SKKInputSessionParameter;

class SKKRecursiveEditor {
    std::auto_ptr<SKKBaseEditor> bottom_;
    SKKInputSessionParameter* param_;
    SKKStateMachine state_;
    SKKInputEngine editor_;

    SKKRecursiveEditor();
    SKKRecursiveEditor(const SKKRecursiveEditor&);
    SKKRecursiveEditor& operator=(const SKKRecursiveEditor&);

public:
    SKKRecursiveEditor(SKKRegistrationObserver* registrationObserver,
                       SKKInputSessionParameter* param,
                       SKKBaseEditor* bottom);

    void Dispatch(const SKKEvent& event);
    bool Output();
    void Commit(const std::string& word);

    const SKKEntry Entry() const;
    const std::string Word() const;
};

#endif
