# If RACK_DIR is not defined when calling the Makefile, default to two directories above
RACK_DIR ?= ../..

# FLAGS will be passed to both the C and C++ compiler
FLAGS +=
CFLAGS +=
CXXFLAGS +=

FLAGS += -I./3rdparty/midifile/include
FLAGS += -I./src

# Careful about linking to shared libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine, but they should be added to this plugin's build system.
LDFLAGS +=

# Add .cpp files to the build
SOURCES += $(wildcard src/*.cpp)
SOURCES += $(wildcard 3rdparty/midifile/src/*.cpp)



# Add files to the ZIP package when running `make dist`
# The compiled plugin and "plugin.json" are automatically added.
DISTRIBUTABLES += res
DISTRIBUTABLES += $(wildcard LICENSE*)
DISTRIBUTABLES += $(wildcard presets)

# Include the Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk

# build with the address sanitizer. Must run Rack with the asan_rack
# target
asan: ASAN_FLAGS=-fsanitize=address 
asan: ASAN_LD_FLAGS=-fsanitize=address 
asan: debug

# When building for debugging, stop the debug symbols from being stripped, and set the optimization level to none
debug: STRIP = : DEBUG MODE, THIS LINE IS COMMENTED OUT:  strip
debug: EXTRA_FLAGS = -O0 -DSDTDEBUG $(ASAN_FLAGS)
debug: EXTRA_LD_FLAGS=$(ASAN_LD_FLAGS)
debug: install

release: clean install

TEST_SOURCES += $(wildcard test/*.cpp) 
TEST_OBJECTS += $(patsubst %, build/%.o, $(TEST_SOURCES))
TEST_DEPS = $(patsubst %, build/%.d, $(TEST_SOURCES))
# HACK: use .exe extension even on Mac to simplify the makefile 
TEST_EXES = $(patsubst %, build/%.exe, $(TEST_SOURCES))

TEST_LDFLAGS = $(subst -shared,,$(LDFLAGS))

%.exe: $(TEST_OBJECTS) $(OBJECTS)
	$(CXX) -o $@ $^ $(TEST_LDFLAGS)

-include $(TEST_DEPS)
# HACK: Macos specific - librack needs to be in local directory
test: $(TEST_EXES)
	@if [ ! -f ./libRack.dylib ]; then ln -s $(RACK_DIR)/libRack.dylib; fi
	for f in $(TEST_EXES); do $$f; done

dist: test

asan_rack:
	DYLD_INSERT_LIBRARIES=$(wildcard /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/*/lib/darwin/libclang_rt.asan_osx_dynamic.dylib) /Applications/VCV\ Rack\ 2\ Free.app/Contents/MacOS/Rack 
	

