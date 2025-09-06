#include <iostream>
#include <thread>

#include "http_server.h"

int main(int argc, char *argv[]) {

    std::thread t(http_server_main_thread, argc, argv);
    t.join();  // Wait for the thread to finish
    std::cout << "Thread finished." << std::endl;

    return 0;
}
