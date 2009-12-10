#include <cassert>
#include "SKKDictionaryKeeper.h"

class DebugLoader : public SKKDictionaryLoader {
    std::string path_;

    virtual bool NeedsUpdate() {
        return true;
    }

    virtual const std::string& FilePath() const {
        return path_;
    }

public:
    DebugLoader(const std::string& path) : path_(path) {}
};

int main() {
    DebugLoader loader("SKK-JISYO.TEST");
    SKKDictionaryKeeper keeper;

    keeper.Initialize(&loader, 5, 5);

    assert(keeper.FindEntry("官寺") == "かんじ");

    std::vector<std::string> result;
    assert(keeper.FindCompletions("か", result, 0));

    result.clear();
    assert(!keeper.FindCompletions("か", result, 3));
}
