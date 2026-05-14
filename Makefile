# Compiler flags
CXX = g++
INCLUDE_DIRS = -Iinclude -Iinclude/external
CXXFLAGS = $(INCLUDE_DIRS) -std=c++17 -fPIC
# Linking flags
LDLIBS = -lX11 -lGL -lXrandr lib/libfreetype.a

# Directories
SRC_DIR = src
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj

LIB_NAME := floydia

SOURCES = $(wildcard $(SRC_DIR)/*/*.cpp) $(SRC_DIR)/libsimpl.c $(SRC_DIR)/glad.c
# Generate object file names from source files
OBJECTS = $(patsubst $(SRC_DIR)/%, $(OBJ_DIR)/%, $(SOURCES))
OBJECTS := $(OBJECTS:.cpp=.o)
OBJECTS := $(OBJECTS:.c=.o)


# Libraries
STATIC_LIB = $(BUILD_DIR)/lib$(LIB_NAME).a
SHARED_LIB = $(BUILD_DIR)/lib$(LIB_NAME).so

# ----------
# Default targets
all: dirs $(STATIC_LIB) $(SHARED_LIB)

# Create build directories
dirs:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(OBJ_DIR)

# Static library
$(STATIC_LIB): $(OBJECTS)
	gcc-ar rcs $@ $^

# Shared library
$(SHARED_LIB): $(OBJECTS)
	$(CXX) -shared -o $@ $^ $(LDLIBS)

# compile source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	gcc -fPIC -Iinclude/ -Iinclude/external -MMD -MP -c $< -o $@


# ----------


clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean dirs debug release

# Debug build
# -Wpadded
# debug: CXXFLAGS += -O0 -fsanitize=address -g -Wall -Wextra -Wuninitialized -Wunreachable-code
debug: CXXFLAGS += -O0 -DFLOYD_DEBUG_MAPPED_BUFFER -g -Wall -Wextra -Wuninitialized -Wunreachable-code -Wpadded
debug: all

# Release build
# -march=native -> Optimize for current CPU (may not work on all CPUs)
# -flto -> Program analysis at link time
release: CXXFLAGS += -O3 -march=native -ffast-math -DNDEBUG -funroll-loops
release: LDFLAGS += -flto
release: all

vars:
	@echo "CXX: $(CXX)"
	@echo "CXXFLAGS: $(CXXFLAGS)"
	@echo "SOURCES: $(SOURCES)"
	@echo "OBJECTS: $(OBJECTS)"
	@echo "TARGET: $(LIB_NAME)"

DEPS := $(OBJECTS:.o=.d)
# No error if .d files dont exist yet
# After first build dependencies are tracked
-include $(DEPS)
