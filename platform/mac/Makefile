###
### AquaSKK
###

TARGET	= AquaSKK.app
DEST	= /Library/Input\ Methods
XCODE	= xcodebuild -project proj/AquaSKK.xcodeproj
DEBUG_BUILD	= $(XCODE) -configuration Debug
RELEASE_BUILD	= $(XCODE) -configuration Release
BUILD	= proj/build/Debug

all:
	$(DEBUG_BUILD) build

clean:
	$(DEBUG_BUILD) clean

install: copy
	@pid=`ps -A|awk '/[A]quaSKK.app/ { print $$1; }'`; \
	if [[ "$$pid" -ne "" ]]; then \
		kill $$pid; \
	fi

copy: all
	sudo rm -rf $(DEST)/$(TARGET)
	sudo cp -r $(BUILD)/$(TARGET) $(DEST)
	sync; sync; sync

test:
	cd src/engine/tests; make test;

release_build:
	$(RELEASE_BUILD) build

beta: release_build
	cd Package; make beta; cd ..

release: release_build
	cd Package; make release; cd ..
