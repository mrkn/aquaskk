#include <cassert>
#include "SKKCommonDictionary.h"

int main() {
    SKKCommonDictionary dict;

    dict.Initialize("SKK-JISYO.TEST");

    SKKCandidateSuite suite;
    dict.FindOkuriAri("NOT_EXIST", suite);
    assert(suite.IsEmpty());

    dict.FindOkuriAri("よi", suite);
    assert(suite.ToString() == "/良/好/酔/善/");

    suite.Clear();
    dict.FindOkuriNasi("NOT_EXIST", suite);
    assert(suite.IsEmpty());

    dict.FindOkuriNasi("かんじ", suite);
    assert(suite.ToString() == "/漢字/寛治/官寺/");

    assert(dict.FindEntry("漢字") == "かんじ");

    return 0;
}
