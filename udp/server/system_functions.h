#ifndef SYSTEM_FUNCTIONS_H
#define SYSTEM_FUNCTIONS_H

#ifdef _WIN32
    #ifndef _WIN32_WINNT
    #define _WIN32_WINNT _WIN32_WINNT_WIN7
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/socket.h>
    #include <netdb.h>
    #include <netinet/in.h>
#endif

// Function declarations
int initialization();
void listen_for_data(int internet_socket, struct sockaddr_storage *client_internet_address, socklen_t *client_internet_address_length, char *buffer, int buffer_size);
void send_response(int internet_socket, struct sockaddr_storage *client_internet_address, socklen_t client_internet_address_length, const char *response, int response_length);
void cleanup(int internet_socket);

// OS specific initialization and cleanup
void OSInit(void);
void OSCleanup(void);

#endif // SYSTEM_FUNCTIONS_H