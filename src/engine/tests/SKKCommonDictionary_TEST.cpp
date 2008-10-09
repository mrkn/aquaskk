#include <cassert>
#include "SKKCommonDictionary.h"

int main() {
    SKKCommonDictionary dict;

    dict.Initialize("SKK-JISYO.TEST");

    assert(dict.FindOkuriAri("NOT_EXIST") == "");
    assert(dict.FindOkuriAri("よi") == "/良/好/酔/善/"); 
    assert(dict.FindOkuriNasi("NOT_EXIST") == "");
    assert(dict.FindOkuriNasi("かんじ") == "/漢字/寛治/官寺/");
    assert(dict.FindEntry("漢字") == "かんじ");

    return 0;
}
