#include <cassert>
#include "SKKUserDictionary.h"

int main() {
    SKKUserDictionary dict;

    dict.Initialize("skk-jisyo.utf8");

    assert(dict.FindOkuriAri("おくりあr") == "/送り有/");
    assert(dict.FindOkuriNasi("かんじ") == "/漢字/");

    dict.RegisterOkuriAri("おくりあr", "り", "送りあ");
    dict.RegisterOkuriNasi("かんり", "管理");

    assert(dict.FindOkuriAri("おくりあr") == "/送りあ/送り有/[り/送りあ/]/");
    assert(dict.FindOkuriNasi("かんり") == "/管理/");

    std::vector<std::string> result;
    assert(dict.FindCompletions("かん", result) && result[0] == "かんり" && result[1] == "かんじ");

    dict.RemoveOkuriAri("おくりあr", "送りあ");
    dict.RemoveOkuriNasi("かんり", "管理");

    assert(dict.FindOkuriAri("おくりあr") == "/送り有/");
    assert(dict.FindOkuriNasi("かんり") == "");

    dict.SetPrivateMode(true);

    assert(dict.FindOkuriAri("おくりあr") == "/送り有/");
    assert(dict.FindOkuriNasi("かんじ") == "/漢字/");

    dict.RegisterOkuriAri("おくりあr", "り", "送りあ");
    dict.RegisterOkuriNasi("かんり", "管理");

    assert(dict.FindOkuriAri("おくりあr") == "/送りあ/送り有/[り/送りあ/]/");
    assert(dict.FindOkuriNasi("かんり") == "/管理/");

    dict.SetPrivateMode(false);

    assert(dict.FindOkuriAri("おくりあr") == "/送り有/");
    assert(dict.FindOkuriNasi("かんじ") == "/漢字/");
}
