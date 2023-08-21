#include <iostream>
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
