#ifdef _WIN32
	#define _WIN32_WINNT _WIN32_WINNT_WIN7
	#include <winsock2.h> //for all socket programming
	#include <ws2tcpip.h> //for getaddrinfo, inet_pton, inet_ntop
	#include <stdio.h> //for fprintf, perror
	#include <unistd.h> //for close
	#include <stdlib.h> //for exit
	#include <string.h> //for memset
	#include <stdint.h> //for int32_t
	#include <time.h> //for random number
	void OSInit( void )
	{
		WSADATA wsaData;
		int WSAError = WSAStartup( MAKEWORD( 2, 0 ), &wsaData ); 
		if( WSAError != 0 )
		{
			fprintf( stderr, "WSAStartup errno = %d\n", WSAError );
			exit( -1 );
		}
	}
	void OSCleanup( void )
	{
		WSACleanup();
	}
	#define perror(string) fprintf( stderr, string ": WSA errno = %d\n", WSAGetLastError() )
#else
	#include <sys/socket.h> //for sockaddr, socket, socket
	#include <sys/types.h> //for size_t
	#include <netdb.h> //for getaddrinfo
	#include <netinet/in.h> //for sockaddr_in
	#include <arpa/inet.h> //for htons, htonl, inet_pton, inet_ntop
	#include <errno.h> //for errno
	#include <stdio.h> //for fprintf, perror
	#include <unistd.h> //for close
	#include <stdlib.h> //for exit
	#include <string.h> //for memset
	#include <stdint.h> //for int32_t
	#include <time.h> //for random number
	void OSInit( void ) {}
	void OSCleanup( void ) {}
    #define SD_SEND SHUT_WR
#endif

int initialization();
//int connection( int internet_socket );
void con_exe( int internet_socket );
void cleanup( int internet_socket );

int main( int argc, char * argv[] )
{
	//////////////////
	//Initialization//
	//////////////////

	OSInit();

	int internet_socket = initialization();

	//////////////////////////
	//Connection & execution//
	//////////////////////////

	con_exe( internet_socket );


	////////////
	//Clean up//
	////////////

	cleanup( internet_socket );

	OSCleanup();

	return 0;
}

int initialization()
{
	//Step 1.1
	struct addrinfo internet_address_setup;
	struct addrinfo * internet_address_result;
	memset( &internet_address_setup, 0, sizeof internet_address_setup );
	internet_address_setup.ai_family = AF_INET; //gets ip4 only cuz other no worky :/
	internet_address_setup.ai_socktype = SOCK_STREAM;
	internet_address_setup.ai_flags = AI_PASSIVE;
	int getaddrinfo_return = getaddrinfo( NULL, "24042", &internet_address_setup, &internet_address_result );
	if( getaddrinfo_return != 0 )
	{
		fprintf( stderr, "getaddrinfo: %s\n", gai_strerror( getaddrinfo_return ) );
		exit( 1 );
	}

	int internet_socket = -1;
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
			int bind_return = bind( internet_socket, internet_address_result_iterator->ai_addr, internet_address_result_iterator->ai_addrlen );
			if( bind_return == -1 )
			{
				perror( "bind" );
				close( internet_socket );
			}
			else
			{
				//Step 1.4
				int listen_return = listen( internet_socket, 1 );
				if( listen_return == -1 )
				{
					close( internet_socket );
					perror( "listen" );
				}
				else
				{
					break;
				}
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

	return internet_socket;
}

void con_exe( int internet_socket )
{
	// File descriptor sets for select
	fd_set current_sockets, ready_socket;
	int fdmax = internet_socket;

	FD_ZERO(&current_sockets);
	FD_SET(internet_socket, &current_sockets); // Add server socket to set
 
	srand(time(NULL)); // Seed random number generator

	// Array to store secret number for each client socket
	long int secret_number[FD_SETSIZE];
	for (int j = 0; j <= FD_SETSIZE; j++) 
	{
		secret_number[j] = -1; // Initialize with invalid value
	}

	while (1)
	{	
		ready_socket = current_sockets; // Copy set for select()

		if (select(fdmax + 1, &ready_socket, NULL, NULL, NULL) < 0)
		{
			perror("select");
			exit( 4 );
		}

		// Loop over all possible descriptors
		for (int i = 0; i <= fdmax; i++)
		{
			if (FD_ISSET(i, &ready_socket)) // Check if descriptor is ready
			{
				if (i == internet_socket)
				{
					// New client connection
					struct sockaddr_in client_addr;
					socklen_t client_len = sizeof(client_addr);
					int new_fd = accept(internet_socket, (struct sockaddr *)&client_addr, &client_len);
					if (new_fd < 0) 
					{
						perror("accept");
						exit( 5 );
					}
					FD_SET(new_fd, &current_sockets); // Add client socket to set
					if (new_fd > fdmax)
					{
						fdmax = new_fd;
					}
				}
				else
				{
					// Existing client sent a guess
					int32_t client_guess_network;
					int number_of_bytes_sent;

					// Receive guess from client
					int number_of_bytes_received = recv(i, (char *)&client_guess_network, sizeof(client_guess_network), 0);
					if (number_of_bytes_received <= 0)
					{
						// Client disconnected or error
						close(i);
						FD_CLR(i, &current_sockets);
					}
					else
					{
						// Assign secret number if not already assigned
						if (secret_number[i] > 1000000 || secret_number[i] < 0)
						{
							secret_number[i] = ((rand() << 15) | rand()) % 1000001;
							printf("client %d number = %ld\n", i, secret_number[i]);
						}
						// Convert guess to host byte order
						int32_t client_guess_host = ntohl(client_guess_network);
						printf("client %d guessed: %d\n", i, client_guess_host);

						// Compare guess with secret number
						const char *response;
						if (client_guess_host < secret_number[i])
						{
							response = "Hoger\n"; // Guess is too low
						}
						else if (client_guess_host > secret_number[i])
						{
							response = "Lager\n"; // Guess is too high
						}
						else
						{
							response = "Correct\n"; // Guess is correct
							// Generate new number for this client
							secret_number[i] = ((rand() << 15) | rand()) % 1000001;
							printf("client %d number = %ld\n", i, secret_number[i]);
						}

						// Send result back to client
						number_of_bytes_sent = send(i, response, strlen(response), 0);
						if (number_of_bytes_sent == -1)
						{
							perror("send");
							exit( 7 );
						}
					}
				}	
			}
		}
	}
}

void cleanup( int internet_socket )
{
	close( internet_socket ); // Close listening socket
}