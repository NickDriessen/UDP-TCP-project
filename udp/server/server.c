#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "system_functions.h" // Include the header file

#define INITIAL_TIMEOUT_SECONDS 8
#define EXTENDED_TIMEOUT_SECONDS 16
#define MAX_CLIENTS 100

typedef enum {
    INITIAL,
    AWAITING_GUESS,
    TIMEOUT,
	RESULT
} ServerState;

typedef struct {
	struct sockaddr_storage address;
	socklen_t address_length;
	int guess;
} ClientGuess;

int main(int argc, char *argv[]) {
    // Initialization
    OSInit();
    int internet_socket = initialization();

	char buffer[1000];
	ServerState currentState = INITIAL;

	int randomNumber = 0;
    ClientGuess clients[MAX_CLIENTS];
    int clientCount = 0;
    int timeout = INITIAL_TIMEOUT_SECONDS;

    // Declare once (to fix redefinition error)
    fd_set readfds;
    struct timeval tv;

	while(1)
	{
		switch(currentState){
			case INITIAL:
				srand(time(NULL));
				randomNumber = rand() % 100;	
				printf("Generated random number: %d\n", randomNumber);

				clientCount = 0;
                timeout = INITIAL_TIMEOUT_SECONDS;

				currentState = AWAITING_GUESS;
				break;

			case AWAITING_GUESS:
				FD_ZERO(&readfds);
				FD_SET(internet_socket, &readfds);

				tv.tv_sec = timeout;
				tv.tv_usec = 0;

				if (select(internet_socket + 1, &readfds, NULL, NULL, &tv) > 0) {
					if (FD_ISSET(internet_socket, &readfds)) {
						struct sockaddr_storage client_internet_address;
						socklen_t client_internet_address_length = sizeof(client_internet_address);

						listen_for_data(internet_socket, &client_internet_address, &client_internet_address_length, buffer, sizeof(buffer));

						int guess = atoi(buffer);
						printf("User guessed: %d\n", guess);

						int alreadyPlayed = 0;
						for (int i = 0; i < clientCount; i++) {
							if (memcmp(&clients[i].address, &client_internet_address, sizeof(struct sockaddr_storage)) == 0) {
								alreadyPlayed = 1;
								break;
							}
						}

						if (!alreadyPlayed && clientCount < MAX_CLIENTS) {
							clients[clientCount].address = client_internet_address;
							clients[clientCount].address_length = client_internet_address_length;
							clients[clientCount].guess = guess;
							clientCount++;
						}
						
						// Shorter timeout for next guess
						timeout = INITIAL_TIMEOUT_SECONDS / 2;
					}
				} else {
					// Timeout occurred
					printf("Initial guess phase timeout, moving to TIMEOUT phase\n");
					currentState = TIMEOUT;
				}
				break;
			case TIMEOUT:
				time_t timeoutStart = time(NULL);
				while (time(NULL) - timeoutStart < EXTENDED_TIMEOUT_SECONDS) {
					FD_ZERO(&readfds);
					FD_SET(internet_socket, &readfds);
					tv.tv_sec = EXTENDED_TIMEOUT_SECONDS - (time(NULL) - timeoutStart);
					tv.tv_usec = 0;

					if (select(internet_socket + 1, &readfds, NULL, NULL, &tv) > 0) {
						if (FD_ISSET(internet_socket, &readfds)) {
							struct sockaddr_storage temp_client_address;
							socklen_t temp_client_address_length = sizeof(temp_client_address);
							listen_for_data(internet_socket, &temp_client_address, &temp_client_address_length, buffer, sizeof(buffer));

							int knownClient = 0;
							for (int i = 0; i < clientCount; ++i) {
								if (memcmp(&clients[i].address, &temp_client_address, sizeof(struct sockaddr_storage)) == 0) {
									knownClient = 1;
									break;
								}
							}

							if (!knownClient) {
								send_response(internet_socket, &temp_client_address, temp_client_address_length, "You lost!", strlen("You lost!"));
							}
							// else: known client, do nothing
						}
					} else {
						break; // timeout, exit TIMEOUT phase
					}
				}
				currentState = RESULT;
				break;
			case RESULT:
				// Determine the closest guess
                int closestDiff = 1000;
                int winnerIndex = -1;
                for (int i = 0; i < clientCount; ++i) {
                    int diff = abs(clients[i].guess - randomNumber);
                    if (diff < closestDiff) {
                        closestDiff = diff;
                        winnerIndex = i;
                    }
                }

                // Send response to all clients
                for (int i = 0; i < clientCount; ++i) {
                    if (i == winnerIndex) {
                        send_response(internet_socket, &clients[i].address, clients[i].address_length, "You won!", strlen("You won!"));
                    } else {
                        send_response(internet_socket, &clients[i].address, clients[i].address_length, "You lost!", strlen("You lost!"));
                    }
                }

                currentState = INITIAL;
                break;
		}
	}

    // Clean up
    cleanup(internet_socket);
    OSCleanup();

    return 0;
}