#include <iostream>
#include "WindowImpl.h"

#include "Thread.h"

int main() {
    window::WindowImpl a(true);
    try {
        a.Exec();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
