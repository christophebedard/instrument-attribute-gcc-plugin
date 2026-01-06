CXX=g++
TARGET_GCC=gcc
PLUGIN_NAME= instrument_attribute
PLUGIN_SOURCE_FILES= src/$(PLUGIN_NAME).c
PLUGIN_INCLUDE_DIR= include
UTILS_SOURCE_FILES= src/utils.c
UTILS_HEADER_FILES= $(PLUGIN_INCLUDE_DIR)/utils.h
GCCPLUGINS_DIR:= $(shell $(TARGET_GCC) -print-file-name=plugin)
CXX_FLAGS_BASE= -fPIC -O2
CXXFLAGS+= -I$(GCCPLUGINS_DIR)/include $(CXX_FLAGS_BASE)

TEST_UTILS_DIR= test/utils
TEST_UTILS_SOURCE_FILES= $(TEST_UTILS_DIR)/test_utils.c

TEST_E2E_DIR= test/e2e
TEST_E2E_SRC_DIR= $(TEST_E2E_DIR)/src
TEST_E2E_OBJ_DIR= $(TEST_E2E_DIR)/obj
TEST_E2E_INCLUDE_DIR= $(TEST_E2E_DIR)/include
TEST_E2E_SRC= $(wildcard $(TEST_E2E_SRC_DIR)/*.c)
TEST_E2E_OBJ= $(TEST_E2E_SRC:$(TEST_E2E_SRC_DIR)/%.c=$(TEST_E2E_OBJ_DIR)/%.o)
TEST_E2E_CPPFLAGS= -I$(TEST_E2E_INCLUDE_DIR) -MMD -MP
TEST_E2E_TRACE_DIR= $(TEST_E2E_DIR)/trace

.PHONY: verify test test_utils test_all
all: $(PLUGIN_NAME).so

utils.o: $(UTILS_SOURCE_FILES) $(UTILS_HEADER_FILES)
	$(CXX) $(CXX_FLAGS_BASE) -I$(PLUGIN_INCLUDE_DIR) $< -c -o $@

$(PLUGIN_NAME).so: $(PLUGIN_SOURCE_FILES) $(UTILS_HEADER_FILES) utils.o
	$(CXX) -shared $(CXXFLAGS) -I$(PLUGIN_INCLUDE_DIR) $^ -o $@

verify: $(PLUGIN_NAME).so
	$(CXX) $(CXXFLAGS) -fplugin=./$(PLUGIN_NAME).so -c -x c++ /dev/null -o /dev/null

test_utils: test/test_utils
	./$<

test/test_utils: $(TEST_UTILS_SOURCE_FILES) utils.o
	$(CXX) -I$(PLUGIN_INCLUDE_DIR) -o $@ $^

test: test/test_end-to-end
	./test/run_end-to-end_tests.sh

test/test_end-to-end: $(TEST_E2E_OBJ) | $(PLUGIN_NAME).so $(TEST_E2E_OBJ_DIR) $(TEST_E2E_TRACE_DIR)
	$(TARGET_GCC) $^ -o $@

$(TEST_E2E_OBJ_DIR)/%.o: $(TEST_E2E_SRC_DIR)/%.c | $(TEST_E2E_OBJ_DIR) $(PLUGIN_NAME).so
	VERBOSE=1 $(TARGET_GCC) $(TEST_E2E_CPPFLAGS) \
		-fplugin=./$(PLUGIN_NAME).so -finstrument-functions \
		-fplugin-arg-instrument_attribute-debug \
		-fplugin-arg-instrument_attribute-include-file-list=test/e2e/src/some_,test/e2e/include/other/other_file.h \
		-fplugin-arg-instrument_attribute-include-function-list=instrumented_with_function_list,myawesomelib_,random_other_function_name \
		-fplugin-arg-instrument_attribute-include-exact-function-list=exact_match_function \
		-c $< -o $@
-include $(TEST_E2E_OBJ:.o=.d)

$(TEST_E2E_OBJ_DIR) $(TEST_E2E_TRACE_DIR):
	mkdir -p $@

test_all: verify test test_utils

trace: test
	chmod +x trace.sh
	rm -rf test-trace/
	mkdir -p test-trace/
	./trace.sh

clean:
	rm -f utils.o $(PLUGIN_NAME).so
	rm -rf test/test_utils test/test_end-to-end $(TEST_E2E_OBJ_DIR)
	rm -rf test-trace/ $(TEST_E2E_TRACE_DIR)
