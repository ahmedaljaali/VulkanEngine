#include "Application.h"

// std
#include <cstdlib>
#include <exception>
#include <iostream>

int main(void)
{
    VE::Application app;

    try
    {
        app.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}
