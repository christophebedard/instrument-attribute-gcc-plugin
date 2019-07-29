#include <stdio.h>

void __attribute__((instrument_function)) instrumented_function()
{
    printf("this is instrumented\n");
}

void NOT_instrumented_function()
{
    printf("this is NOT instrumented\n");
}

int __attribute__((instrument_function)) main()
{
    instrumented_function();
    NOT_instrumented_function();
    return 0;
}
