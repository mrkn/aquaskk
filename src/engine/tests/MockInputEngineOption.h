// -*- C++ -*-

#ifndef MockInputEngineOption_h
#define MockInputEngineOption_h

#include "SKKInputEngineOption.h"

class MockInputEngineOption : public SKKInputEngineOption {
    virtual bool FixIntermediateConversion() { return true; }
    virtual bool EnableDynamicCompletion() { return false; }
    virtual int DynamicCompletionRange() { return 0; }
    virtual bool EnableAnnotation() { return false; }
    virtual bool DisplayShortestMatchOfKanaConversions() { return false; }
};

#endif
