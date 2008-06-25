#include <cassert>
#include "SKKDictionaryFactory.h"
#include "SKKBaseDictionary.h"
#include "SKKCommonDictionary.h"

int main() {
    SKKDictionaryFactory& factory = SKKDictionaryFactory::theInstance();

    SKKBaseDictionary* dict = factory.Create(SKKCommonDictionaryType, "SKK-JISYO.TEST");

    assert(dict->FindOkuriNasi("かんじ") == "/漢字/寛治/官寺/");

    return 0;
}
