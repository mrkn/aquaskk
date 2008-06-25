#include <cassert>
#include <errno.h>
#include "SKKProxyDictionary.h"
#include "SKKCommonDictionary.h"
#include "jconv.h"

void session(int fd, SKKCommonDictionary& dict) {
    net::socket::tcpstream sock(fd);
    unsigned char cmd;

    do {
        cmd = sock.get();
        switch(cmd) {
        case '0':		// 切断
            break;

	case '1': {		// 検索
	    std::string word;
            sock >> word;
            sock.get();

	    std::string key;
	    std::string result;

	    jconv::convert_eucj_to_utf8(word, key);

	    // 検索文字列の最後が [a-z] なら『送りあり』
	    if(1 < key.size() && 0x7f < (unsigned)key[0] && std::isalpha(key[key.size() - 1])) {
		result = dict.FindOkuriAri(key);
	    } else {
		result = dict.FindOkuriNasi(key);
	    }

	    // 見つかった？
	    if(!result.empty()) {
		std::string candidates;
		jconv::convert_utf8_to_eucj(result, candidates);
		sock << '1' << candidates << std::endl;
	    } else {
		sock << '4' << word << std::endl;
	    }
	    sock << std::flush;
	}
            break;

        default:		// 無効なコマンド
	    sock << '0' << std::flush;
            break;
	}
    } while(sock.good() && cmd != '0');
    sock.close();
}

void* server(void* param) {
    SKKCommonDictionary dict;

    dict.Initialize("SKK-JISYO.TEST");

    net::socket::tcpserver skkserv(23000);

    while(true) {
	session(skkserv.accept(), dict);
    }

    return 0;
}

int main() {
    pthread_t thread;

    pthread_create(&thread, 0, server, 0);
    pthread_detach(thread);

    SKKProxyDictionary proxy;

    proxy.Initialize("127.0.0.1:23000");

    assert(proxy.FindOkuriAri("よi") == "/良/好/酔/善/");
    assert(proxy.FindOkuriAri("NOT-EXIST") == "");
    assert(proxy.FindOkuriNasi("かんじ") == "/漢字/寛治/官寺/");
    assert(proxy.FindOkuriNasi("NOT-EXIST") == "");
}
