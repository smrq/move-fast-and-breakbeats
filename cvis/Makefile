TARGET_EXEC ?= cvis

BUILD_DIR ?= build
SRC_DIRS ?= src

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s) lib/glad/src/glad.c
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

CXX := clang++
CPPFLAGS ?= -Isrc -Ilib/glad/include -Ilib/glm -Ilib/stb -MMD -MP
CXXFLAGS ?= -std=c++14
LDFLAGS ?= -lavcodec -lavformat -lavfilter -lavdevice -lswresample -lswscale -lavutil -lavresample -lfftw3 -lglfw -framework OpenGL

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: clean run

all: $(BUILD_DIR)/$(TARGET_EXEC)

clean:
	$(RM) -r $(BUILD_DIR)

run: $(BUILD_DIR)/$(TARGET_EXEC)
	./$(BUILD_DIR)/$(TARGET_EXEC) -i ../vis/public/media/tetrik.flac -o tetrik.mp4 -f ./media/endframe.png -e 452.684 -s 0.75

-include $(DEPS)

MKDIR_P ?= mkdir -p
