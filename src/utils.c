// Copyright (C) 2021 Christophe Bedard
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

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"

char * strdup_(const char * str)
{
  size_t len = strlen(str) + 1;
  char * dup_str = new char[len];
  memcpy(dup_str, str, len);
  return dup_str;
}

size_t count(const char * str, const char * character)
{
  assert(1 == strlen(character));
  const char * p = str;
  size_t count = 0;

  do {
    if(*p == *character) count++;
  } while (*(p++));

  return count;
}

void split_str(char * str, const char * sep, struct string_list * list)
{
  assert(0 < strlen(sep));
  if (0 == strlen(str)) {
    list->len = 0u;
    list->data_len = 0u;
    return;
  }

  // Could be wrong, e.g. "a,,b"
  size_t data_len = count(str, sep) + 1;
  list->data = new char*[data_len];

  char * rest = NULL;
  char * token;
  size_t len = 0;
  for (token = strtok_r(str, sep, &rest); NULL != token; token = strtok_r(NULL, sep, &rest)) {
    // Only use it if it's not empty
    if (strlen(token) > 0) {
      list->data[len] = strdup_(token);
      len++;
    }
  }

  list->len = len;
  list->data_len = data_len;
}

void free_list(struct string_list * list)
{
  for (size_t i = 0; i < list->data_len; i++) {
    free(list->data[i]);
  }
  free(list->data);
  list->len = 0;
  list->data_len = 0;
}

const char * list_strstr(struct string_list * list, const char * str1)
{
  for (size_t i = 0; i < list->len; i++) {
    if (NULL != strstr(str1, list->data[i])) {
      return list->data[i];
    }
  }
  return NULL;
}
