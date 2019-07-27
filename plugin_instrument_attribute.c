#include <iostream>

#include "gcc-plugin.h"
#include "plugin-version.h"

int plugin_is_GPL_compatible;

int plugin_init(
    struct plugin_name_args * plugin_info,
    struct plugin_gcc_version * version)
{
    std::cout << "Plugin: instrument_function attribute" << std::endl;
    return 0;
}
