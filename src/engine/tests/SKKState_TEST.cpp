#include <cassert>
#include <cstdio>
#include "SKKStateMachine.h"
#include "SKKFrontEnd.h"
#include "SKKKeymap.h"
#include "SKKRomanKanaConverter.h"
#include "utf8util.h"

#include "TestFrontEnd.h"
#include "TestCandidateWindow.h"

static SKKKeymap keymap;

SKKState::Event makeEvent(char code, int mods = 0) {
    SKKEvent event = keymap.Fetch(code, 0, mods);
    return SKKState::Event(event.id, event);
}

void bulkInput(const std::string& input, SKKStateMachine& machine) {
    for(std::string::const_iterator ch = input.begin(); ch != input.end(); ++ ch) {
	machine.Dispatch(makeEvent(*ch));
    }
}

int main() {
#if 0
    TestFrontEnd test_frontend;
    SKKFrontEnd frontend(&test_frontend);
    TestCandidateWindow test_window;
    SKKCandidateWindow::Bind(&test_window);
    SKKCandidateWindow window(&frontend);
    SKKInputMode inputmode;
    SKKStateMachine machine(SKKState(&inputmode, &context, &window));
    keymap.Initialize("keymap.conf");

    SKKRomanKanaConverter::theInstance().Initialize("kana-rule.conf");

    machine.Start();

    bulkInput("akashinan", machine);

    if(context.IsModified()) {
	context.Emit(&frontend);
	std::cerr << std::endl;
    }

    bulkInput("Kinen", machine);

    if(context.IsModified()) {
	context.Emit(&frontend);
	std::cerr << std::endl;
    }

    machine.Dispatch(SKK_CANCEL);

    if(context.IsModified()) {
	context.Emit(&frontend);
	std::cerr << std::endl;
    }

    bulkInput("Kanji", machine);
    
    if(context.IsModified()) {
	context.Emit(&frontend);
	std::cerr << std::endl;
    }

    machine.Dispatch(SKK_ENTER);
    
    if(context.IsModified()) {
	context.Emit(&frontend);
	std::cerr << std::endl;
    }

    bulkInput("KesS", machine);
    
    if(context.IsModified()) {
	context.Emit(&frontend);
	std::cerr << std::endl;
    }
#endif
}
