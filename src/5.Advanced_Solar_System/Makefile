# files dir
# .
# ├── include
# │   ├── imgui
# │   │   └── src
# │   │       ├── imgui.cpp
# │   │       └── ...
# │   ├── GLFW
# │   ├── glad
# │   ├── glm
# │   ├── stb
# ├   └── assimp
# ├── lib
# ├── output
# │   ├── glfw3.dll
# │   └── main.exe
# └── src
#     ├── other directories
#     └── target_dir
# run 'make run dir=target_dir' at the root to compile and run the target project

# define the Cpp compiler to use
CXX          = g++

# define any compile-time flags
CXXFLAGS    := -std=c++17 -Wall -Wextra -g -MMD -MP

# define output directory
OUTPUT      := output

# define source directory
SRC	        := src/$(dir)
CLEAN_SRC   := $(SRC)/*.o

# define run task's arguments
ARGS        :=

# define include directory
INCLUDE     := include

# define lib directory
LIB         := lib

LIBRARIES   := -lglad -lglfw3dll -llibassimp -lOpengl32 -lglu32
MAIN        := main.exe
SOURCEDIRS  := $(SRC)
INCLUDEDIRS := $(INCLUDE)
LIBDIRS     := $(LIB)
FIXPATH     = $(subst //,/,$1)
RM          := rm -f

# define any directories containing header files other than /usr/include
INCLUDES    := $(patsubst %,-I%, $(INCLUDEDIRS:%/=%))

# define the C libs
LIBS        := $(patsubst %,-L%, $(LIBDIRS:%/=%))

# define the C source files
SOURCES     := $(wildcard $(patsubst %,%/*.cpp, $(SOURCEDIRS)))
IMGUI_SRCS  := $(wildcard $(INCLUDE)/imgui/src/*.cpp)

# define the object files
OBJECTS     := $(call FIXPATH,$(SOURCES:.cpp=.o))
IMGUI_OBJS  := $(call FIXPATH,$(IMGUI_SRCS:.cpp=.o))

# define the dependency files
DEPS        := $(call FIXPATH,$(OBJECTS:.o=.d))

# output/main.exe
OUTPUTMAIN  := $(call FIXPATH,$(OUTPUT)/$(MAIN))

.PHONY: all run clean
all: $(MAIN)
	@echo Executable generated at $(OUTPUTMAIN)

$(MAIN): $(OBJECTS) $(IMGUI_OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(OUTPUTMAIN) $(OBJECTS) $(IMGUI_OBJS) $(LFLAGS) $(LIBS) $(LIBRARIES)

%.o : %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCLUDES) -o $@ $<

clean:
	$(RM) $(OUTPUTMAIN)
	$(RM) $(call FIXPATH,$(CLEAN_SRC))
	$(RM) $(call FIXPATH,$(DEPS))
	@echo Cleanup complete!

run: all
	./$(OUTPUTMAIN) $(ARGS)
	@echo run complete!

# include the dependency files
-include $(DEPS)
