#include <iostream>

#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree.h"

int plugin_is_GPL_compatible;

static struct plugin_info info = {
    "0.0.1",
    "This plugin provides the instrument_function attribute.",
};

static struct attribute_spec instrument_function_attr =
{
    "instrument_function",
    0,
    -1,
    false,
    false,
    false,
    NULL,  // No need for a handling function
};

static void register_attributes(void * event_data, void * data)
{
    register_attribute(&instrument_function_attr);
}

void handle(void * event_data, void * data)
{
    tree fndecl = (tree) event_data;
    if (TREE_CODE(fndecl) == FUNCTION_DECL)
    {
        if (lookup_attribute("instrument_function", DECL_ATTRIBUTES(fndecl)) != NULL_TREE)
        {
            std::cout << "instrument_function: "
                << "(" << DECL_SOURCE_FILE(fndecl) << ":" << DECL_SOURCE_LINE(fndecl) << ") "
                << get_name(fndecl)
                << std::endl;
            DECL_NO_INSTRUMENT_FUNCTION_ENTRY_EXIT(fndecl) = 0;
        }
        else
        {
            DECL_NO_INSTRUMENT_FUNCTION_ENTRY_EXIT(fndecl) = 1;
        }
    }
}

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

    register_callback(
        plugin_info->base_name,
        PLUGIN_FINISH_PARSE_FUNCTION,
        handle,
        NULL);

    register_callback(
        plugin_info->base_name,
        PLUGIN_ATTRIBUTES,
        register_attributes,
        NULL);
    return 0;
}
