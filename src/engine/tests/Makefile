###
### Makefile for unit tests
###

VPATH	= $(filter-out %/.svn, $(wildcard ../*)) ../../bridge
CXXFLAGS= $(foreach dir,$(VPATH),-I$(dir)) -I.. -g -Wall
TESTSRCS= $(wildcard *_TEST.cpp)
TESTOBJS= $(TESTSRCS:.cpp=.o)
TESTBINS= $(TESTSRCS:.cpp=)
DEPDIR	= .depend

LIB	= libskk.a

DICT_OBJS = SKKCommonDictionary.o SKKAutoUpdateDictionary.o \
	SKKUserDictionary.o SKKProxyDictionary.o SKKGadgetDictionary.o

BACKEND_OBJS = SKKBackEnd.o SKKDictionaryCache.o SKKDictionaryFactory.o \
	SKKNumericConverter.o

EDITOR_OBJS = SKKPrimaryEditor.o  SKKComposingEditor.o SKKOkuriEditor.o \
	SKKCandidateEditor.o SKKEntryRemoveEditor.o SKKRegisterEditor.o \
	SKKInputEngine.o SKKContextBuffer.o SKKTextBuffer.o SKKInputQueue.o \
	SKKOutputQueue.o

KEYMAP_OBJS = SKKKeymapEntry.o SKKKeymap.o

SELECTOR_OBJS = SKKSelector.o SKKInlineSelector.o SKKWindowSelector.o

SESSION_OBJS = SKKInputSession.o SKKInputEngine.o

TRIE_OBJS = SKKRomanKanaConverter.o SKKTrie.o

OTHER_OBJS = SKKEntry.o SKKCandidate.o \
	SKKDictionaryFile.o SKKDictionaryKeeper.o \
	SKKCompleter.o SKKState.o SKKInputMode.o \
	jconv.o

LIBOBJS	= $(DICT_OBJS) $(BACKEND_OBJS) $(KEYMAP_OBJS) $(SELECTOR_OBJS) \
	$(EDITOR_OBJS) $(SESSION_OBJS) $(TRIE_OBJS) $(OTHER_OBJS)

LDFLAGS	= -L. -lskk

all: $(TESTBINS)

test: all
	@for test in $(TESTBINS); do echo $$test && ./$$test > /dev/null; done

clean:
	$(RM) -r $(TESTBINS) $(LIB) *.o $(DEPDIR)

$(LIB): $(LIBOBJS)
	$(AR) -rsu $@ $(LIBOBJS)

$(TESTBINS): %: %.o $(LIB) $(DICT_OBJS)
	$(CXX) -o $@ $< $(LDFLAGS)

$(DEPDIR)/%.d: %.cpp $(DEPDIR)
	$(CXX) -MM $(CXXFLAGS) $< | sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' > $@

$(DEPDIR):
	@mkdir $@

ifneq ($(MAKECMDGOALS), clean)
-include $(addprefix $(DEPDIR)/,$(DICT_OBJS:.o=.d) $(LIBOBJS:.o=.d) $(TESTOBJS:.o=.d))
endif
