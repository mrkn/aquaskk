/* -*- C++ -*-

  MacOS X implementation of the SKK input method.

  Copyright (C) 2006-2007 Tomotaka SUWA <t.suwa@mac.com>

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

#ifndef SKKProxyDictionary_h
#define SKKProxyDictionary_h

#include "SKKBaseDictionary.h"
#include "socketutil.h"

// 外部 skkserv 辞書
class SKKProxyDictionary: public SKKBaseDictionary {
    net::socket::tcpstream session_;
    net::socket::endpoint server_;
    bool active_;

    bool search(const std::string& key, std::string& response);

public:
    SKKProxyDictionary();
    virtual ~SKKProxyDictionary();

    virtual void Initialize(const std::string& path);

    virtual std::string FindOkuriAri(const std::string& query);
    virtual std::string FindOkuriNasi(const std::string& query);
};

#endif
