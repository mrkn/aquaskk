#include <cassert>
#include "SKKTrie.h"

int main() {
    SKKTrie root;
    const SKKTrie* node;
    int state;

    node = root.Traverse("a", state);
    assert(state == -1 && !node);

    root.Add("a", SKKTrie("あ", "ア", "ｱ", ""));
    root.Add("kya", SKKTrie("きゃ", "キャ", "ｷｬ", ""));
    root.Add("ka",  SKKTrie("か", "カ", "ｶ", ""));
    root.Add("n", SKKTrie("ん", "ン", "ﾝ", ""));
    root.Add("nn", SKKTrie("ん", "ン", "ﾝ", ""));
    root.Add("xx", SKKTrie("っ", "ッ", "ｯ", "x"));

    node = root.Traverse("m", state);
    assert(state == -1 && !node);

    node = root.Traverse("a", state);
    assert(state == 1 && node && node->KanaString(HirakanaInputMode) == "あ");

    node = root.Traverse("kya", state);
    assert(state == 3 && node && node->KanaString(KatakanaInputMode) == "キャ");

    node = root.Traverse("ki", state);
    assert(state == 1 && !node);

    node = root.Traverse("ky", state);
    assert(state == 0 && !node);

    node = root.Traverse("n", state);
    assert(state == 0 && !node);

    node = root.Traverse("ng", state);
    assert(state == 1 && node && node->KanaString(Jisx0201KanaInputMode) == "ﾝ");

    node = root.Traverse("nn", state);
    assert(state == 2 && node && node->KanaString(Jisx0201KanaInputMode) == "ﾝ");

    node = root.Traverse("xx", state);
    assert(state == 2 && node && node->NextState() == "x");
}
