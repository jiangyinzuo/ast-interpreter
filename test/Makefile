BUILD_DIR := ./build
SRCS := $(shell find */test*.c)

# String substitution for every C/C++ file.
# As an example, hello.cpp turns into ./build/hello.cpp.o
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

TARGET_EXEC := $(patsubst %.c,$(BUILD_DIR)/%,$(SRCS))
EXEC_OUTPUTS := $(patsubst %.c,$(BUILD_DIR)/%.output,$(SRCS))

all: $(EXEC_OUTPUTS)

%.output: %
	$< > $@

# The final build step.
$(TARGET_EXEC): $(OBJS) $(BUILD_DIR)/libtest.c.o
	gcc $@.c.o $(BUILD_DIR)/libtest.c.o -o $@ $(LDFLAGS)

$(BUILD_DIR)/libtest.c.o:
	gcc -c libtest.c -o $@

# Build step for C source
$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	gcc -c $< -o $@

clean:
	rm -rf build && mkdir build