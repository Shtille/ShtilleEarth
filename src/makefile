# Makefile for Shtille Earth binary

ROOT_PATH = ..

# Platform-specific defines
ifeq ($(OS),Windows_NT)
	# Windows
	LDFLAGS = -s -mwindows
	TARGET_EXT = .exe
else
	LDFLAGS =
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		# Linux
		TARGET_EXT = .app
	endif
	ifeq ($(UNAME_S),Darwin)
		# Mac OS X
		TARGET_EXT = .app
	endif
endif

# 'TARGET' should coinside with directory name
TARGET = src
TARGET_NAME = ShtilleEarth
TARGET_FILE = $(BINARY_PATH)/$(TARGET_NAME)$(TARGET_EXT)

INCLUDE = \
	-I$(ROOT_PATH)/deps/scythe/include \
	-I$(ROOT_PATH)/deps/scythe/src \
	-I$(ROOT_PATH)/deps/saim/include
DEFINES = 

SRC_DIRS = planet .
SRC_FILES = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.cpp))

# intermediate directory for generated object files
OBJDIR := .o
# intermediate directory for generated dependency files
DEPDIR := .d

# object files, auto generated from source files
OBJECTS := $(patsubst %,$(OBJDIR)/%.o,$(basename $(SRC_FILES)))
# dependency files, auto generated from source files
DEPS := $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRC_FILES)))

# compilers (at least gcc and clang) don't create the subdirectories automatically
ifeq ($(OS),Windows_NT)
$(foreach dir,$(subst /,\\,$(dir $(OBJECTS))),$(shell if not exist $(dir) mkdir $(dir)))
$(foreach dir,$(subst /,\\,$(dir $(DEPS))),$(shell if not exist $(dir) mkdir $(dir)))
else
$(shell mkdir -p $(dir $(OBJECTS)) >/dev/null)
$(shell mkdir -p $(dir $(DEPS)) >/dev/null)
endif

# User library dependencies
DEPENDENT_LIBRARIES = scythe saim
DEPENDENT_LIB_FILES = $(foreach name,$(DEPENDENT_LIBRARIES),$(patsubst %,$(LIBRARY_PATH)/lib%$(STATIC_LIB_EXT),$(name)))

# C++ flags
CXXFLAGS := -std=c++11
# C/C++ flags
CPPFLAGS := -g -Wall -O3
#CPPFLAGS += -Wextra -pedantic
CPPFLAGS += $(INCLUDE)
CPPFLAGS += $(DEFINES)
# linker flags
LDFLAGS += -L$(LIBRARY_PATH)
LDLIBS = -lcurl -lsaim -lscythe -lstdc++ -lfreetype -ljpeg -lpng -lz
ifeq ($(OS),Windows_NT)
	LDLIBS += -lgdi32 -lglew -lopengl32
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		# TODO: Linux-specific libraries
	endif
	ifeq ($(UNAME_S),Darwin)
		LDLIBS += -framework Cocoa -framework OpenGL -framework Foundation
	endif
endif
# flags required for dependency generation; passed to compilers
DEPFLAGS = -MT $@ -MD -MP -MF $(DEPDIR)/$*.Td

# compile C++ source files
COMPILE.cc = $(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c -o $@
# link object files to binary
LINK.o = $(CXX) $(LDFLAGS) $(LDLIBS) -o $@
# precompile step
PRECOMPILE =
# postcompile step
ifeq ($(OS),Windows_NT)
	POSTCOMPILE = MOVE /Y $(DEPDIR)\\$(subst /,\\,$*.Td) $(DEPDIR)\\$(subst /,\\,$*.d)
else
	POSTCOMPILE = mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d
endif

ifeq ($(OS),Windows_NT)
	CLEAN = rmdir /Q /S $(OBJDIR) && rmdir /Q /S $(DEPDIR)
else
	CLEAN = rm -r $(OBJDIR) $(DEPDIR)
endif

all: $(TARGET)

.PHONY: clean
clean:
	@$(CLEAN)

.PHONY: help
help:
	@echo available targets: all clean

$(TARGET): $(TARGET_FILE)

$(TARGET_FILE): $(OBJECTS) $(DEPENDENT_LIB_FILES)
	@echo linking $(TARGET_NAME)$(TARGET_EXT)
	@$(LINK.o) $(OBJECTS)

$(OBJDIR)/%.o: %.cpp
$(OBJDIR)/%.o: %.cpp $(DEPDIR)/%.d
	@$(PRECOMPILE)
	@echo compiling $<
	@$(COMPILE.cc) $<
	@$(POSTCOMPILE)

.PRECIOUS = $(DEPDIR)/%.d
$(DEPDIR)/%.d: ;

-include $(DEPS)