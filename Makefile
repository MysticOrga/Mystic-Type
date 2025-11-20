##
## EPITECH PROJECT, 2025
## R-type
## File description:
## Makefile
##

CC			=	clang++
CPPFLAGS	=	-std=c++20 -Wall -Werror -Wextra
DEBUGFLAGS	=		-g -g3 -ggdb
RM			=	rm -rf
EXE			= 	server
BUILD		= 	./Dev
SRC 		= 	$(shell find src -type f -name '*.cpp')
SOCKET		=	$(shell find . -type s)
OBJ			=	$(SRC:.cpp=.o)

all: $(OBJ) build
	$(CC) $(CPPFLAGS) $(OBJ) -o $(EXE)
	@mv $(OBJ) $(BUILD)

debug:
	$(CC) $(SRC) $(CPPFLAGS) $(DEBUGFLAGS) -o $(EXE)

build:
	@mkdir -p $(BUILD)

clean:
	$(RM) $(BUILD)
	$(RM) $(SOCKET)
	$(RM) vgcore*

fclean: clean
	$(RM) $(EXE)

re: fclean all