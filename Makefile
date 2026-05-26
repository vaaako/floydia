# Compiler flags
CXX = g++
INCLUDE_DIRS = -Iinclude -Iinclude/external -Iinclude/external/imgui
CXXFLAGS = $(INCLUDE_DIRS) -std=c++17 -fPIC
# Linking flags
LDLIBS = -lX11 -lGL -lXrandr

# Directories
SRC_DIR   = src
LIB_DIR   = lib
BUILD_DIR = build
OBJ_DIR   = $(BUILD_DIR)/obj

LIB_NAME := floydia

SRC       := $(wildcard $(SRC_DIR)/*.cpp $(SRC_DIR)/*/*.cpp)
RGFW_SRC  := $(SRC_DIR)/rgfwimpl.c
GLAD_SRC  := $(LIB_DIR)/glad/glad.c
IMGUI_SRC := $(LIB_DIR)/imgui/imgui.cpp \
             $(LIB_DIR)/imgui/imgui_draw.cpp \
             $(LIB_DIR)/imgui/imgui_tables.cpp \
             $(LIB_DIR)/imgui/imgui_impl_opengl3.cpp \
             $(LIB_DIR)/imgui/imgui_widgets.cpp
SOURCES = $(SRC) $(RGFW_SRC) $(GLAD_SRC) $(IMGUI_SRC)

# Map every source file to OBJ_DIR/<path>.o
OBJECTS := $(patsubst %, $(OBJ_DIR)/%.o,$(SOURCES))
OBJECTS := $(OBJECTS:.cpp.o=.o)
OBJECTS := $(OBJECTS:.c.o=.o)


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

# src/**/*.cpp
$(OBJ_DIR)/$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# lib/**/*.cpp (imgui)
$(OBJ_DIR)/$(LIB_DIR)/%.o: $(LIB_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# lib/**/*.c (glad)
$(OBJ_DIR)/$(LIB_DIR)/%.o: $(LIB_DIR)/%.c
	@mkdir -p $(dir $@)
	gcc -fPIC $(INCLUDE_DIRS) -MMD -MP -c $< -o $@

# src/*.c (RGFW)
# NOTE: RGFW seems to only work when compiled with a C compiler
$(OBJ_DIR)/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	gcc -fPIC $(INCLUDE_DIRS) -MMD -MP -c $< -o $@


# ----------

clean:
	rm -rf $(BUILD_DIR)

# Debug build
# -Wpadded
debug: CXXFLAGS += -O0 -DRGFW_DEBUG -DFLOYD_DEBUG_MAPPED_BUFFER -DFLOYD_DEBUG_TEXT -g -Wall -Wextra -Wuninitialized -Wunreachable-code
debug: all

# Release build
# -march=native: Optimize for current CPU (may not work on all CPUs)
# -flto: Program analysis at link time
release: CXXFLAGS += -O3 -march=native -ffast-math -DNDEBUG -funroll-loops
release: LDFLAGS += -flto
release: all

vars:
	@echo "CXX: $(CXX)"
	@echo "CXXFLAGS: $(CXXFLAGS)"
	@echo "SOURCES: $(SOURCES)"
	@echo "OBJECTS: $(OBJECTS)"
	@echo "TARGET: $(LIB_NAME)"

# No error if .d files dont exist yet
# After first build dependencies are tracked
DEPS := $(OBJECTS:.o=.d)
-include $(DEPS)

.PHONY: all clean dirs debug release vars
