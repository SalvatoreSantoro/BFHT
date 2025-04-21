### VARIABLES & FLAGS

NAME:=bfht
# useful: -Wpadded

ALPHA?=
PROBING?=
HT_SIZE_TYPE?=
HT_INITIAL_SIZE?=
HT_SIZE_MAX_GROWINGS?=

TUNING_FLAGS := \
  $(if $(ALPHA),-DALPHA=$(ALPHA)) \
  $(if $(PROBING),-DPROBING=$(PROBING)) \
  $(if $(HT_SIZE_TYPE),-DHT_SIZE_TYPE=$(HT_SIZE_TYPE)) \
  $(if $(HT_INITIAL_SIZE),-DHT_INITIAL_SIZE=$(HT_INITIAL_SIZE)) \
  $(if $(HT_SIZE_MAX_GROWINGS),-DHT_SIZE_MAX_GROWINGS=$(HT_SIZE_MAX_GROWINGS))

CC:=gcc
DEFAULT_FLAGS:=-std=c99 -O3 -Wall -Wextra -Werror -pedantic $(TUNING_FLAGS)
CFLAGS:=$(DEFAULT_FLAGS) -DNDDEBUG
TEST_CFLAGS:=$(CFLAGS) -D_POSIX_C_SOURCE=200112L #time.h MONOTONIC CLOCK

### DIRECTORIES
SRC_DIR := src
BUILD_DIR := build
TEST_DIR := test
LIB_NAME := $(NAME).a

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

TEST_SRCS := $(wildcard $(TEST_DIR)/*.c)
TEST_OBJS := $(patsubst $(TEST_DIR)/%.c, $(BUILD_DIR)/%.t.o, $(TEST_SRCS)) \
             $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.t.o, $(SRCS))


### BUILD
all: $(BUILD_DIR)/$(LIB_NAME)

$(BUILD_DIR)/$(LIB_NAME): $(OBJS)
	ar rcs $@ $^


$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@



$(BUILD_DIR)/%.d.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS_DEBUG) -c $< -o $@


### CREATE BUILD DIRECTORY IF NOT EXISTS
$(BUILD_DIR):
	@"mkdir" $(BUILD_DIR)

### TEST TARGET
test: build_tests
	@echo "Running tests..."
	$(BUILD_DIR)/test_binary -t

profiling: build_tests 
	@echo "Running profiling..."
	$(BUILD_DIR)/test_binary -p

build_tests: $(TEST_OBJS)
	$(CC) $(TEST_CFLAGS) $(TEST_OBJS) -o $(BUILD_DIR)/test_binary

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
	$(CC) (TEST_CFLAGS) -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer $(TEST_OBJS) -o $(BUILD_DIR)/test_binary
	@echo "Running tests..."
	$(BUILD_DIR)/test_binary

### CLEAN BUILD FILES
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
