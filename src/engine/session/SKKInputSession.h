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

#include "SKKEditorStack.h"

class SKKInputSessionParameter;
class SKKInputEngine;
class SKKEvent;

class SKKInputSession : public SKKRegistrationObserver {
    std::vector<SKKInputEngine*> stack_;
    SKKInputSessionParameter* param_;
    SKKRegistrationObserver::Event event_;
    bool preventReentrantCall_;

    SKKInputSession();
    SKKInputSession(const SKKInputSession&);
    SKKInputSession& operator=(const SKKInputSession&);

    SKKInputEngine* top();
    void pushEngine();
    void popEngine();
    void finish(bool commit);

    virtual void SKKRegistrationUpdate(SKKRegistrationObserver::Event event);

public:
    SKKInputSession(SKKInputSessionParameter* param);
    ~SKKInputSession();

    bool HandleEvent(const SKKEvent& event);
    void Activate();
    void Deactivate();
};

#endif
