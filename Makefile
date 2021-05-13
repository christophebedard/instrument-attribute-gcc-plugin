CXX=g++
TARGET_GCC=gcc
PLUGIN_NAME= instrument_attribute
PLUGIN_SOURCE_FILES= src/$(PLUGIN_NAME).c
PLUGIN_INCLUDE_DIR= include
UTILS_SOURCE_FILES= src/utils.c
UTILS_HEADER_FILES= $(PLUGIN_INCLUDE_DIR)/utils.h
TEST_UTILS_SOURCE_FILES= test/test_utils.c
TEST_SOURCE_FILES= test/test.c test/other/other_file.h
GCCPLUGINS_DIR:= $(shell $(TARGET_GCC) -print-file-name=plugin)
CXX_FLAGS_BASE= -fPIC -O2
CXXFLAGS+= -I$(GCCPLUGINS_DIR)/include $(CXX_FLAGS_BASE)

all: $(PLUGIN_NAME).so

utils.o: $(UTILS_SOURCE_FILES) $(UTILS_HEADER_FILES)
	$(CXX) $(CXX_FLAGS_BASE) -I$(PLUGIN_INCLUDE_DIR) $< -c -o $@

$(PLUGIN_NAME).so: $(PLUGIN_SOURCE_FILES) $(UTILS_HEADER_FILES) utils.o
	$(CXX) -shared $(CXXFLAGS) -I$(PLUGIN_INCLUDE_DIR) $^ -o $@

clean:
	rm -f utils.o $(PLUGIN_NAME).so
	rm -f test/test_utils test/test
	rm -rf test-trace/

verify: $(PLUGIN_NAME).so
	$(CXX) $(CXXFLAGS) -fplugin=./$(PLUGIN_NAME).so -c -x c++ /dev/null -o /dev/null

test_utils: $(TEST_UTILS_SOURCE_FILES) utils.o
	$(CXX) -I$(PLUGIN_INCLUDE_DIR) -o test/$@ $^
	./test/$@

test: $(TEST_SOURCE_FILES) $(PLUGIN_NAME).so
	VERBOSE=1 $(TARGET_GCC) -fplugin=./$(PLUGIN_NAME).so -finstrument-functions \
		-fplugin-arg-instrument_attribute-include-file-list=test/other/other_file3.h,test/other/other_file.h \
		-fplugin-arg-instrument_attribute-include-function-list=instrumented_with_function_list,random_other_function_name \
		$< -o test/$@

test_all: test_utils verify test

trace: test
	chmod +x trace.sh
	rm -rf test-trace/
	mkdir -p test-trace/
	./trace.sh
