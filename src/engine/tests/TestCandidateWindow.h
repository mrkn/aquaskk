#ifndef TestCandidateWindow_h
#define TestCandidateWindow_h

#include "SKKCandidateWindow.h"

class TestCandidateWindow : public SKKCandidateWindow {
public:
    virtual void Setup(SKKCandidateIterator begin, SKKCandidateIterator end, std::vector<int>& pages) {
	pages.push_back(end - begin);
    }
    virtual void Show(SKKCandidateIterator begin, SKKCandidateIterator end, int cursor, int page_pos, int page_max) {}
    virtual void Hide() {}
    virtual void Activate() {}
    virtual void Deactivate() {}
    virtual int LabelIndex(char label) { return 0; }
};

#endif
