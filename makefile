#https://github.com/arduino/Arduino/blob/master/build/shared/manpage.adoc
ADIR=/home/ominenko/work/ides/arduino-1.8.1/
COMPILE=$(ADIR)arduino
PROJECT_FOLDER=/cryptfs/tmp/hobby/clock/
PROJECT_NAME=clock.ino
PROJECT=$(PROJECT_FOLDER)$(PROJECT_NAME)
#FLAGS="-v"
all: clean build
upload:
	$(COMPILE) --upload $(FLAGS) $(PROJECT)
build:
	$(COMPILE) --verify $(FLAGS) $(PROJECT)
	@echo "done"
clean:
	@echo "clean"
