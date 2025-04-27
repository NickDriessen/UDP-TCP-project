#ifndef SYSTEM_FUNCTIONS_H
#define SYSTEM_FUNCTIONS_H

#ifdef _WIN32
    #define _WIN32_WINNT _WIN32_WINNT_WIN7
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/socket.h>
    #include <netdb.h>
    #include <netinet/in.h>
#endif

// Function declarations
int initialization();
void execution(int internet_socket);
void cleanup(int internet_socket);

// OS specific initialization and cleanup
void OSInit(void);
void OSCleanup(void);

#endif // SYSTEM_FUNCTIONS_H