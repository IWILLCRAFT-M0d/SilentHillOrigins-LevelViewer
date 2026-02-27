CXX = g++
CXXFLAGS = -std=c++17 -Wall -g

SDL2_CFLAGS := $(shell pkg-config --cflags sdl2)
SDL2_LIBS   := $(shell pkg-config --libs sdl2)

INCLUDES = -I./src -I./vendor -I./vendor/imgui -I./vendor/imgui/backends -I./vendor/imguizmo $(SDL2_CFLAGS)

# Список усіх вихідних файлів
SRCS = src/main.cpp src/Common.cpp src/PS2Texture.cpp src/Loader.cpp src/UI.cpp \
       vendor/imgui/imgui.cpp \
       vendor/imgui/imgui_draw.cpp \
       vendor/imgui/imgui_tables.cpp \
       vendor/imgui/imgui_widgets.cpp \
       vendor/imgui/backends/imgui_impl_sdl2.cpp \
       vendor/imgui/backends/imgui_impl_opengl3.cpp \
       vendor/imguizmo/ImGuizmo.cpp

# Автоматично створюємо список .o файлів з .cpp файлів
OBJS = $(SRCS:.cpp=.o)

LIBS = -lGL -lGLEW $(SDL2_LIBS)

TARGET = SHOViewer

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
