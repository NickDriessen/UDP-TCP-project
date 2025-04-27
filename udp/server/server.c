#include <stdio.h>
#include "system_functions.h" // Include the header file

int main(int argc, char *argv[]) {
    // Initialization
    OSInit();
    int internet_socket = initialization();

    // Execution
    execution(internet_socket);

    // Clean up
    cleanup(internet_socket);
    OSCleanup();

    return 0;
}