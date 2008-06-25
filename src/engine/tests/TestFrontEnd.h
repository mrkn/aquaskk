#ifndef TestFrontEnd_h
#define TestFrontEnd_h

#include "SKKFrontEnd.h"
#include <sstream>

class TestFrontEnd : public SKKFrontEnd {
    std::stringstream fixed_;
    std::stringstream composing_;

    virtual void InsertString(const std::string& str) {
	fixed_ << str;
    }

    virtual void ComposeString(const std::string& str, int cursorOffset) {
	composing_ << str << "(" << cursorOffset << ")";
    }

    virtual std::pair<int, int> WindowPosition() const {
        return std::make_pair<int, int>(0, 0);
    }

    virtual int WindowLevel() const { return 0; }

public:
    void Clear() {
	fixed_.str("");
	composing_.str("");
    }

    std::string Fixed() { return fixed_.str(); }
    std::string Composing() { return composing_.str(); }
};

#endif
