TARGET_EXEC ?= cvis

BUILD_DIR ?= build
SRC_DIRS ?= src

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s) lib/glad/src/glad.c
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

CXX := clang++
CPPFLAGS ?= -Ilib/glad/include -Ilib/glm -MMD -MP
CXXFLAGS ?= -std=c++11
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
	./$(BUILD_DIR)/$(TARGET_EXEC) ../vis/public/media/tetrik.flac tetrik.mp4

-include $(DEPS)

MKDIR_P ?= mkdir -p
