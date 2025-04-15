### VARIABLES & FLAGS

NAME:=bfht
# useful: -Wpadded

CC:=gcc
DEFAULT_FLAGS:=-std=c99 -O2 -Wall -Wextra -Werror -pedantic
CFLAGS:=$(DEFAULT_FLAGS) -DNDDEBUG
CFLAGS_DEBUG:=$(DEFAULT_FLAGS) -g -DDEBUG
TEST_CFLAGS:=$(CFLAGS_DEBUG) -fno-plt -fno-pie
TEST_LDFLAGS:=-no-pie


### DIRECTORIES
SRC_DIR := src
BUILD_DIR := build
TEST_DIR := test
LIB_NAME := $(NAME).a
LIB_NAME_DEBUG := $(NAME)_debug.a

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))
OBJS_DEBUG := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.d.o, $(SRCS))

TEST_SRCS := $(wildcard $(TEST_DIR)/*.c)
TEST_OBJS := $(patsubst $(TEST_DIR)/%.c, $(BUILD_DIR)/%.t.o, $(TEST_SRCS)) \
             $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.t.o, $(SRCS))


### BUILD
all: $(BUILD_DIR)/$(LIB_NAME)

$(BUILD_DIR)/$(LIB_NAME): $(OBJS)
	ar rcs $@ $^


$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@


### DEBUG
debug: $(BUILD_DIR)/$(LIB_NAME_DEBUG)

$(BUILD_DIR)/$(LIB_NAME_DEBUG): $(OBJS_DEBUG)
	ar rcs $@ $^

$(BUILD_DIR)/%.d.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS_DEBUG) -c $< -o $@


### CREATE BUILD DIRECTORY IF NOT EXISTS
$(BUILD_DIR):
	@"mkdir" $(BUILD_DIR)

### TEST TARGET
test: $(TEST_OBJS)
	$(CC) $(TEST_CFLAGS) $(TEST_LDFLAGS) $(TEST_OBJS) -o $(BUILD_DIR)/test_binary
	@echo "Running tests..."
	$(BUILD_DIR)/test_binary

$(BUILD_DIR)/%.t.o: $(TEST_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(TEST_CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.t.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(TEST_CFLAGS) -c $< -o $@


### SANITIZERS
.PHONY: valgrind
valgrind: test
	valgrind --tool=memcheck --leak-check=full --track-origins=yes -s $(BUILD_DIR)/test_binary

.PHONY: asan
asan: $(TEST_OBJS)
	$(CC) $(TEST_CFLAGS) $(TEST_LDFLAGS) -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer $(TEST_OBJS) -o $(BUILD_DIR)/test_binary
	@echo "Running tests..."
	$(BUILD_DIR)/test_binary

### CLEAN BUILD FILES
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
