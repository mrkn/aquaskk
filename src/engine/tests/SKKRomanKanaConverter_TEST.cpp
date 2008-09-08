#include <cassert>
#include "SKKRomanKanaConverter.h"

int main() {
    SKKRomanKanaConverter& conv = SKKRomanKanaConverter::theInstance();

    conv.Initialize("kana-rule.conf");

    bool result;
    std::string in;
    std::string out;
    std::string next;

    result = conv.Execute(HirakanaInputMode, "a", out, next);
    assert(result == true && next == "" && out == "あ");

    result = conv.Execute(KatakanaInputMode, "a", out, next);
    assert(next == "" && out == "ア");
    result = conv.Execute(Jisx0201KanaInputMode, "a", out, next);
    assert(next == "" && out == "ｱ");

    result = conv.Execute(HirakanaInputMode, "gg", out, next);
    assert(next == "g" && out == "っ");

    result = conv.Execute(HirakanaInputMode, ",", out, next);
    assert(next == "" && out == "、");

    result = conv.Execute(HirakanaInputMode, "#", out, next);
    assert(next == "" && out == "＃");

    result = conv.Execute(HirakanaInputMode, " ", out, next);
    assert(next == "" && out == " ");

    result = conv.Execute(HirakanaInputMode, "kyl", out, next);
    assert(next == "" && out == "l");

    result = conv.Execute(HirakanaInputMode, "z,", out, next);
    assert(next == "" && out == "‥");

    result = conv.Execute(HirakanaInputMode, "co", out, next);
    assert(next == "" && out == "お");

    result = conv.Execute(HirakanaInputMode, "'", out, next);
    assert(next == "" && out == "'");

    result = conv.Execute(HirakanaInputMode, "k1", out, next);
    assert(next == "" && out == "1");

    result = conv.Execute(HirakanaInputMode, "kgya", out, next);
    assert(next == "" && out == "ぎゃ");

    result = conv.Execute(HirakanaInputMode, "k1234gya", out, next);
    assert(next == "" && out == "1234ぎゃ");

    result = conv.Execute(HirakanaInputMode, "gyagyugyo", out, next);
    assert(next == "" && out == "ぎゃぎゅぎょ");

    result = conv.Execute(HirakanaInputMode, "chho", out, next);
    assert(next == "" && out == "ほ");

    result = conv.Execute(HirakanaInputMode, "c", out, next);
    assert(next == "c" && out == "");

    result = conv.Execute(HirakanaInputMode, "pmp", out, next);
    assert(next == "p" && out == "");

    result = conv.Execute(HirakanaInputMode, "pmpo", out, next);
    assert(next == "" && out == "ぽ");

    result = conv.Execute(HirakanaInputMode, "kanji", out, next);
    assert(next == "" && out == "かんじ");

    result = conv.Execute(HirakanaInputMode, "/", out, next);
    assert(next == "" && out == "/");

    result = conv.Execute(HirakanaInputMode, "z ", out, next);
    assert(next == "" && out == "　");
}
