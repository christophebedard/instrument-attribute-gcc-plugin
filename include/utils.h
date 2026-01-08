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

#include <stdio.h>
#include <stdlib.h>

/// Dynamic strings list.
struct string_list {
  /// String array.
  char ** data;
  /// Number of strings in the array.
  size_t len;
  /// Actual length of the array, could be greater than len.
  size_t data_len;
};

/// Custom strdup.
/**
 * Not poisoned.
 */
char * strdup_(const char * str);

/// Count the number of occurrences of the given character in the given string.
size_t count(const char * str, const char * character);

/// Split string into list using given separator.
void split_str(char * str, const char * sep, struct string_list * list);

/// Free and reset list.
void free_list(struct string_list * list);

/// Check if a string in the list is a substring of the given string
/**
 * \param list the list
 * \param str1 the string
 * \return the match (const string) from the list, or NULL.
 */
const char * list_strstr(struct string_list * list, const char * str1);

/// Check if the given string exactly matches any string in the list
/**
 * \param list the list
 * \param str1 the string
 * \return the match (const string) from the list, or NULL.
 */
 const char * list_strcmp(struct string_list * list, const char * str1);
