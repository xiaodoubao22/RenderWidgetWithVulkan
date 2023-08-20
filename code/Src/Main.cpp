#include <iostream>
#include <GLFW/glfw3.h>
#include "WindowImpl.h"

int main() {
    window::WindowImpl a;
    try {
        a.Exec();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
