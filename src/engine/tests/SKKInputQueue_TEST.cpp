#include <cassert>
#include <iostream>
#include "SKKInputQueue.h"
#include "SKKRomanKanaConverter.h"

class TestInputQueueObserver : public SKKInputQueueObserver {
    std::string fixed_;
    std::string queue_;

public:
    virtual void SKKInputQueueUpdate(const std::string& fixed) {
        fixed_ = fixed;
    }

    virtual void SKKInputQueueUpdate(const std::string& fixed, const std::string& queue) {
        fixed_ = fixed;
        queue_ = queue;
    }

    void Clear() {
        fixed_.clear();
        queue_.clear();
    }

    bool Test(const std::string& fixed, const std::string& queue) {
        return fixed_ == fixed && queue_ == queue;
    }
};

int main() {
    SKKRomanKanaConverter& converter = SKKRomanKanaConverter::theInstance();
    converter.Initialize("kana-rule.conf");

    TestInputQueueObserver observer;
    SKKInputQueue queue(&observer);

    queue.AddChar('a');
    assert(observer.Test("あ", ""));

    queue.AddChar('k');
    assert(observer.Test("", "k"));
    queue.AddChar('y');
    assert(observer.Test("", "ky"));
    queue.RemoveChar();
    assert(observer.Test("", "k"));
    queue.AddChar('i');
    assert(observer.Test("き", ""));

    queue.AddChar('n');
    assert(observer.Test("", "n"));
    queue.Terminate();
    assert(observer.Test("ん", ""));

    queue.AddChar('n');
    assert(queue.CanConvert('i'));
}