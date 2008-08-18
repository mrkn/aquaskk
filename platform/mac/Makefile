###
### AquaSKK
###

TARGET	= AquaSKK.app
DEST	= /Library/Input\ Methods
WORKDIR	= /tmp/AquaSKK
XCODE	= xcodebuild -project proj/AquaSKK.xcodeproj
DEBUG_BUILD	= $(XCODE) -configuration Debug
RELEASE_BUILD	= $(XCODE) -configuration Release
PKGMAKER	= /Developer/usr/bin/packagemaker
PKGOPTION	= -b -i jp.sourceforge.aquaskk.pkg -d AquaSKK.pmdoc
FREEZE	= /usr/local/bin/freeze
HDIMAGE	= hdiutil create -ov -srcfolder $(WORKDIR)
BUILD	= proj/build/Debug
TODAY	= `date +%Y%m%d`

all:
	$(DEBUG_BUILD) build

clean:
	$(DEBUG_BUILD) clean
	$(RELEASE_BUILD) clean

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
	cd ../../src/engine/tests; make test;

release:
	$(RELEASE_BUILD) build

pkg: release
	rm -fr AquaSKK.pkg
	$(FREEZE) proj/AquaSKK.packproj

dmg: pkg
	rm -fr $(WORKDIR)
	mkdir $(WORKDIR)
	mv AquaSKK.pkg $(WORKDIR)
	$(HDIMAGE) /tmp/AquaSKK-$(TODAY).dmg

package: release
	rm -fr $(WORKDIR)
	mkdir $(WORKDIR)
	cd proj; $(PKGMAKER) $(PKGOPTION) -o $(WORKDIR)/AquaSKK.pkg; cd ..
