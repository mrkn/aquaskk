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

#ifndef SKKInputSession_h
#define SKKInputSession_h

#include "SKKRegistrationObserver.h"
#include "SKKInputModeSelector.h"
#include <vector>

class SKKInputSessionParameter;
class SKKRecursiveEditor;
class SKKEvent;

class SKKInputSession : public SKKRegistrationObserver {
    std::auto_ptr<SKKInputSessionParameter> param_;
    std::vector<SKKRecursiveEditor*> stack_;
    std::vector<SKKRecursiveEditor*> temp_;
    SKKInputModeSelector selector_;
    bool inEvent_;

    SKKRecursiveEditor* top();
    SKKRecursiveEditor* createEditor(SKKBaseEditor* bottom);
    void popEditor();
    void beginEvent();
    void endEvent();

    virtual void SKKRegistrationBegin(SKKBaseEditor* bottom);
    virtual void SKKRegistrationFinish(const std::string& word);
    virtual void SKKRegistrationCancel();

    SKKInputSession();
    SKKInputSession(const SKKInputSession&);
    SKKInputSession& operator=(const SKKInputSession&);

public:
    SKKInputSession(SKKInputSessionParameter* param);
    ~SKKInputSession();

    void AddInputModeListener(SKKInputModeListener* listener);

    bool HandleEvent(const SKKEvent& event);
    void Clear();

    void Activate();
    void Deactivate();
};

#endif
