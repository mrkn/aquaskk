#include <cassert>
#include "SKKFrontEnd.h"

class SKKFrontEndTest : public SKKFrontEnd {
    std::string insert_;
    std::string compose_;
    int cursor_;

public:
    virtual void InsertString(const std::string& str) {
	insert_ = str;
    }

    virtual void ComposeString(const std::string& str, int cursorOffset = 0) {
	compose_ = str;
	cursor_ = cursorOffset;
    }

    virtual void Clear() {
        ComposeString("");
    }

    virtual void ShowCompletion(const std::string&, int) {}
    virtual void HideCompletion() {}

    virtual std::pair<int, int> WindowPosition() const {
        return std::make_pair<int, int>(0, 0);
    }

    virtual int WindowLevel() const { return 0; }

    bool ok(const std::string str, const std::string compose, int cursorOffset) {
	return insert_ == str && compose_ == compose && cursor_ == cursorOffset;
    }
};

int main() {
    SKKFrontEndTest* test = new SKKFrontEndTest();
    SKKFrontEnd* frontend = test;

    frontend->InsertString("str");
    frontend->ComposeString("compose", -10);

    assert(test->ok("str", "compose", -10));

    delete test;
}
