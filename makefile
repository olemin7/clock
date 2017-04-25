#https://github.com/arduino/Arduino/blob/master/build/shared/manpage.adoc
ADIR=/home/ominenko/work/ides/arduino-1.8.2/
COMPILE=$(ADIR)arduino
PROJECT_FOLDER=$(pwd)
PROJECT_NAME=clock.ino
PROJECT=$(PROJECT_FOLDER)$(PROJECT_NAME)
#FLAGS="-v"
FLAGS_TEST=-DTEST -std=c++11
LIB_TEST=
TESTS=CIntensity.cpp
OUT_DIR=./obj/
#---------------------------------------------------------------------
# executables
#---------------------------------------------------------------------
MD := mkdir -p
RM := rm
CC := g++


all: clean build
upload:
	$(COMPILE) --upload $(FLAGS) $(PROJECT)
build:
	$(COMPILE) --verify $(FLAGS) $(PROJECT)
	@echo "done"
clean:
	@echo "clean"
	$(RM) -rf $(OUT_DIR)
dirs:
	$(MD) $(OUT_DIR)
test: dirs $(TESTS)
	g++ $(TESTS) $(FLAGS_TEST) $(LIB_TEST) -o $(OUT_DIR)$(TESTS).o
	./$(OUT_DIR)$(TESTS).o
.PHONY: all upload clean test dirs
