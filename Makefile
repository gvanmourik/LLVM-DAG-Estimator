CLANG_HOME = /Users/gvanmou/Programs/source/llvm-clang-6.0.0/build
PROGRAM_NAME = for_loop_tracker

CC := clang++

INCLUDES := -I$(CLANG_HOME)/include \
			-Iinclude
CXXFLAGS := -fno-rtti \
			`$(CLANG_HOME)/bin/llvm-config --cxxflags --ldflags --system-libs --libs all core mcjit native` \
			-rdynamic \
			-Wno-unused-command-line-argument
LDFLAGS := -std=c++11
LIBS :=

ifeq ($(cfg), true)
	CFLAGS += -DGEN_CFG
endif

TARGET = $(PROGRAM_NAME)
SOURCE = $(PROGRAM_NAME).cpp
OBJECT = $(SOURCE:.cpp=.o)

all : $(TARGET)

debug: CXXFLAGS += -DDEBUG -g
debug: $(TARGET) 

%.o : %.cpp
	@echo "\nCompiling (patience you must have)..."
	$(CC) $(INCLUDES) $(CXXFLAGS) -o $@ -c $<

$(TARGET) : $(OBJECT)
	@echo "\nLinking..."
	$(CC) -o $@ $^ $(INCLUDES) $(CXXFLAGS) $(LIBS)

clean : $(OBJECT)
	@echo "Cleaning up..."
	rm -f *.o $(TARGET)