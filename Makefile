CXX                = gcc
CXXFLAGS_COMMON    = -std=c17 -Wall -Wextra
CXXFLAGS_RELEASE   = $(CXXFLAGS_COMMON) -O3
CXXFLAGS_RELEASE_2 = $(CXXFLAGS_RELEASE) --static -DNDEBUG
CXXFLAGS_DEBUG     = $(CXXFLAGS_COMMON) -O0 -g3 -fsanitize=address,undefined
CXXFLAGS_LINK      = 

MODE ?= debug
ifeq ($(MODE), fast)
	CXXFLAGS = $(CXXFLAGS_RELEASE_2)
else ifeq ($(MODE), release)
	CXXFLAGS = $(CXXFLAGS_RELEASE)
else
	CXXFLAGS = $(CXXFLAGS_DEBUG)
endif

SRC_DIR = .
UTL_DIR = utils
OBJ_DIR = obj
BIN_DIR = .

UTILS   = $(wildcard $(UTL_DIR)/*.c)
HEADERS = $(wildcard $(UTL_DIR)/*.h)
OBJECTS = $(patsubst $(UTL_DIR)/%.c,$(OBJ_DIR)/%.o,$(UTILS))

$(OBJ_DIR)/%.o: $(UTL_DIR)/%.c $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/main.o:
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/main.c -o $(OBJ_DIR)/main.o

karatsuba: $(OBJECTS) $(OBJ_DIR)/main.o
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/karatsuba.c -o $(OBJ_DIR)/karatsuba.o
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_LINK) $(OBJECTS) $(OBJ_DIR)/karatsuba.o $(OBJ_DIR)/main.o -o $(BIN_DIR)/karatsuba

naif: $(OBJECTS) $(OBJ_DIR)/main.o
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/naif.c -o $(OBJ_DIR)/naif.o
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_LINK) $(OBJECTS) $(OBJ_DIR)/naif.o $(OBJ_DIR)/main.o -o $(BIN_DIR)/naif

gen:
	$(CXX) $(CXXFLAGS) $(SRC_DIR)/gen.c -o $(BIN_DIR)/gen

.PHONY: clean
clean:
	rm -f $(OBJ_DIR)/*.o karatsuba naif

.PHONY: clean2
clean2:
	rm -f $(OBJ_DIR)/*.o karatsuba naif gen
