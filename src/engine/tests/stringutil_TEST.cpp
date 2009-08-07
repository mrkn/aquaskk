#include "stringutil.h"
#include <cassert>
#include <iostream>

int main() {
    string::splitter splitter;
    std::string str;

    splitter.split("a,b,c,d");

    assert(splitter >> str && str == "a");
    assert(splitter >> str && str == "b");
    assert(splitter >> str && str == "c");
    assert(splitter >> str && str == "d");
    assert(!(splitter >> str));

    splitter.split("abc::def::ghi", "::");

    assert(splitter >> str && str == "abc");
    assert(splitter >> str && str == "def");
    assert(splitter >> str && str == "ghi");
    assert(!(splitter >> str));

    splitter.split("::abc:,def:,ghi::", "::");
    assert(splitter >> str && str == "abc:,def:,ghi");
    assert(!(splitter >> str));
}
