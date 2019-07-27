#include <iostream>

#include "gcc-plugin.h"
#include "plugin-version.h"

int plugin_is_GPL_compatible;

static struct plugin_info info = {
    "0.0.1",
    "This plugin provides the instrument_function attribute.",
};

int plugin_init(
    struct plugin_name_args * plugin_info,
    struct plugin_gcc_version * version)
{
    std::cout << "Plugin: instrument_function attribute" << std::endl;

    register_callback(
        plugin_info->base_name,
        PLUGIN_INFO,
        NULL,
        &info);
    return 0;
}
