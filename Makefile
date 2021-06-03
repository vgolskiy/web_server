NAME = webserv

INCLUDE = ./includes/
HEADERS = 	Client.hpp \
			Config.hpp \
			Server.hpp \
			Socket.hpp \
			Request.hpp \
			Response.hpp \
#HEADERS = $(addprefix $(INCLUDE), $(HEADERS))

SRC_DIR = ./srcs/
SRCS = 	main.cpp \
		utils.cpp \
		Socket.cpp \
		Client.cpp \
		readConfig.cpp \
		Server.cpp \
		convertConfig.cpp \
		Request.cpp \
		Response.cpp \
		
#SRC = $(addprefix $(SRC_DIR), $(SRC))

OBJ_DIR = ./obj/
OBJECTS = $(patsubst %.cpp, $(OBJ_DIR)%.o, $(SRCS))

CL = clang++
FLAGS = -std=c++98 -g -Wall -Wextra -Werror

all: $(NAME)

$(NAME): $(OBJ_DIR) $(OBJECTS)
	@$(CL) $(OBJECTS) -o $@ -I $(INCLUDE)

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)
	#@mkdir -p $(OBJ_DIR)$(SRCS)

#$(OBJ_DIR)%.o : $(SRC_DIR)%.cpp $(HEADERS)
$(OBJ_DIR)%.o : %.cpp $(HEADERS)
	@$(CL) $(FLAGS) -c $< -o $@ -I $(INCLUDE)

clean:
	@rm -rf $(OBJ_DIR)

fclean: clean
	@rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re
