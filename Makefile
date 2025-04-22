### VARIABLES & FLAGS

NAME:=bfht
# useful: -Wpadded

ALPHA?=
PROBING?=
HT_SIZE_TYPE?=
HT_INITIAL_SIZE?=
HT_SIZE_MAX_GROWINGS?=
PROF_OFILE?=

TUNING_FLAGS := \
  $(if $(ALPHA),-DALPHA=$(ALPHA)) \
  $(if $(PROBING),-DPROBING=$(PROBING)) \
  $(if $(HT_SIZE_TYPE),-DHT_SIZE_TYPE=$(HT_SIZE_TYPE)) \
  $(if $(HT_INITIAL_SIZE),-DHT_INITIAL_SIZE=$(HT_INITIAL_SIZE)) \
  $(if $(HT_SIZE_MAX_GROWINGS),-DHT_SIZE_MAX_GROWINGS=$(HT_SIZE_MAX_GROWINGS))

OUTFILE:=$(PROF_OFILE)

CC:=gcc
DEFAULT_FLAGS:=-std=c99 -O3 -Wall -Wextra -Werror -pedantic $(TUNING_FLAGS)
CFLAGS:=$(DEFAULT_FLAGS) -DNDDEBUG
TEST_CFLAGS:=$(CFLAGS) 
PROF_CFLAGS:=$(CFLAGS) -D_POSIX_C_SOURCE=200112L #time.h MONOTONIC CLOCK

### DIRECTORIES
SRC_DIR := src
BUILD_DIR := build
LIB_NAME := $(NAME).a
TEST_DIR := test
PROF_DIR := prof


SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

TEST_SRCS := $(wildcard $(TEST_DIR)/*.c)
TEST_OBJS := $(patsubst $(TEST_DIR)/%.c, $(BUILD_DIR)/%.t.o, $(TEST_SRCS)) \
             $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.t.o, $(SRCS))

PROF_SRCS := $(wildcard $(PROF_DIR)/*.c)
PROF_OBJS := $(patsubst $(PROF_DIR)/%.c, $(BUILD_DIR)/%.p.o, $(PROF_SRCS)) \
             $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.p.o, $(SRCS))


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
test: $(TEST_OBJS) 
	@echo "Running tests..."
	$(CC) $(TEST_CFLAGS) $(TEST_OBJS) -o $(BUILD_DIR)/test_binary
	$(BUILD_DIR)/test_binary

profiling: $(PROF_OBJS)
	@echo "Running profiling..."
	$(CC) $(PROF_CFLAGS) $(PROF_OBJS) -o $(BUILD_DIR)/prof_binary
	$(BUILD_DIR)/prof_binary $(OUTFILE)

#PROFILE OBJECTS

$(BUILD_DIR)/%.p.o: $(PROF_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(PROF_CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.p.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(PROF_CFLAGS) -c $< -o $@

#TEST OBJECTS

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
