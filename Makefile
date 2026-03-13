# Compiler flags
CXX = g++
INCLUDE_DIRS = -Iinclude -Iinclude/external
CXXFLAGS = $(INCLUDE_DIRS) -std=c++17 -fPIC
# Linking flags
LDLIBS = -lX11 -lGL -lXrandr

# Directories
SRC_DIR = src
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj

LIB_NAME := floydia

SOURCES = $(wildcard $(SRC_DIR)/*/*.cpp) $(SRC_DIR)/glad.c
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
	ar rcs $@ $^

# Shared library
$(SHARED_LIB): $(OBJECTS)
	$(CXX) -shared -o $@ $^ $(LDLIBS)

# compile source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	gcc -fPIC -Iinclude/external -c $< -o $@


# ----------


clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean dirs debug release

# Debug build
debug: CXXFLAGS += -O0 -g -Wall -Wextra -Wuninitialized -Wunreachable-code
# debug: CXXFLAGS += -O0 -g -Wall -Wextra -Wpadded -Wuninitialized -Wunreachable-code
debug: all

# Release build
# -march=native -> Optimize for current CPU (may not work on all CPUs)
release: CXXFLAGS += -O3 -march=native -ffast-math -DNDEBUG
release: all

vars:
	@echo "CXX: $(CXX)"
	@echo "CXXFLAGS: $(CXXFLAGS)"
	@echo "SOURCES: $(SOURCES)"
	@echo "OBJECTS: $(OBJECTS)"
	@echo "TARGET: $(LIB_NAME)"
