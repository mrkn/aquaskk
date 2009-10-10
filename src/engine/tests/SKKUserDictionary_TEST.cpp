#include <cassert>
#include <iostream>
#include "SKKUserDictionary.h"

int main() {
    SKKUserDictionary dict;
    SKKCandidateSuite suite;

    dict.Initialize("skk-jisyo.utf8");

    dict.FindOkuriAri("おくりあr", suite);
    assert(suite.ToString() == "/送り有/");

    suite.Clear();
    dict.FindOkuriNasi("かんじ", suite);
    assert(suite.ToString() == "/漢字/");

    dict.RegisterOkuriAri("おくりあr", "り", SKKCandidate("送りあ"));
    dict.RegisterOkuriNasi("かんり", SKKCandidate("管理"));

    suite.Clear();

    dict.FindOkuriAri("おくりあr", suite);
    assert(suite.ToString() == "/送りあ/送り有/[り/送りあ/]/");

    suite.Clear();
    dict.FindOkuriNasi("かんり", suite);
    assert(suite.ToString() == "/管理/");

    std::vector<std::string> result;
    assert(dict.FindCompletions("かん", result) && result[0] == "かんり" && result[1] == "かんじ");

    dict.RemoveOkuriAri("おくりあr", SKKCandidate("送りあ"));
    dict.RemoveOkuriNasi("かんり", SKKCandidate("管理"));

    suite.Clear();
    dict.FindOkuriAri("おくりあr", suite);
    assert(suite.ToString() == "/送り有/");

    suite.Clear();
    dict.FindOkuriNasi("かんり", suite);
    assert(suite.IsEmpty());

    dict.SetPrivateMode(true);

    suite.Clear();
    dict.FindOkuriAri("おくりあr", suite);
    assert(suite.ToString() == "/送り有/");

    suite.Clear();
    dict.FindOkuriNasi("かんじ", suite);
    assert(suite.ToString() == "/漢字/");

    dict.RegisterOkuriAri("おくりあr", "り", SKKCandidate("送りあ"));
    dict.RegisterOkuriNasi("かんり", SKKCandidate("管理"));

    suite.Clear();
    dict.FindOkuriAri("おくりあr", suite);
    assert(suite.ToString() == "/送りあ/送り有/[り/送りあ/]/");

    suite.Clear();
    dict.FindOkuriNasi("かんり", suite);
    assert(suite.ToString() == "/管理/");

    dict.SetPrivateMode(false);

    suite.Clear();
    dict.FindOkuriAri("おくりあr", suite);
    assert(suite.ToString() == "/送り有/");

    suite.Clear();
    dict.FindOkuriNasi("かんじ", suite);
    assert(suite.ToString() == "/漢字/");

    assert(dict.FindEntry("漢字") == "かんじ");

    dict.RemoveOkuriNasi("ほかん1", SKKCandidate(""));
    suite.Clear();
    dict.FindOkuriNasi("ほかん1", suite);
    assert(suite.ToString() == "/補完1/");

    dict.RegisterOkuriNasi("とぐるほかん", SKKCandidate(""));
    suite.Clear();
    dict.FindOkuriNasi("とぐるほかん", suite);
    assert(suite.IsEmpty());

    dict.RemoveOkuriNasi("とぐるほかん", SKKCandidate(""));
    suite.Clear();
    dict.FindOkuriNasi("とぐるほかん", suite);
    assert(suite.IsEmpty());

    result.clear();
    assert(!dict.FindCompletions("かん", result, 3));

    suite.Clear();
    dict.RegisterOkuriNasi("encode", SKKCandidate("abc;def"));
    dict.FindOkuriNasi("encode", suite);
    assert(suite.ToString() == "/abc;def/");

    suite.Clear();
    dict.RemoveOkuriNasi("encode", SKKCandidate("abc;def"));
    dict.FindOkuriNasi("encode", suite);
    assert(suite.IsEmpty());
}
