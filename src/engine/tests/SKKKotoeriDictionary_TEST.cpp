#include <cassert>
#include <iostream>
#include <sys/types.h>
#include <pwd.h>
#include "SKKKotoeriDictionary.h"

std::string user_dictionary() {
    struct passwd* passwd = getpwnam(getlogin()); 

    std::string path(passwd->pw_dir);

    endpwent();

    path += "/Library/Dictionaries/ユーザ辞書";

    return path;
}

int main() {
    SKKKotoeriDictionary dict;

    dict.Initialize(user_dictionary());
    SKKCandidateSuite suite;

    dict.FindOkuriAri("てすと", suite);
    assert(suite.IsEmpty());

    dict.FindOkuriNasi("ことえりてすと", suite);
    assert(suite.ToString() == "/[2f][5b][3b]/言選りテスト/");

    suite.Clear();
    dict.FindOkuriNasi("NOT-EXIST", suite);
    assert(suite.IsEmpty());
}
