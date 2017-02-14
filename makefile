#https://github.com/arduino/Arduino/blob/master/build/shared/manpage.adoc
ADIR=/home/ominenko/work/arduino-1.8.1/
COMPILE=$(ADIR)arduino
PROJECT=clock.ino
all: clean build
build:
	$(COMPILE) --verify -v $(PROJECT)
clean:
	@echo "clean"
