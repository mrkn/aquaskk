#include <cassert>
#include <sys/stat.h>
#include "SKKAutoUpdateDictionary.h"

int main() {
    const char* path1 = "SKK-JISYO.S1";
    const char* path2 = "SKK-JISYO.S2";
    SKKAutoUpdateDictionary dict1, dict2;

    remove(path1);
    remove(path2);

    dict1.Initialize("openlab.ring.gr.jp /skk/skk/dic/SKK-JISYO.S SKK-JISYO.S1");
    dict2.Initialize("openlab.ring.gr.jp:80 /skk/skk/dic/SKK-JISYO.S SKK-JISYO.S2");

    dict1.FindOkuriAri("dummy");
    dict2.FindOkuriAri("dummy");

    struct stat st1, st2;

    stat(path1, &st1);
    stat(path2, &st2);

    assert(st1.st_size == st2.st_size);
}
