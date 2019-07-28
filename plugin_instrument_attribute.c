#include <iostream>

#include "gcc-plugin.h"
#include "plugin-version.h"

#include "tree.h"
#include "diagnostic-core.h"

int plugin_is_GPL_compatible;

static struct plugin_info info = {
    "0.0.1",
    "This plugin provides the instrument_function attribute.",
};

tree handle_instrument_function_attribute(
    tree * node,
    tree name,
    tree args,
    int flags,
    bool * no_add_attrs)
{
    tree decl = *node;

    std::cout << "hande_instrument_function_attribute: " << std::endl
        << "\tname   : " << get_name(decl) << std::endl
        << "\taddr   : " << name << std::endl
        << "\tsource : " << DECL_SOURCE_FILE(decl) << ":" << DECL_SOURCE_LINE(decl) << std::endl;

    if (TREE_CODE(decl) != FUNCTION_DECL)
    {
        error_at(
            DECL_SOURCE_LOCATION(decl),
            "%qE attribute applies only to functions",
            name);
        *no_add_attrs = true;
    }
    else
    {
        DECL_NO_INSTRUMENT_FUNCTION_ENTRY_EXIT(decl) = 0;
    }

    return NULL_TREE;
}

static struct attribute_spec instrument_function_attr =
{
    "instrument_function",
    0,
    -1,
    false,
    false,
    false,
    &handle_instrument_function_attribute,
};

static void register_attributes(void * event_data, void * data)
{
    register_attribute(&instrument_function_attr);
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
        PLUGIN_ATTRIBUTES,
        register_attributes,
        NULL);
    return 0;
}
