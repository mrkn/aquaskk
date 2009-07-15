#include <iostream>
#include "SKKGadgetDictionary.h"

int main() {
    SKKGadgetDictionary dict;

    dict.Initialize("hoge");

    SKKCandidateSuite suite;
    dict.FindOkuriNasi("today", suite);
    dict.FindOkuriNasi("now", suite);
    dict.FindOkuriNasi("=(32768+64)*1024", suite);

    std::cerr << suite.ToString() << std::endl;
}
