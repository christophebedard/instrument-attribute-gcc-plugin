// Copyright (C) 2019-2021 Christophe Bedard
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
#include <assert.h>

#include "gcc-plugin.h"

#include "langhooks.h"
#include "plugin-version.h"
#include "stringpool.h"
#include "tree.h"
#include "attribs.h"

#include "utils.h"

int plugin_is_GPL_compatible;

const char * ATTRIBUTE_NAME = "instrument_function";
const char * PLUGIN_NAME = "instrument_function attribute";
const char * LIST_DELIMITER = ",";
const char * ARG_DEBUG = "debug";
const char * ARG_INCLUDE_FILE_LIST = "include-file-list";
const char * ARG_INCLUDE_FUNCTION_LIST = "include-function-list";

bool is_verbose = false;
bool is_debug = false;
#define VERBOSE(...) \
  do { \
    if (is_verbose) { \
      printf(__VA_ARGS__); \
    } \
  } while (0)
#define DEBUG(...) \
  do { \
    if (is_debug) { \
      printf(__VA_ARGS__); \
    } \
  } while (0)

struct string_list include_files = {};
struct string_list include_functions = {};

static struct plugin_info info = {
  "0.2.0",  // version
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

void print_list(struct string_list * list)
{
  printf("    list of size %ld: ", list->len);
  for (size_t i = 0; i < list->len; i++) {
    printf("%s, ", list->data[i]);
  }
  printf("\n");
}

bool should_instrument_function(tree fndecl)
{
  // If the function has our attribute, enable instrumentation
  if (NULL_TREE != lookup_attribute(ATTRIBUTE_NAME, DECL_ATTRIBUTES(fndecl))) {
    DEBUG("    function instrumented from attribute: %s\n", get_name(fndecl));
    return true;
  }

  // If the function's file is in the include-file-list, enable instrumentation
  if (include_files.len > 0) {
    const char * function_file = DECL_SOURCE_FILE(fndecl);

    // Check if an element in the list is a substring of the function's file's path
    DEBUG("    checking file: %s\n", function_file);
    const char * result = list_strstr(&include_files, function_file);
    if (NULL != result) {
      DEBUG("      function instrumented from file list: %s (%s)\n", result, get_name(fndecl));
      return true;
    }
  }

  // If the function is in the include-function-list, enable instrumentation
  if (include_functions.len > 0) {
    const char * function_name = lang_hooks.decl_printable_name (fndecl, 1);

    // Check if the function name is in the list
    DEBUG("    checking function: %s\n", function_name);
    const char * result = list_strstr(&include_functions, function_name);
    if (NULL != result) {
      DEBUG("      function instrumented from function name list: %s\n", result);
      return true;
    }
  }

  return false;
}

void handle(void * event_data, void * data)
{
  tree fndecl = (tree) event_data;

  // Make sure it's a function
  if (FUNCTION_DECL != TREE_CODE(fndecl)) {
    return;
  }

  DEBUG("DECL_SOURCE_FILE(fndecl)=%s\n", DECL_SOURCE_FILE(fndecl));
  DEBUG("event_data=%p\n", event_data);

  // Check if the function should be instrumented
  if (should_instrument_function(fndecl)) {
    VERBOSE(
      "  instrumented function: (%s:%d/%s) %s\n",
      DECL_SOURCE_FILE(fndecl),
      DECL_SOURCE_LINE(fndecl),
      DECL_SOURCE_FILE(DECL_ORIGIN(fndecl)),
      get_name(fndecl));
    DECL_NO_INSTRUMENT_FUNCTION_ENTRY_EXIT(fndecl) = 0;
  }
  // Otherwise explicitly disable it
  else {
    DEBUG(
      "  NOT instrumented function: (%s:%d) %s\n",
      DECL_SOURCE_FILE(fndecl),
      DECL_SOURCE_LINE(fndecl),
      get_name(fndecl));
    DECL_NO_INSTRUMENT_FUNCTION_ENTRY_EXIT(fndecl) = 1;
  }
}

void parse_plugin_args(struct plugin_name_args * plugin_info)
{
  const int argc = plugin_info->argc;
  struct plugin_argument * argv = plugin_info->argv;

  for (int i = 0; i < argc; ++i) {
    // Check for debug argument, and enable debug mode if found
    if (0 == strncmp(argv[i].key, ARG_DEBUG, strlen(ARG_DEBUG))) {
      is_debug = true;
      // Also enable verbose
      is_verbose = true;
    }
    // Check for file list
    else if (0 == strncmp(argv[i].key, ARG_INCLUDE_FILE_LIST, strlen(ARG_INCLUDE_FILE_LIST))) {
      char * include_file_list = argv[i].value;
      DEBUG("Plugin parameter:\n");
      DEBUG("  %s: %s\n", argv[i].key, include_file_list);
      split_str(include_file_list, LIST_DELIMITER, &include_files);
      if (is_debug) {
        print_list(&include_files);
      }
    }
    // Check for function list
    else if (0 == strncmp(argv[i].key, ARG_INCLUDE_FUNCTION_LIST, strlen(ARG_INCLUDE_FUNCTION_LIST))) {
      char * include_function_list = argv[i].value;
      DEBUG("Plugin parameter:\n");
      DEBUG("  %s: %s\n", argv[i].key, include_function_list);
      split_str(include_function_list, LIST_DELIMITER, &include_functions);
      if (is_debug) {
        print_list(&include_functions);
      }
    }
  }
}

void check_verbose()
{
  char * verbose_value = secure_getenv("VERBOSE");
  if (NULL != verbose_value && 0 == strncmp(verbose_value, "1", 1)) {
    is_verbose = true;
  }
}

void plugin_fini(void * gcc_data, void * user_data)
{
  free_list(&include_files);
  free_list(&include_functions);
}

int plugin_init(
  struct plugin_name_args * plugin_info,
  struct plugin_gcc_version * version)
{
  check_verbose();
  parse_plugin_args(plugin_info);

  VERBOSE("Plugin: %s\n", PLUGIN_NAME);

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

  // Headers files/function declarations
  register_callback(
    plugin_info->base_name,
    PLUGIN_FINISH_DECL,
    handle,
    NULL);
  // Source files/function definitions
  register_callback(
    plugin_info->base_name,
    PLUGIN_FINISH_PARSE_FUNCTION,
    handle,
    NULL);

  register_callback(
    plugin_info->base_name,
    PLUGIN_FINISH,
    plugin_fini,
    NULL);

  return 0;
}
