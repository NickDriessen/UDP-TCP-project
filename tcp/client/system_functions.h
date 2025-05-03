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

int initialization();
void listen_for_data(int internet_socket, char *buffer, int buffer_size);
void send_message(int internet_socket, const char *message, int message_length);
void cleanup(int internet_socket);

void OSInit(void);
void OSCleanup(void);

#endif 