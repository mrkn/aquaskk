/*

  MacOS X implementation of the SKK input method.

  Copyright (C) 2006-2008 Tomotaka SUWA <t.suwa@mac.com>

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

std::string SKKProxyDictionary::FindOkuriAri(const std::string& str) {
    std::string key;
    std::string response;
    std::string result;

    jconv::convert_utf8_to_eucj(str, key);

    // ヒットした？
    if(search(key, response)) {
	jconv::convert_eucj_to_utf8(response, result);
    }

    return result;
}

std::string SKKProxyDictionary::FindOkuriNasi(const std::string& str) {
    std::string key;
    std::string response;
    std::string result;

    jconv::convert_utf8_to_eucj(str, key);

    // ヒットした？
    if(search(key, response)) {
	jconv::convert_eucj_to_utf8(response, result);
    }

    return result;
}

bool SKKProxyDictionary::search(const std::string& str, std::string& response) {
    // 接続する
    if(!session_) {
	session_.open(server_);
	if(!session_) return false;
    }

    // 再入でループするのを防ぐ
    if(active_) return false;
    active_ = true;

    // クエリ送信
    session_ << '1' << str << ' ' << std::flush;

    // 結果受信
    std::getline(session_, response);

    active_ = false;

    if(response[0] == '1') {
	response = response.substr(1);
	return true;
    } else {
	return false;
    }
}

// ファクトリメソッドの登録
#include "SKKDictionaryFactory.h"
static bool initialize = SKKRegisterFactoryMethod<SKKProxyDictionary>(SKKProxyDictionaryType);
