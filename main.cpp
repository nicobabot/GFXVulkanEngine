#include "HelloTriangleApp.h"

int main() 
{
    HelloTriangleApp app;
    
    try
    {
        app.Run();
    }
    catch (const std::exception& e)
    {
        std::cerr << RED_TEXT << e.what() << std::endl;
        std::cerr << RESET_TEXT << std::endl;

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}