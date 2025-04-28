#include "system_functions.h" // Include the header file
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <errno.h>
#endif

#ifdef _WIN32
void OSInit(void) {
    WSADATA wsaData;
    int WSAError = WSAStartup(MAKEWORD(2, 0), &wsaData);
    if (WSAError != 0) {
        fprintf(stderr, "WSAStartup errno = %d\n", WSAError);
        exit(-1);
    }
}

void OSCleanup(void) {
    WSACleanup();
}

void perror(const char *string) {
    fprintf(stderr, "%s: WSA errno = %d\n", string, WSAGetLastError());
}
#else
void OSInit(void) {}
void OSCleanup(void) {}
#endif

int initialization() {
    // Step 1.1
    struct addrinfo internet_address_setup;
    struct addrinfo *internet_address_result;
    memset(&internet_address_setup, 0, sizeof internet_address_setup);
    internet_address_setup.ai_family = AF_INET6; // AF_INET6; //AF_UNSPEC;
    internet_address_setup.ai_socktype = SOCK_DGRAM;
    internet_address_setup.ai_flags = AI_PASSIVE;
    int getaddrinfo_return = getaddrinfo(NULL, "24042", &internet_address_setup, &internet_address_result);
    if (getaddrinfo_return != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getaddrinfo_return));
        exit(1);
    }

    int internet_socket = -1;
    struct addrinfo *internet_address_result_iterator = internet_address_result;
    while (internet_address_result_iterator != NULL) {
        // Step 1.2
        internet_socket = socket(internet_address_result_iterator->ai_family, internet_address_result_iterator->ai_socktype, internet_address_result_iterator->ai_protocol);
        if (internet_socket == -1) {
            perror("socket");
        } else {
            // Step 1.3
            int bind_return = bind(internet_socket, internet_address_result_iterator->ai_addr, internet_address_result_iterator->ai_addrlen);
            if (bind_return == -1) {
                close(internet_socket);
                perror("bind");
            } else {
                break;
            }
        }
        internet_address_result_iterator = internet_address_result_iterator->ai_next;
    }

    freeaddrinfo(internet_address_result);

    if (internet_socket == -1) {
        fprintf(stderr, "socket: no valid socket address found\n");
        exit(2);
    }

    return internet_socket;
}

void listen_for_data(int internet_socket, struct sockaddr_storage *client_internet_address, socklen_t *client_internet_address_length, char *buffer, int buffer_size) {
    int number_of_bytes_received = recvfrom(internet_socket, buffer, buffer_size - 1, 0, (struct sockaddr *) client_internet_address, client_internet_address_length);
    if (number_of_bytes_received == -1) {
        perror("recvfrom");
    } else {
        buffer[number_of_bytes_received] = '\0';
        printf("Received : %s\n", buffer);
    }
}

void send_response(int internet_socket, struct sockaddr_storage *client_internet_address, socklen_t client_internet_address_length, const char *response, int response_length) {
    int number_of_bytes_sent = sendto(internet_socket, response, response_length, 0, (struct sockaddr *) client_internet_address, client_internet_address_length);
    if (number_of_bytes_sent == -1) {
        perror("sendto");
    }
}

void cleanup(int internet_socket) {
    // Step 3.1
    close(internet_socket);
}
