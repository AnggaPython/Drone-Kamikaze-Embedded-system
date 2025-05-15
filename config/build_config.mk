# Konfigurasi Build untuk Drone Kamikaze Project

# Compiler dan flags
CXX := g++
CXXFLAGS := -std=c++03 -pedantic -Wall -Wextra -Werror
CXXFLAGS += -fno-exceptions -fno-rtti -fno-threadsafe-statics
CXXFLAGS += -Os -ffunction-sections -fdata-sections
CXXFLAGS += -DNDEBUG -D_MISRA_CPP_COMPLIANT

# Flags khusus untuk MISRA compliance
MISRA_FLAGS := -Wcast-align -Wcast-qual -Wconversion
MISRA_FLAGS += -Wfloat-equal -Wformat=2 -Winline -Wmissing-declarations
MISRA_FLAGS += -Wmissing-include-dirs -Wredundant-decls -Wshadow
MISRA_FLAGS += -Wswitch-default -Wswitch-enum -Wundef -Wuninitialized
MISRA_FLAGS += -Wunused -Wunused-parameter -Wvla

# Linker flags
LDFLAGS := -Wl,--gc-sections

# Direktori
SRC_DIR := src
INC_DIR := include
OBJ_DIR := obj
BIN_DIR := bin
DOC_DIR := docs
TEST_DIR := tests

# Include paths
INC_PATHS := -I$(INC_DIR) -I$(SRC_DIR)

# Source files
SRC_FILES := $(wildcard $(SRC_DIR)/**/*.cpp) $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))

# Output binary
TARGET := $(BIN_DIR)/kamikaze_drone

# Target utama
all: dirs $(TARGET)

# Membuat direktori yang diperlukan
dirs:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(OBJ_DIR)/core
	@mkdir -p $(OBJ_DIR)/navigation
	@mkdir -p $(OBJ_DIR)/control
	@mkdir -p $(OBJ_DIR)/communication
	@mkdir -p $(OBJ_DIR)/targeting
	@mkdir -p $(OBJ_DIR)/safety

# Membuat binary
$(TARGET): $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) $(MISRA_FLAGS) $(LDFLAGS) $^ -o $@

# Kompilasi source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(MISRA_FLAGS) $(INC_PATHS) -c $< -o $@

# Clean
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Tests
tests:
	$(MAKE) -C $(TEST_DIR)

# Documentation
docs:
	doxygen Doxyfile

.PHONY: all dirs clean tests docs 