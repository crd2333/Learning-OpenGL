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
# │   └── assimp
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

# 之前 makefile 的备份
# #
# # 'make'        build executable file 'main'
# # 'make clean'  removes all .o and executable files
# #

# # define the Cpp compiler to use
# CXX          = g++

# # define any compile-time flags
# CXXFLAGS    := -std=c++17 -Wall -Wextra -g -MMD -MP

# # define library paths in addition to /usr/lib
# #   if I wanted to include libraries not in /usr/lib I'd specify their path using -Lpath, something like:
# LFLAGS       =

# # define output directory
# OUTPUT      := output

# # define source directory 运行时修改此处路径
# SRC	        := src/$(dir)      # 传递 var 变量定义执行文件目录
# CLEAN_SRC   := src/$(dir)/*.o  # 删除所有.o文件

# # define run task's arguments
# ARGS        :=

# # define include directory
# INCLUDE     := include

# # define lib directory
# LIB         := lib

# ifeq ($(OS),Windows_NT)
# LIBRARIES   := -lglad -lglfw3dll -llibassimp -lOpengl32 -lglu32
# MAIN        := main.exe
# SOURCEDIRS  := $(SRC)
# INCLUDEDIRS := $(INCLUDE)
# LIBDIRS     := $(LIB)
# FIXPATH      = $(subst //,/,$1)
# RM          := rm -f   # powershell olso use rm
# MD          := mkdir
# else
# LIBRARIES   := -lglad -lglfw
# # LIBRARIES   := -lglad -lglfw -ldl -lpthread
# MAIN        := main
# SOURCEDIRS  := $(shell find $(SRC) -type d)
# INCLUDEDIRS := $(shell find $(INCLUDE) -type d)
# LIBDIRS     := $(shell find $(LIB) -type d)
# FIXPATH      = $1
# RM           = rm -f
# MD          := mkdir -p
# endif

# # define any directories containing header files other than /usr/include
# INCLUDES    := $(patsubst %,-I%, $(INCLUDEDIRS:%/=%))

# # define the C libs
# LIBS        := $(patsubst %,-L%, $(LIBDIRS:%/=%))

# # define the C source files
# SOURCES     := $(wildcard $(patsubst %,%/*.cpp, $(SOURCEDIRS)))
# # add source files for imgui
# # IMGUI_SRC := $(INCLUDE)/imgui/src
# # SOURCES += $(IMGUI_SRC)/imgui_impl_glfw.cpp      \
# #            $(IMGUI_SRC)/imgui_impl_opengl3.cpp   \
# #            $(IMGUI_SRC)/imgui.cpp                \
# #            $(IMGUI_SRC)/imgui_draw.cpp           \
# #            $(IMGUI_SRC)/imgui_widgets.cpp        \
# #            $(IMGUI_SRC)/imgui_demo.cpp           \
# #            $(IMGUI_SRC)/imgui_tables.cpp    # the latter two are optional
# IMGUI_SRCS := $(wildcard $(INCLUDE)/imgui/src/*.cpp)

# # define the object files
# OBJECTS     := $(call FIXPATH,$(SOURCES:.cpp=.o))
# IMGUI_OBJS  := $(call FIXPATH,$(IMGUI_SRCS:.cpp=.o))

# # define the dependency files
# DEPS        := $(call FIXPATH,$(OBJECTS:.o=.d))

# # output/main.exe
# OUTPUTMAIN  := $(call FIXPATH,$(OUTPUT)/$(MAIN))

# .PHONY: all run clean
# all: $(OUTPUT) $(MAIN) # 需要有输出目录和可执行文件
# 	@echo Executable generated at $(OUTPUTMAIN)

# $(OUTPUT):
# 	$(MD) $(OUTPUT)

# # $(MAIN): $(OBJECTS) $(IMGUI_OBJS)
# # 	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(OUTPUTMAIN) $(OBJECTS) $(IMGUI_OBJS) $(LFLAGS) $(LIBS) $(LIBRARIES)
# # 暂时用不到 imgui
# $(MAIN): $(OBJECTS)
# 	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(OUTPUTMAIN) $(OBJECTS) $(LFLAGS) $(LIBS) $(LIBRARIES)

# %.o : %.cpp
# 	$(CXX) -c $(CXXFLAGS) $(INCLUDES) -o $@ $<

# clean:
# 	$(RM) $(OUTPUTMAIN)
# 	$(RM) $(call FIXPATH,$(CLEAN_SRC))
# 	$(RM) $(call FIXPATH,$(DEPS))
# 	@echo Cleanup complete!

# run: all   # 此处./src/$(dir) 传递 main 函数 argv 的参数
# 	./$(OUTPUTMAIN) $(ARGS)
# 	@echo run complete!

# # include the dependency files
# -include $(DEPS)