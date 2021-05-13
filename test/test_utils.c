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

#define TEST_START() printf("%s\n", __FUNCTION__)

void test_strdup_()
{
  TEST_START();

  const char * str = "mystring";
  char * str2 = strdup_(str);
  assert(str2 != str);
  assert(0 == strcmp(str, str2));
  assert(NULL != str2);
  free(str2);
}

void test_count()
{
  TEST_START();

  assert(1u == count("abc", "a"));
  assert(3u == count("abaac", "a"));
  assert(0u == count("", "a"));
}

void test_string_list()
{
  TEST_START();

  const char * sep = ",";
  {
    char str[] = "some,random,list";
    struct string_list list = {};
    split_str(str, sep, &list);
    assert(3u == list.data_len);
    assert(3u == list.len);
    assert(0 == strcmp("some", list.data[0]));
    assert(0 == strcmp("random", list.data[1]));
    assert(0 == strcmp("list", list.data[2]));
    free_list(&list);
    assert(0u == list.data_len);
    assert(0u == list.len);
  }

  {
    char str[] = "some,random,list,";
    struct string_list list = {};
    split_str(str, sep, &list);
    assert(4u == list.data_len);
    assert(3u == list.len);
    assert(0 == strcmp("some", list.data[0]));
    assert(0 == strcmp("random", list.data[1]));
    free_list(&list);
    assert(0u == list.data_len);
    assert(0u == list.len);
  }

  {
    char str[] = "abc";
    struct string_list list = {};
    split_str(str, sep, &list);
    assert(1u == list.data_len);
    assert(1u == list.len);
    assert(0 == strcmp("abc", list.data[0]));
    free_list(&list);
    assert(0u == list.data_len);
    assert(0u == list.len);
  }

  {
    char str[] = "";
    struct string_list list = {};
    split_str(str, sep, &list);
    assert(0u == list.data_len);
    assert(0u == list.len);
    free_list(&list);
    assert(0u == list.data_len);
    assert(0u == list.len);
  }
}

void test_list_strstr()
{
  TEST_START();

  const char * sep = ",";
  {
    char str[] = "/path/to/,random_,mysuperfunction";
    const char * test1 = "/path/to/something";
    const char * test2 = "random_function";
    const char * test3 = "mysuperawesomefunction";
    struct string_list list = {};
    split_str(str, sep, &list);
    const char * match1 = list_strstr(&list, test1);
    assert(NULL != match1);
    assert(0 == strcmp(match1, "/path/to/"));
    const char * match2 = list_strstr(&list, test2);
    assert(NULL != match2);
    assert(0 == strcmp(match2, "random_"));
    const char * match3 = list_strstr(&list, test3);
    assert(NULL == match3);
    free_list(&list);
  }

  {
    char str[] = "";
    const char * test1 = "/path/to/something";
    const char * test2 = "";
    struct string_list list = {};
    split_str(str, sep, &list);
    const char * match1 = list_strstr(&list, test1);
    assert(NULL == match1);
    const char * match2 = list_strstr(&list, test2);
    assert(NULL == match2);
    free_list(&list);
  }
}

int main()
{
  test_strdup_();
  test_count();
  test_string_list();
  test_list_strstr();
  return 0;
}
