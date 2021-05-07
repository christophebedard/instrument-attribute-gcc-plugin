// Copyright (C) 2019-2020 Christophe Bedard
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <stdio.h>

#include "gcc-plugin.h"

#include "langhooks.h"
#include "plugin-version.h"
#include "stringpool.h"
#include "tree.h"
#include "attribs.h"

int plugin_is_GPL_compatible;

#define ATTRIBUTE_NAME "instrument_function"
#define LIST_DELIMITER ","
#define DEBUG(...) \
  do { \
    if (is_debug) { \
      printf(__VA_ARGS__); \
    } \
  } while (0)
#define VERBOSE(...) \
  do { \
    if (is_verbose) { \
      printf(__VA_ARGS__); \
    } \
  } while (0)

bool is_debug = false;
bool is_verbose = false;

char * include_file_list = NULL;
char * include_function_list = NULL;

static struct plugin_info info = {
  "0.0.1",
  "This plugin provides the instrument_function attribute and"
    " other flags to enable instrumentation through allow-listing.",
};

static struct attribute_spec instrument_function_attr = {
  ATTRIBUTE_NAME,
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

bool should_instrument_function(tree fndecl)
{
  // If the function has our attribute, enable instrumentation
  if (lookup_attribute(ATTRIBUTE_NAME, DECL_ATTRIBUTES(fndecl)) != NULL_TREE)
  {
    return true;
  }

  // If the function's file is in the include-file-list, enable instrumentation
  if (include_file_list != NULL)
  {
    const char * function_file = DECL_SOURCE_FILE(fndecl);

    // Chekc if an element in the list is a substring of the function's file's path 
    char * list_element;
    char * rest = include_file_list;
    while (list_element = strtok_r(rest, LIST_DELIMITER, &rest))
    {
      if (strstr(function_file, list_element) != NULL)
      {
        VERBOSE("\tfunction instrumented from file list: %s (%s)\n", list_element, get_name(fndecl));
        return true;
      }
    }
  }

  // If the function is in the include-function-list, enable instrumentation
  if (include_function_list != NULL)
  {
    const char * function_name = lang_hooks.decl_printable_name (fndecl, 1);

    // Check if the function name is in the list
    char * list_element;
    char * rest = include_function_list;
    while (list_element = strtok_r(rest, LIST_DELIMITER, &rest))
    {
      if (strstr(function_name, list_element) != NULL)
      {
        VERBOSE("\tfunction instrumented from function name list: %s\n", list_element);
        return true;
      }
    }
  }

  return false;
}

void handle(void * event_data, void * data)
{
  tree fndecl = (tree) event_data;

  // Make sure it's a function
  if (TREE_CODE(fndecl) == FUNCTION_DECL)
  {
    // Check if the function should be instrumented
    if (should_instrument_function(fndecl))
    {
      DEBUG(
        "instrumented function: (%s:%d) %s\n",
        DECL_SOURCE_FILE(fndecl),
        DECL_SOURCE_LINE(fndecl),
        get_name(fndecl));
      DECL_NO_INSTRUMENT_FUNCTION_ENTRY_EXIT(fndecl) = 0;
    }
    // Otherwise explicitly disable it
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

  VERBOSE("Parameters:\n");
  for (int i = 0; i < argc; ++i)
  {
    // Check for debug argument, and enable debug mode if found
    if (strncmp(argv[i].key, "debug", 5) == 0)
    {
      is_debug = true;
    }
    // Check for file list
    else if (strncmp(argv[i].key, "include-file-list", 17) == 0)
    {
      include_file_list = argv[i].value;
      VERBOSE("\t%s: %s\n", argv[i].key, include_file_list);
    }
    // Check for function list
    else if (strncmp(argv[i].key, "include-function-list", 21) == 0)
    {
      include_function_list = argv[i].value;
      VERBOSE("\t%s: %s\n", argv[i].key, include_function_list);
    }
  }
}

void check_verbose()
{
  char * verbose_value = secure_getenv("VERBOSE");
  if (verbose_value != NULL && strncmp(verbose_value, "1", 1) == 0)
  {
    is_debug = true;
    is_verbose = true;
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
