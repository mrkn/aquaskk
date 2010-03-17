/*

  MacOS X implementation of the SKK input method.

  Copyright (C) 2006-2010 Tomotaka SUWA <tomotaka.suwa@gmail.com>

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

#include <sstream>
#include "SKKProxyDictionary.h"
#include "jconv.h"

SKKProxyDictionary::SKKProxyDictionary() : active_(false) {}

SKKProxyDictionary::~SKKProxyDictionary() {
    session_.close();
}

void SKKProxyDictionary::Initialize(const std::string& location) {
    std::string host(location);
    std::replace(host.begin(), host.end(), ':', ' ');
    std::istringstream buf(host);

    unsigned short port;
    buf >> host >> port;

    server_ = net::socket::endpoint(host, port);
}

void SKKProxyDictionary::Find(const SKKEntry& entry, SKKCandidateSuite& result) {
    // 接続する
    if(!session_) {
	session_.open(server_);
	if(!session_) return;
    }

    // 再入でループするのを防ぐ
    if(active_) return;
    active_ = true;

    // クエリ送信
    session_ << '1' << jconv::eucj_from_utf8(entry.EntryString()) << ' ' << std::flush;

    // 結果受信
    std::string response;
    std::getline(session_, response);

    active_ = false;

    if(response[0] == '1') {
        SKKCandidateSuite suite(jconv::utf8_from_eucj(response.substr(1)));

        result.Add(suite);
    }
}
