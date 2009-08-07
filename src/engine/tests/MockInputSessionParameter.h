// -*- C++ -*-

#ifndef MockInputSessionParameter_h
#define MockInputSessionParameter_h

#include "SKKInputSessionParameter.h"
#include "MockFrontEnd.h"
#include "MockStateConfiguration.h"
#include "MockMessenger.h"
#include "MockClipboard.h"
#include "MockCandidateWindow.h"
#include "MockAnnotator.h"
#include "MockDynamicCompletor.h"
#include "MockInputEngineOption.h"

class MockInputSessionParameter : public SKKInputSessionParameter {
    MockFrontEnd* frontend_;
    MockStateConfiguration config_;
    MockMessenger messenger_;
    MockClipboard clipboard_;
    MockCandidateWindow candidate_;
    MockAnnotator annotator_;
    MockDynamicCompletor completor_;
    MockInputEngineOption option_;

public:
    MockInputSessionParameter() : frontend_(new MockFrontEnd()) {}
    
    virtual SKKFrontEnd* FrontEnd() { return frontend_; }
    virtual SKKStateConfiguration* StateConfiguration() { return &config_; }
    virtual SKKMessenger* Messenger() { return &messenger_; }
    virtual SKKClipboard* Clipboard() { return &clipboard_; }
    virtual SKKCandidateWindow* CandidateWindow() { return &candidate_; }
    virtual SKKAnnotator* Annotator() { return &annotator_; }
    virtual SKKDynamicCompletor* DynamicCompletor() { return &completor_; }
    virtual SKKInputEngineOption* InputEngineOption() { return &option_; }

    SKKInputModeListener* Listener() { return frontend_; }
    TestResult& Result() { return *frontend_; }
    void SetSelectedString(const std::string& str) { frontend_->SetSelectedString(str); }
    void SetYankString(const std::string& str) { clipboard_.SetString(str); }
};

#endif
