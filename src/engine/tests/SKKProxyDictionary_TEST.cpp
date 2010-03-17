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
	    std::string key;
            sock >> word;
            sock.get();

	    jconv::convert_eucj_to_utf8(word, key);

	    SKKCandidateSuite result;
            SKKEntry entry(key);

	    // 検索文字列の最後が [a-z] なら『送りあり』
	    if(1 < key.size() && 0x7f < (unsigned)key[0] && std::isalpha(key[key.size() - 1])) {
                entry = SKKEntry(key, "dummy");
	    }

            dict.Find(entry, result);

	    // 見つかった？
	    if(!result.IsEmpty()) {
		std::string candidates;
		jconv::convert_utf8_to_eucj(result.ToString(), candidates);
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
    SKKCandidateSuite suite;

    proxy.Initialize("127.0.0.1:23000");

    proxy.Find(SKKEntry("よi", "い"), suite);
    assert(suite.ToString() == "/良/好/酔/善/");

    suite.Clear();
    proxy.Find(SKKEntry("NOT-EXIST", "i"), suite);
    assert(suite.IsEmpty());

    proxy.Find(SKKEntry("かんじ"), suite);
    assert(suite.ToString() == "/漢字/寛治/官寺/");

    suite.Clear();
    
    proxy.Find(SKKEntry("NOT-EXIST"), suite);
    assert(suite.IsEmpty());
}

