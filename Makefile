CLANG_HOME = /Users/gvanmou/Programs/source/llvm-clang-6.0.0/build/
PROGRAM_NAME = for_loop_tracker
EXE_NAME = run_loop_tracker

CC := clang++

INCLUDES := -I$(CLANG_HOME)/include \
			-I./
CXXFLAGS := -fno-use-cxa-atexit \
			-fno-rtti \
			`llvm-config --cxxflags --ldflags --system-libs --libs all core mcjit native` \
			-rdynamic
CFLAGS := -fno-rtti \
		  `llvm-config --cxxflags`
LDFLAGS := -std=c++11
LIBS :=

TARGET = $(EXE_NAME)
SOURCE = $(PROGRAM_NAME).cpp
OBJECT = $(SOURCE:.cpp=.o)

all : $(TARGET)

debug: CXXFLAGS += -DDEBUG -g
debug: CPPFLAGS += -DDEBUG -g
debug: $(TARGET) 

%.o : %.cpp
	$(CC) $(INCLUDES) $(CFLAGS) -o $@ -c $<

$(TARGET) : $(OBJECT)
	$(CC) -o $@ $^ $(INCLUDES) $(CXXFLAGS) $(LIBS)

clean : $(OBJECT)
	@echo Cleaning up...
	rm -f *.o $(TARGET)