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

#include "gcc-plugin.h"

#include "langhooks.h"
#include "plugin-version.h"
#include "stringpool.h"
#include "tree.h"
#include "attribs.h"

int plugin_is_GPL_compatible;

const char * LIST_DELIMITER = ",";

#define ATTRIBUTE_NAME "instrument_function"
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

struct string_list {
  /// String array.
  char ** data;
  /// Number of strings in the array.
  size_t len;
  /// Actual length of the array, could be greater than len.
  size_t data_len;
};

struct string_list include_files = {};
struct string_list include_functions = {};

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

char * strdup_(const char * str)
{
  size_t len = strlen(str) + 1;
  char * dup_str = new char[len];
  memcpy(dup_str, str, len);
  return dup_str;
}

size_t count_characters(const char * str, const char * character)
{
  const char * p = str;
  size_t count = 0;

  do {
    if(*p == *character) count++;
  } while (*(p++));

  return count;
}

void split_str(char * str, const char * sep, struct string_list * list)
{
  // Could be wrong, e.g. "a,,b"
  size_t data_len = count_characters(str, sep) + 1;
  list->data = new char*[data_len];

  char * rest = NULL;
  char * token;
  size_t len = 0;
  for (token = strtok_r(str, sep, &rest); token != NULL; token = strtok_r(NULL, sep, &rest))
  {
    // Only use it if it's not empty
    if (strlen(token) > 0)
    {
      list->data[len] = strdup_(token);
      len++;
    }
  }

  list->len = len;
  list->data_len = data_len;
}

void print_list(struct string_list * list)
{
  printf("\t\tlist of size %ld: ", list->len);
  for (size_t i = 0; i < list->len; i++)
  {
    printf("%s, ", list->data[i]);
  }
  printf("\n");
}

char * list_strstr(struct string_list * list, const char * str1)
{
  for (size_t i = 0; i < list->len; i++)
  {
    if (strstr(str1, list->data[i]) != NULL)
    {
      return list->data[i];
    }
  }
  return NULL;
}

bool should_instrument_function(tree fndecl)
{
  // If the function has our attribute, enable instrumentation
  if (lookup_attribute(ATTRIBUTE_NAME, DECL_ATTRIBUTES(fndecl)) != NULL_TREE)
  {
    VERBOSE("\tfunction instrumented from attribute: %s\n", get_name(fndecl));
    return true;
  }

  // If the function's file is in the include-file-list, enable instrumentation
  if (include_files.len > 0)
  {
    const char * function_file = DECL_SOURCE_FILE(fndecl);

    // Check if an element in the list is a substring of the function's file's path
    DEBUG("\tchecking file: %s\n", function_file);
    char * result = list_strstr(&include_files, function_file);
    if (NULL != result)
    {
      VERBOSE("\t\tfunction instrumented from file list: %s (%s)\n", result, get_name(fndecl));
      return true;
    }
  }

  // If the function is in the include-function-list, enable instrumentation
  if (include_functions.len > 0)
  {
    const char * function_name = lang_hooks.decl_printable_name (fndecl, 1);

    // Check if the function name is in the list
    DEBUG("\tchecking function: %s\n", function_name);
    char * result = list_strstr(&include_functions, function_name);
    if (NULL != result)
    {
      VERBOSE("\t\tfunction instrumented from function name list: %s\n", result);
      return true;
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
      DEBUG(
        "NOT instrumented function: (%s:%d) %s\n",
        DECL_SOURCE_FILE(fndecl),
        DECL_SOURCE_LINE(fndecl),
        get_name(fndecl));
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
      char * include_file_list = argv[i].value;
      VERBOSE("\t%s: %s\n", argv[i].key, include_file_list);
      split_str(include_file_list, LIST_DELIMITER, &include_files);
      if (is_debug) {
        print_list(&include_files);
      }
    }
    // Check for function list
    else if (strncmp(argv[i].key, "include-function-list", 21) == 0)
    {
      char * include_function_list = argv[i].value;
      VERBOSE("\t%s: %s\n", argv[i].key, include_function_list);
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
  if (verbose_value != NULL && strncmp(verbose_value, "1", 1) == 0)
  {
    is_debug = true;
    is_verbose = true;
  }
}

void free_list(struct string_list * list)
{
  for (size_t i = 0; i < list->data_len; i++)
  {
    free(list->data[i]);
  }
  free(list->data);
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

  register_callback(
    plugin_info->base_name,
    PLUGIN_FINISH,
    plugin_fini,
    NULL);

  return 0;
}
