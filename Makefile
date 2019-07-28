CXX=g++
TARGET_GCC=gcc
PLUGIN_SOURCE_FILES= plugin_instrument_attribute.c
TEST_SOURCE_FILES= test.c
GCCPLUGINS_DIR:= $(shell $(TARGET_GCC) -print-file-name=plugin)
CXXFLAGS+= -I$(GCCPLUGINS_DIR)/include -fPIC -O2

all: plugin

plugin: plugin.so

plugin.so: $(PLUGIN_SOURCE_FILES)
	$(CXX) -shared $(CXXFLAGS) $^ -o $@

clean:
	rm -f plugin.so
	rm -f test
	rm -rf test-traces/

verify: plugin.so
	$(CXX) $(CXXFLAGS) -fplugin=./plugin.so -c -x c++ /dev/null -o /dev/null

test: ${TEST_SOURCE_FILES} plugin
	$(CXX) $(CXXFLAGS) -fplugin=./plugin.so $(CXXFLAGS) -finstrument-functions -g $< -o $@

trace: test
	chmod +x trace.sh
	rm -rf test-traces
	mkdir -p test-traces
	./trace.sh
