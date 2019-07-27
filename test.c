#include <iostream>

void __attribute__((instrument_function)) an_instrumented_function()
{
    std::cout << "this is instrumented here" << std::endl;
}

void not_instrumented_function()
{
    std::cout << "this is not instrumented" << std::endl;
}

int main()
{
    std::cout << "Test." << std::endl;
    return 0;
}
