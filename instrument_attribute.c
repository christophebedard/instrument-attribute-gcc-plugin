#include <stdio.h>

#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree.h"

int plugin_is_GPL_compatible;

bool is_debug = false;
#define DEBUG(...) \
  if (is_debug) { \
    printf(__VA_ARGS__); \
  }

static struct plugin_info info = {
  "0.0.1",
  "This plugin provides the instrument_function attribute.",
};

static struct attribute_spec instrument_function_attr = {
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

  // Make sure it's a function
  if (TREE_CODE(fndecl) == FUNCTION_DECL)
  {
    // If the function has our attribute, enable instrumentation,
    // otherwise explicitly disable it
    if (lookup_attribute("instrument_function", DECL_ATTRIBUTES(fndecl)) != NULL_TREE)
    {
      DEBUG(
        "instrument_function: (%s:%d) %s\n",
        DECL_SOURCE_FILE(fndecl),
        DECL_SOURCE_LINE(fndecl),
        get_name(fndecl));
      DECL_NO_INSTRUMENT_FUNCTION_ENTRY_EXIT(fndecl) = 0;
    }
    else
    {
      DECL_NO_INSTRUMENT_FUNCTION_ENTRY_EXIT(fndecl) = 1;
    }
  }
}

void parse_plugin_args(struct plugin_name_args * plugin_info)
{
  int argc = plugin_info->argc;
  struct plugin_argument * argv = plugin_info->argv;

  for (int i = 0; i < argc; ++i)
  {
    // Check for debug argument, and enable debug mode if found
    if (strncmp(argv[i].key, "debug", 5) == 0)
    {
      is_debug = true;
    }
  }
}

void check_verbose()
{
  char * verbose_value = secure_getenv("VERBOSE");
  if (verbose_value != NULL && strncmp(verbose_value, "1", 1) == 0)
  {
    is_debug = true;
  }
}

int plugin_init(
  struct plugin_name_args * plugin_info,
  struct plugin_gcc_version * version)
{
  check_verbose();
  parse_plugin_args(plugin_info);

  DEBUG("Plugin: instrument_function attribute\n");

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

  register_callback(
    plugin_info->base_name,
    PLUGIN_FINISH_PARSE_FUNCTION,
    handle,
    NULL);
  return 0;
}
