CXX=g++
TARGET_GCC=gcc
PLUGIN_SOURCE_FILES= instrument_attribute.c
TEST_SOURCE_FILES= test.c
GCCPLUGINS_DIR:= $(shell $(TARGET_GCC) -print-file-name=plugin)
CXXFLAGS+= -I$(GCCPLUGINS_DIR)/include -fPIC -O2

all: instrument_attribute.so

instrument_attribute.so: $(PLUGIN_SOURCE_FILES)
	$(CXX) -shared $(CXXFLAGS) $^ -o $@

clean:
	rm -f instrument_attribute.so
	rm -f test
	rm -rf test-trace/

verify: instrument_attribute.so
	$(CXX) $(CXXFLAGS) -fplugin=./instrument_attribute.so -c -x c++ /dev/null -o /dev/null

test: ${TEST_SOURCE_FILES} instrument_attribute.so
	gcc -fplugin=./instrument_attribute.so -finstrument-functions -fplugin-arg-instrument_attribute-debug $< -o $@

trace: test
	chmod +x trace.sh
	rm -rf test-trace/
	mkdir -p test-trace/
	./trace.sh
