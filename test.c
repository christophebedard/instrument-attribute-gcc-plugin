#include <iostream>

#define INSTRUMENT_FUNCTION __attribute__((instrument_function))

void INSTRUMENT_FUNCTION an_instrumented_function()
{
    std::cout << "this is instrumented" << std::endl;
}

void not_instrumented_function()
{
    std::cout << "this is not instrumented" << std::endl;
}

int INSTRUMENT_FUNCTION main()
{
    std::cout << "an_instrumented_function()" << std::endl;
    an_instrumented_function();

    std::cout << "not_instrumented_function()" << std::endl;
    not_instrumented_function();
    return 0;
}
