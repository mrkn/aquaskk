/*

  MacOS X implementation of the SKK input method.

  Copyright (C) 2007 Tomotaka SUWA <t.suwa@mac.com>

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

#include "SKKAutoUpdateDictionary.h"
#include <sys/stat.h>
#include <cerrno>
#include <sstream>
#include <fstream>
#include "socketutil.h"

// 辞書ダウンロードクラス
class SKKAutoUpdateDictionaryLoader : public SKKDictionaryLoader {
    bool first_;
    short port_;
    std::string host_;
    std::string url_;
    std::string path_;
    std::string tmp_path_;

    virtual bool run() {
	if(first_) {
	    notify();
	}

	net::socket::tcpstream http(host_, port_);

	if(request(http)) {
	    int length = content_length(http);

	    if(download(http, length)) {
		notify();
	    }
	}

	return true;
    }

    bool request(net::socket::tcpstream& http) {
	char timestamp[64];
	struct stat st;

	if(stat(path_.c_str(), &st) != 0) {
	    st.st_mtime = 0;
	}

	// HTTP 日付を生成する(RFC 822, updated by RFC 1123)
	//
	// 例) "Sun, 06 Nov 1994 08:49:37 GMT"
	strftime(timestamp, sizeof(timestamp),
		 "%a, %d %b %Y %T GMT", gmtime(&st.st_mtime));

	http << "GET " << url_ << " HTTP/1.1\r\n";
	http << "Host: " << host_ << "\r\n";
	http << "If-Modified-Since: " << timestamp << "\r\n";
	http << "Connection: close\r\n";
	http << "\r\n" << std::flush;

	return http;
    }

    int content_length(net::socket::tcpstream& http) {
	int length = 0;
	std::string response;

	while(std::getline(http, response) && response != "\r") {
	    if(response.find("HTTP/1.1") != std::string::npos) {
		std::istringstream buf(response);

		// "HTTP/1.1 200" を期待する
		buf >> response >> response;
		if(response != "200") {
		    while(std::getline(http, response)) {}
		    break;
		}
	    }

	    if(response.find("Content-Length") != std::string::npos) {
		std::istringstream buf(response);
		buf >> response >> length;
	    }
	}

	return length;
    }

    bool download(net::socket::tcpstream& http, int length) {
	if(!length) return false;

	std::string line;
	std::ofstream ofs(tmp_path_.c_str());

	while(std::getline(http, line)) {
	    ofs << line << std::endl;
	}

	struct stat st;
	if(stat(tmp_path_.c_str(), &st) < 0 || st.st_size != length) {
	    return false;
	}

	if(rename(tmp_path_.c_str(), path_.c_str()) != 0) {
	    std::cerr << "SKKAutoUpdateDictionaryLoader::download(): rename failed: errno="
		      << errno << std::endl;
	    return false;
	}

	return true;
    }

    void notify() {
	SKKDictionaryFile tmp;

	if(tmp.Load(path_)) {
	    tmp.Sort();
	    NotifyObserver(tmp);
	} else {
            if(first_) {
                first_ = false;
                NotifyObserver(tmp);
            }
        }
    }

public:
    SKKAutoUpdateDictionaryLoader(const std::string& location) : first_(true), port_(80) {
	std::istringstream buf(location);

	buf >> host_ >> url_;

	buf.ignore(1);
	std::getline(buf, path_);

	buf.clear();
	std::replace(host_.begin(), host_.end(), ':', ' ');
	buf.str(host_);

	buf >> host_ >> port_;

	tmp_path_ = path_ + ".download";
    }
};

// ======================================================================

SKKAutoUpdateDictionary::SKKAutoUpdateDictionary() : loader_(0) {}

void SKKAutoUpdateDictionary::Initialize(const std::string& location) {
    if(loader_.get()) return;

    loader_ = std::auto_ptr<SKKAutoUpdateDictionaryLoader>(new SKKAutoUpdateDictionaryLoader(location));
    keeper_.Initialize(loader_.get(), 60 * 60 * 6, 3);
}

std::string SKKAutoUpdateDictionary::FindOkuriAri(const std::string& query) {
    return keeper_.FindOkuriAri(query);
}

std::string SKKAutoUpdateDictionary::FindOkuriNasi(const std::string& query) {
    return keeper_.FindOkuriNasi(query);
}

bool SKKAutoUpdateDictionary::FindCompletions(const std::string& entry, std::vector<std::string>& result) {
    return keeper_.FindCompletions(entry, result);
}

// ファクトリメソッドの登録
#include "SKKDictionaryFactory.h"
static bool initialize = SKKRegisterFactoryMethod<SKKAutoUpdateDictionary>(SKKAutoUpdateDictionaryType);
