#include <cassert>
#include "SKKDictionaryKeeper.h"

class DebugLoader : public SKKDictionaryLoader {
    std::string path_;

    virtual bool run() {
        SKKDictionaryFile file;

        if(file.Load(path_)) {
            file.Sort();
            NotifyObserver(file);
        }

        return true;
    }

public:
    DebugLoader(const std::string& path) : path_(path) {}
};

int main() {
    DebugLoader loader("SKK-JISYO.TEST");
    SKKDictionaryKeeper keeper;

    keeper.Initialize(&loader, 5, 5);

    assert(keeper.FindEntry("官寺") == "かんじ");
}
