/* -*- C++ -*-

  MacOS X implementation of the SKK input method.

  Copyright (C) 2002 phonohawk
  Copyright (C) 2005-2008 Tomotaka SUWA <t.suwa@mac.com>

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
/*
    Directory Maneger対応　2002.09.25 Shin_ichi Abe.
*/

#ifndef SKKKotoeriDictionary_h
#define SKKKotoeriDictionary_h

#include "SKKBaseDictionary.h"

class KotoeriImpl;

class SKKKotoeriDictionary : public SKKBaseDictionary {
    std::auto_ptr<KotoeriImpl> impl_;

public:
    SKKKotoeriDictionary();
    virtual ~SKKKotoeriDictionary();

    virtual void Initialize(const std::string& location);

    virtual std::string FindOkuriAri(const std::string& str);
    virtual std::string FindOkuriNasi(const std::string& str);
};

#endif
