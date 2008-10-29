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

#ifndef SKKComposingEditor_h
#define SKKComposingEditor_h

#include "SKKBaseEditor.h"
#include "SKKTextBuffer.h"

class SKKComposingEditor : public SKKBaseEditor {
    SKKTextBuffer composing_;
    std::string completion_;
    bool enableDynamicCompletion_;
    bool modified_;

    void setModified();
    void updateCompletion();

public:
    SKKComposingEditor();

    virtual void Input(const std::string& fixed, const std::string& input, char code);
    virtual void Input(Event event);
    virtual void Clear();
    virtual void Output(SKKContextBuffer& buffer) const;
    virtual void Commit(std::string& queue);
    virtual void Flush();
    virtual bool IsModified() const;

    const std::string QueryString() const;
    void SetEntry(const std::string& entry);
    void EnableDynamicCompletion(bool flag);
};

#endif
