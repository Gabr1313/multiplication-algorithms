CXX = gcc
CXXFLAGS_COMMON  = -std=c17 -Wall -Wextra
CXXFLAGS_RELEASE = $(CXXFLAGS_COMMON) -O3 --static -DNDEBUG
CXXFLAGS_DEBUG   = $(CXXFLAGS_COMMON) -O0 -g3 -fsanitize=address,undefined
CXXFLAGS_LINK    = 

MODE ?= debug
ifeq ($(MODE), debug)
	CXXFLAGS = $(CXXFLAGS_DEBUG)
else 
	CXXFLAGS = $(CXXFLAGS_RELEASE)
endif

SRC_DIR = .
UTL_DIR = utils
OBJ_DIR = obj
BIN_DIR = .

UTILS   = $(wildcard $(UTL_DIR)/*.c)
HEADERS = $(wildcard $(UTL_DIR)/*.h)
OBJECTS = $(patsubst $(UTL_DIR)/%.c,$(OBJ_DIR)/%.o,$(UTILS))

$(warning $(UTILS))
$(warning $(HEADERS))
$(warning $(OBJECTS))

$(OBJ_DIR)/%.o: $(UTL_DIR)/%.c $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

karatsuba: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/karatsuba.c -o $(OBJ_DIR)/karatsuba.o
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_LINK) $(OBJECTS) $(OBJ_DIR)/karatsuba.o -o $(BIN_DIR)/karatsuba

.PHONY: clean
clean:
	rm -f $(OBJ_DIR)/*.o
