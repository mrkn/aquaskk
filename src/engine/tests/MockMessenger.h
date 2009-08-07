// -*- C++ -*-

#ifndef MockMessenger_h
#define MockMessenger_h

#include "SKKMessenger.h"

class MockMessenger : public SKKMessenger {
    virtual void SendMessage(const std::string& msg) {}
};

#endif
