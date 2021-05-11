CXX=g++
TARGET_GCC=gcc
PLUGIN_NAME= instrument_attribute
PLUGIN_SOURCE_FILES= $(PLUGIN_NAME).c
TEST_SOURCE_FILES= test/test.c test/other/other_file.h
GCCPLUGINS_DIR:= $(shell $(TARGET_GCC) -print-file-name=plugin)
CXXFLAGS+= -I$(GCCPLUGINS_DIR)/include -fPIC -O2

all: $(PLUGIN_NAME).so

$(PLUGIN_NAME).so: $(PLUGIN_SOURCE_FILES)
	$(CXX) -shared $(CXXFLAGS) $^ -o $@

clean:
	rm -f $(PLUGIN_NAME).so
	rm -f test/test
	rm -rf test-trace/

verify: $(PLUGIN_NAME).so
	$(CXX) $(CXXFLAGS) -fplugin=./$(PLUGIN_NAME).so -c -x c++ /dev/null -o /dev/null

test: ${TEST_SOURCE_FILES} $(PLUGIN_NAME).so
	VERBOSE=1 $(TARGET_GCC) -fplugin=./$(PLUGIN_NAME).so -finstrument-functions \
		-fplugin-arg-instrument_attribute-include-file-list=/something/else,test/other \
		-fplugin-arg-instrument_attribute-include-function-list=instrumented_with_function_list,random_other_function_name \
		$< -o test/$@

trace: test
	chmod +x trace.sh
	rm -rf test-trace/
	mkdir -p test-trace/
	./trace.sh
