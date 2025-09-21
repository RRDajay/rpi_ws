#include <iostream>
#include <thread>

#include "http_server.h"
#include "http_client.h"

int main(int argc, char *argv[]) {

    std::thread t1(http_server_main_thread, argc, argv);
    std::thread t2(http_client_main_thread, argc, argv);

    t1.join();  // Wait for the thread to finish
    t2.join();  // Wait for the thread to finish

    std::cout << "Thread finished." << std::endl;

    return 0;
}
