#include "system_functions.h" 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef _WIN32
#include <windows.h>
#else
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

#define SD_SEND SHUT_WR
#endif

int initialization() {
    //Step 1.1
	struct addrinfo internet_address_setup;
	struct addrinfo * internet_address_result;
	memset( &internet_address_setup, 0, sizeof internet_address_setup );
	internet_address_setup.ai_family = AF_INET;
	internet_address_setup.ai_socktype = SOCK_STREAM;
	int getaddrinfo_return = getaddrinfo( "127.0.0.1", "24042", &internet_address_setup, &internet_address_result );
	if( getaddrinfo_return != 0 )
	{
		fprintf( stderr, "getaddrinfo: %s\n", gai_strerror( getaddrinfo_return ) );
		exit( 1 );
	}

	int internet_socket = -1;
	int connection = 1;
	struct addrinfo * internet_address_result_iterator = internet_address_result;
	while( internet_address_result_iterator != NULL )
	{
		//Step 1.2
		internet_socket = socket( internet_address_result_iterator->ai_family, internet_address_result_iterator->ai_socktype, internet_address_result_iterator->ai_protocol );
		if( internet_socket == -1 )
		{
			perror( "socket" );
		}
		else
		{
			//Step 1.3
			int connect_return = connect( internet_socket, internet_address_result_iterator->ai_addr, internet_address_result_iterator->ai_addrlen );
			if( connect_return == -1 )
			{
				perror( "connect" );
				close( internet_socket );
				connection = -1;
			}
			else
			{
				break;
			}
		}
		internet_address_result_iterator = internet_address_result_iterator->ai_next;
	}

	freeaddrinfo( internet_address_result );

	if( internet_socket == -1 )
	{
		fprintf( stderr, "socket: no valid socket address found\n" );
		exit( 2 );
	}

	if(internet_socket != -1 && connection != -1) return internet_socket;
	else return connection;
}

void listen_for_data(int internet_socket, char *buffer, int buffer_size) {
    //Step 2.2
	int number_of_bytes_received = 0;
	number_of_bytes_received = recv( internet_socket, buffer, buffer_size - 1, 0 );
	if( number_of_bytes_received == -1 )
	{
		perror( "recv" );
	}
    else if( number_of_bytes_received == 0)
    {
        perror("server closed");
    }
	else
	{
		buffer[number_of_bytes_received] = '\0';
		printf( "Received : %s\n", buffer );
	}
}

void send_message(int internet_socket, const char *message, int message_length) {
     //Step 2.1
	int number_of_bytes_send = send( internet_socket, message, message_length, 0 );
	if( number_of_bytes_send == -1 )
	{
		perror( "send" );
	}
}

void cleanup( int internet_socket )
{
	//Step 3.2
	int shutdown_return = shutdown( internet_socket, SD_SEND );
	if( shutdown_return == -1 )
	{
		perror( "shutdown" );
	}

	//Step 3.1
	close( internet_socket );
}
