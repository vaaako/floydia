CC  = gcc
CXX = g++
AR  = gcc-ar

DEBUG         ?= 0
SINGLE_THREAD ?= 0
EDITOR_PANEL  ?= 0

INCLUDE_DIRS := -Iinclude -Iinclude/external
ifeq ($(EDITOR_PANEL),1)
# Include imgui if compiling with editor panel
INCLUDE_DIRS += -Iinclude/external/imgui
endif

CXXFLAGS := -std=c++17 -fPIC $(INCLUDE_DIRS)
LDFLAGS  :=
LDLIBS    = -lX11 -lGL -lXrandr

CFLAGS := -fPIC $(INCLUDE_DIRS)

# Directories
SRC_DIR   = src
LIB_DIR   = lib
BUILD_DIR = build
OBJ_DIR   = $(BUILD_DIR)/obj
LIB_BASE_NAME = floydia
TARGET_SUFFIX :=

# Debug
ifeq ($(DEBUG),1)
CXXFLAGS += -O0 \
	-g \
	-Wall -Wextra -Wuninitialized -Wunreachable-code \
	-fsanitize=thread \
	-DFLOYD_DEBUG_MAPPED_BUFFER \
	-DFLOYD_DEBUG_TEXT
CFLAGS += -DRGFW_DEBUG

LDFLAGS += -fsanitize=thread
TARGET_SUFFIX += _debug

else
# -march=native: Optimize for current CPU (may not work on all CPUs)
# -flto: Program analysis at link time
CXXFLAGS += -O3 \
	-march=native \
	-ffast-math \
	-funroll-loops \
	-DNDEBUG \
	-flto

CFLAGS += -O3 \
	-march=native \
	-ffast-math \
	-funroll-loops \
	-DNDEBUG \
	-flto

LDFLAGS += -flto
endif

# Single thread
ifeq ($(SINGLE_THREAD),1)
CXXFLAGS += -DFLOYD_SINGLE_THREAD
TARGET_SUFFIX += _single_thread
endif

# Editor panel
ifeq ($(EDITOR_PANEL),1)
CXXFLAGS += -DFLOYD_EDITOR_PANEL
TARGET_SUFFIX += _editor
endif

# Final library name
LIB_NAME   := $(LIB_BASE_NAME)$(TARGET_SUFFIX)
STATIC_LIB := $(BUILD_DIR)/lib$(LIB_NAME).a
SHARED_LIB := $(BUILD_DIR)/lib$(LIB_NAME).so

# Sources
SRC       := $(wildcard $(SRC_DIR)/*.cpp $(SRC_DIR)/*/*.cpp)
RGFW_SRC  := $(SRC_DIR)/rgfwimpl.c
GLAD_SRC  := $(LIB_DIR)/glad/glad.c
IMGUI_SRC := $(LIB_DIR)/imgui/imgui.cpp \
             $(LIB_DIR)/imgui/imgui_draw.cpp \
             $(LIB_DIR)/imgui/imgui_tables.cpp \
             $(LIB_DIR)/imgui/imgui_widgets.cpp \
             $(LIB_DIR)/imgui/imgui_impl_opengl3.cpp
SOURCES := $(SRC) $(RGFW_SRC) $(GLAD_SRC)
ifeq ($(EDITOR_PANEL),1)
SOURCES += $(IMGUI_SRC)
endif

# Map every source file to OBJ_DIR/<path>.o
OBJECTS := $(patsubst %, $(OBJ_DIR)/%.o,$(SOURCES))
OBJECTS := $(OBJECTS:.cpp.o=.o)
OBJECTS := $(OBJECTS:.c.o=.o)



# ----------
# Default targets
all: $(STATIC_LIB) $(SHARED_LIB)

# Static library
$(STATIC_LIB): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $^

# Shared library
$(SHARED_LIB): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CXX) -shared $(LDFLAGS) -o $@ $^ $(LDLIBS)

# src/**/*.cpp
$(OBJ_DIR)/$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

ifeq ($(EDITOR_PANEL),1)
# lib/**/*.cpp (imgui)
$(OBJ_DIR)/$(LIB_DIR)/%.o: $(LIB_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@
endif

# lib/**/*.c (glad)
$(OBJ_DIR)/$(LIB_DIR)/%.o: $(LIB_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# src/*.c (RGFW)
# NOTE: RGFW seems to only work when compiled with a C compiler
$(OBJ_DIR)/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@


# ----------

clean:
	rm -rf $(BUILD_DIR)

vars: all
	@echo "LIBRARY  : $(LIB_NAME)"
	@echo "DEBUG    : $(DEBUG)"
	@echo "SINGLE   : $(SINGLE_THREAD)"
	@echo "EDITOR   : $(EDITOR_PANEL)"
	@echo "SOURCES  : $(SOURCES)"
	@echo "OBJECTS  : $(OBJECTS)"
	@echo "TARGET   : $(LIB_NAME)"

# No error if .d files dont exist yet
# After first build dependencies are tracked
DEPS := $(OBJECTS:.o=.d)
-include $(DEPS)

.PHONY: all clean vars
