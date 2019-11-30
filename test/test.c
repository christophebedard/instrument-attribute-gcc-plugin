#include <stdio.h>
#include "other/other_file.h"

void __attribute__((instrument_function)) instrumented_function()
{
  printf("this is instrumented\n");
}

void NOT_instrumented_function()
{
  printf("this is NOT instrumented\n");
}

void instrumented_with_function_list()
{
  printf("this can be instrumented via include-function-list\n");
}

int __attribute__((instrument_function)) main()
{
  instrumented_function();
  NOT_instrumented_function();
  instrumented_with_function_list();
  instrumented_with_file_list();
  return 0;
}
