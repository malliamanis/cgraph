NC     = \033[0m
BLUE   = \033[1;34m
CYAN   = \033[1;36m
GREEN  = \033[1;32m
YELLOW = \033[1;33m

CC = clang
LD = clang

CFLAGS =  -std=c11 -Wall -pedantic -Isrc
# CFLAGS += -Ideps/raylib/src 

CFLAGS_DEB = -O0 -g -gdwarf-4
CFLAGS_REL = -O3

# LDFLAGS = -Wl,-rpath,./deps/build/raylib/raylib -L./deps/build/raylib/raylib/ -lraylib -lm
LDFLAGS = -lSDL2 -lm

rwildcard = $(foreach d, $(wildcard $1*), $(call rwildcard, $d/, $2) $(filter $(subst *, %, $2), $d))

OBJ_DEB_DIR = build/debug/obj
OBJ_REL_DIR = build/release/obj
SRC         = $(call rwildcard, src, *.c)
OBJ_DEB     = $(patsubst src/%.c, $(OBJ_DEB_DIR)/%.o.d, $(SRC))
OBJ_REL     = $(patsubst src/%.c, $(OBJ_REL_DIR)/%.o,   $(SRC))

EXE_REL = build/release/cgraph
EXE_DEB = build/debug/cgraph

.PHONY: debug release run clean deps depsclean

debug: $(OBJ_DEB)
	@ echo -e "$(GREEN)LINKING EXECUTABLE$(NC) $(EXE_DEB)"
	@ $(LD) $(OBJ_DEB) -o $(EXE_DEB) $(LDFLAGS)

release: $(OBJ_REL)
	@ echo -e "$(GREEN)LINKING EXECUTABLE$(NC) $(EXE_REL)"
	@ $(LD) $(OBJ_REL) -o $(EXE_REL) $(LDFLAGS)

$(OBJ_REL_DIR)/%.o: src/%.c
	@ mkdir -p $(@D)
	@ echo -e "$(GREEN)COMPILING OBJECT$(NC) $@"
	@ $(CC) $(CFLAGS) $(CFLAGS_REL) -c $< -o $@

$(OBJ_DEB_DIR)/%.o.d: src/%.c
	@ mkdir -p $(@D)
	@ echo -e "$(GREEN)COMPILING OBJECT$(NC) $@"
	@ $(CC) $(CFLAGS) $(CFLAGS_DEB) -c $< -o $@

run: debug
	@ echo -e "$(CYAN)EXECUTING$(NC) $(EXE_DEB)"
	@ ./$(EXE_DEB)

clean:
	@ echo -e "$(YELLOW)CLEANING PROJECT$(NC)"
	@ rm -rf build

# deps:
# 	@ echo -e "$(BLUE)UPDATING SUBMODULES$(NC)"        && git submodule update --init --recursive --depth=1
# 	@ echo -e "$(BLUE)BUILDING DEPENDENCY$(NC) RAYLIB" && cd deps && mkdir -p build/raylib && cd build/raylib && \
# 	cmake ../../raylib -DBUILD_SHARED_LIBS=ON && cmake --build . --config Release && make -j4

# depsclean:
# 	@ echo -e "$(YELLOW)CLEANING DEPENDENCIES$(NC)"
# 	@ rm -rf deps/build

