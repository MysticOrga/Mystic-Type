##
## EPITECH PROJECT, 2025
## Mystic-Type
## File description:
## Makefile
##

# ===========================
#      COMPILATION CONFIG
# ===========================

CC          = clang++
CPPFLAGS    = -std=c++20 -Wall -Wextra -Werror
DEBUGFLAGS  = -g3 -ggdb
RM          = rm -rf

BUILD       = ./Dev

# ===========================
#      SOURCES
# ===========================

SRC_COMMON  = $(shell find src -type f -name '*.cpp' \
                ! -name 'main.cpp' ! -name 'mainClient.cpp')

SRC_SERVER  = src/main.cpp

SRC_CLIENT  = src/mainClient.cpp

OBJ_COMMON  = $(SRC_COMMON:%.cpp=$(BUILD)/%.o)
OBJ_SERVER  = $(SRC_SERVER:%.cpp=$(BUILD)/%.o)
OBJ_CLIENT  = $(SRC_CLIENT:%.cpp=$(BUILD)/%.o)

EXE_SERVER  = server
EXE_CLIENT  = client

# ===========================
#         RULES
# ===========================

all: $(EXE_SERVER) $(EXE_CLIENT)

$(EXE_SERVER): $(OBJ_COMMON) $(OBJ_SERVER)
	$(CC) $(CPPFLAGS) $(OBJ_COMMON) $(OBJ_SERVER) -o $@

$(EXE_CLIENT): $(OBJ_COMMON) $(OBJ_CLIENT)
	$(CC) $(CPPFLAGS) $(OBJ_COMMON) $(OBJ_CLIENT) -o $@

$(BUILD)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) -c $< -o $@

# ===========================
#        DEBUG
# ===========================

debug: CPPFLAGS += $(DEBUGFLAGS)
debug: fclean all

# ===========================
#       CLEANING
# ===========================

clean:
	$(RM) $(BUILD)

fclean: clean
	$(RM) $(EXE_SERVER) $(EXE_CLIENT)

re: fclean all

.PHONY: all clean fclean re debug
