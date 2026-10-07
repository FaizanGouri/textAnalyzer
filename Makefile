CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -Iinclude
BUILD_DIR := build
ifeq ($(OS),Windows_NT)
EXE := .exe
else
EXE :=
endif
TARGET := $(BUILD_DIR)/text_analyzer$(EXE)
TEST_TARGET := $(BUILD_DIR)/test_analyzer$(EXE)
FREQUENCY_TEST_TARGET := $(BUILD_DIR)/test_frequency$(EXE)
SOURCES := $(wildcard src/*.c)

.PHONY: all run test clean

all: $(TARGET)

$(TARGET): $(SOURCES) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SOURCES) -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

run: $(TARGET)
	./$(TARGET)

test:
	$(CC) $(CFLAGS) tests/test_analyzer.c src/analyzer.c src/text_utils.c -o $(TEST_TARGET)
	./$(TEST_TARGET)
	$(CC) $(CFLAGS) tests/test_frequency.c src/frequency.c src/text_utils.c -o $(FREQUENCY_TEST_TARGET)
	./$(FREQUENCY_TEST_TARGET)

clean:
	rm -f $(TARGET) $(TEST_TARGET) $(FREQUENCY_TEST_TARGET)
