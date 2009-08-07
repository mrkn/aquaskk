// -*- C++ -*-

#ifndef MockStateConfiguration_h
#define MockStateConfiguration_h

#include "SKKStateConfiguration.h"

class MockStateConfiguration : public SKKStateConfiguration {
    bool suppress_newline_on_commit_;
    int max_count_of_inline_candidates_;

    virtual bool SuppressNewlineOnCommit() {
        return suppress_newline_on_commit_;
    }

    virtual int MaxCountOfInlineCandidates() {
        return max_count_of_inline_candidates_;
    }

public:
    MockStateConfiguration(bool flag = true, int count = 5)
        : suppress_newline_on_commit_(flag)
        , max_count_of_inline_candidates_(count)
        {}
};

#endif
