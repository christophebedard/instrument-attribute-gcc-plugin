#include <stdio.h>

#define INSTRUMENT_FUNCTION __attribute__((instrument_function))

void INSTRUMENT_FUNCTION an_instrumented_function()
{
    printf("this is instrumented\n");
}

void not_instrumented_function()
{
    printf("this is not instrumented");
}

int INSTRUMENT_FUNCTION main()
{
    printf("an_instrumented_function()");
    an_instrumented_function();

    printf("not_instrumented_function()");
    not_instrumented_function();
    return 0;
}
