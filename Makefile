HOST_GCC=gcc
TARGET_GCC=gcc
PLUGIN_SOURCE_FILES= plugin_instrument_attribute.c
GCCPLUGINS_DIR:= $(shell $(TARGET_GCC) -print-file-name=plugin)
CXXFLAGS+= -I$(GCCPLUGINS_DIR)/include -fPIC -O2

all: plugin.so

plugin.so: $(PLUGIN_SOURCE_FILES)
	$(HOST_GCC) -shared $(CXXFLAGS) $^ -o $@

clean:
	rm plugin.so
