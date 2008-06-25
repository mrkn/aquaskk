#include <cassert>
#include "SKKCandidateSuite.h"

void TestAdd() {
    SKKCandidateSuite suite;
    SKKCandidateSuite tmp;
    SKKCandidateContainer cand;

    suite.Add(SKKCandidate("候補"));

    cand.push_back(SKKCandidate("ヒント1"));
    cand.push_back(SKKCandidate("ヒント2"));

    suite.Add(SKKOkuriHint("おくり", cand));

    tmp.Add(suite);

    assert(tmp.ToString() == "/候補/[おくり/ヒント1/ヒント2/]/");
}

void TestUpdate() {
    SKKCandidateSuite suite;
    SKKCandidateSuite tmp;
    SKKCandidateContainer cand;

    suite.Add(SKKCandidate("候補1"));
    suite.Add(SKKCandidate("候補2"));

    cand.push_back(SKKCandidate("候補1"));
    cand.push_back(SKKCandidate("候補2"));

    suite.Add(SKKOkuriHint("おくり", cand));

    suite.Update(SKKCandidate("候補2;アノテーション"));

    assert(suite.ToString() == "/候補2;アノテーション/候補1/[おくり/候補1/候補2/]/");

    cand.clear();
    cand.push_back(SKKCandidate("候補1;アノテーション"));

    suite.Update(SKKOkuriHint("おくり", cand));
    assert(suite.ToString() == "/候補1;アノテーション/候補2;アノテーション/[おくり/候補1;アノテーション/候補2/]/");
}

void TestRemove() {
    SKKCandidateSuite suite;

    suite.Parse("/合;(一致) 話が合う/当/[て/当/]/[って/合;(一致) 話が合う/]/");
    suite.Remove(SKKCandidate("当"));
    assert(suite.ToString() == "/合;(一致) 話が合う/[って/合;(一致) 話が合う/]/");
}

int main() {
    TestAdd();
    TestUpdate();
    TestRemove();
}
