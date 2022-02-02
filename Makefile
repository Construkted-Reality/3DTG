CC = g++
INCLUDE = -I ./include
LIBF = -L ./lib
CXXFLAGS = -Werror -O1 --std=c++17 $(INCLUDE) $(LIBF)
# -DDEBUG -g
LFLAGS = 
LIBS = -lGLTF -ldraco -ldracoenc -ldracodec

OUT = out
APP = 3dtg



# Get Current platfrom name
ifeq ($(OS),Windows_NT)
	uname_S := Windows
else
	uname_S := $(shell uname -s)
endif

# Get output application extension (.exe for Windows, no extension for other systems)
ifeq ($(uname_S), Windows)
	target = $(APP).exe
endif
ifeq ($(uname_S), Linux)
	target = $(APP)
endif
ifeq ($(uname_S), Darwin)
	target = $(APP)
endif

# Create OUT dir if not exists
ifeq (,$(wildcard $(OUT)))
	createDirs := $(shell mkdir $("-p") $(OUT))
endif


EXECUTABLE = $(OUT)/$(target)

# Make does not offer a recursive wildcard function, so here's one:
rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

SRC = $(call rwildcard,src,*.cpp)
OBJ = $(SRC:.cpp=.o)
CLEAN =  $(foreach dir,$(OBJ),"$(dir)")

.PHONY: clean all app

all: app

app: $(OBJ)
	$(CC) $(CXXFLAGS) $(OBJ) -o $(EXECUTABLE) $(LFLAGS) $(LIBS)

clean:
	rm -f $(OBJ)
	rm -f $(EXECUTABLE)