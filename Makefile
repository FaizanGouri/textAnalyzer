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
SEARCH_TEST_TARGET := $(BUILD_DIR)/test_search$(EXE)
FILE_HANDLER_TEST_TARGET := $(BUILD_DIR)/test_file_handler$(EXE)
REPORT_TEST_TARGET := $(BUILD_DIR)/test_report$(EXE)
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
	$(CC) $(CFLAGS) tests/test_search.c src/search.c src/text_utils.c -o $(SEARCH_TEST_TARGET)
	./$(SEARCH_TEST_TARGET)
	$(CC) $(CFLAGS) tests/test_file_handler.c src/file_handler.c src/text_utils.c -o $(FILE_HANDLER_TEST_TARGET)
	./$(FILE_HANDLER_TEST_TARGET)
	$(CC) $(CFLAGS) tests/test_report.c src/report.c src/analyzer.c src/frequency.c src/text_utils.c -o $(REPORT_TEST_TARGET)
	./$(REPORT_TEST_TARGET)

clean:
	rm -f $(TARGET) $(TEST_TARGET) $(FREQUENCY_TEST_TARGET) $(SEARCH_TEST_TARGET) $(FILE_HANDLER_TEST_TARGET) $(REPORT_TEST_TARGET)
