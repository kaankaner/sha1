# All build targets can be used with -j option.

DEP_DIR:=dep
dbg:=1

OBJ_DIR:=./obj
SRC_DIR:=./src
OUTPUT_DIR:=./bin
OUTPUT_BINARY:=$(OUTPUT_DIR)/main.bin

HOST_COMPILER:=g++

# lib is a symlink to lib directory.
LIB_DIR:=lib

GTEST_DIR:=$(LIB_DIR)/googletest/googletest
GTEST_INC_DIR:=$(GTEST_DIR)/include
#GTEST_LIB_DIR:=$(GTEST_DIR)/build1
GTEST_LIB_DIR:=$(GTEST_DIR)/build_static

CCFLAGS:= 
LDFLAGS:=
LIBRARIES:=
INCLUDES:=

CCFLAGS+=-Werror -Wall -Wextra -pedantic -pedantic-errors
CCFLAGS+=-Wno-unused-function
CCFLAGS+=-g -O3
CCFLAGS+=--std=c++11

LIBRARIES+=-pthread
LIBRARIES+=-lm

LIBRARIES +=-L$(GTEST_LIB_DIR)
INCLUDES  +=-I$(GTEST_INC_DIR)
LIBRARIES +=$(GTEST_LIB_DIR)/libgtest.a
#LIBRARIES+=-lgtest

# All cpp files under src.
CPP_FILES:=$(shell find $(SRC_DIR) -name "*.cpp")

# For each cpp file under src, a .o file under obj.
OBJ_FILES:=$(patsubst $(SRC_DIR)/%, $(OBJ_DIR)/%, $(patsubst %.cpp, %.o, $(CPP_FILES)) ) 

# Not using variable expansion here, in case the variables are not initialized.
clean_routine = rm -f bin/*.bin && \
				rm -rf obj/* \
				rm -rf output/*

# Makes sure that all the project directories exist.
mkdirs_routine = @mkdir -p $(DEP_DIR) && \
	mkdir -p bin && \
	mkdir -p $(SRC_DIR) && \
	mkdir -p $(OBJ_DIR) && \
	mkdir -p output && \
	cd $(SRC_DIR) ; find . -type d -exec mkdir -p -- ../$(OBJ_DIR)/{} \; ; cd .. && \
	cd $(SRC_DIR) ; find . -type d -exec mkdir -p -- ../dep/{} \; ; cd ..




all: build

.PHONY: info
info:
	@echo ALL_CCFLAGS=$(ALL_CCFLAGS)
	@echo CPP_FILES=$(CPP_FILES)
	@echo OBJ_FILES=$(OBJ_FILES)

.PHONY: build
build: mkdirs $(OUTPUT_BINARY)
	@echo "build finished..."
#	@spd-say "build finished"




# For the header dependency trick.
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEP_DIR)/$*.Td

$(DEP_DIR)/%.d: ;
.PRECIOUS: $(DEP_DIR)/%.d



$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEP_DIR)/%.d
	$(HOST_COMPILER) $(DEPFLAGS) $(INCLUDES) $(CCFLAGS) -o $@ -c $< -pthread && mv -f $(DEP_DIR)/$*.Td $(DEP_DIR)/$*.d

$(OUTPUT_BINARY): $(OBJ_FILES)
	$(HOST_COMPILER) $(CCFLAGS) -o $@ $+ $(LIBRARIES) -pthread



.PHONY: run
run: build
	$(OUTPUT_BINARY)

# Measure running time.
.PHONY: time
time: build
	time $(OUTPUT_BINARY)

# Start in gdb.
.PHONY: gdb
gdb: build
	gdb -ex=r $(OUTPUT_BINARY)

# Start in valgrind.
.PHONY: valg
valg: build
	valgrind --leak-check=full --show-leak-kinds=all -v $(OUTPUT_BINARY)

.PHONY: clean
clean: 
	$(call mkdirs_routine)
	$(call clean_routine)
	$(call mkdirs_routine)

.PHONY: cleandeps
cleandeps: clean
	rm -rf dep/*
	$(call mkdirs_routine)

.PHONY: mkdirs
mkdirs:
	$(call mkdirs_routine)

# For the header dependency trick.
include $(shell find $(DEP_DIR)/ -type f -name '*.d')
