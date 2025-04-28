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
	int firstGuessReceived = 0;
	int runGame = 1;

    fd_set readfds;
    struct timeval tv;

	while(runGame)
	{
		switch(currentState){
			case INITIAL:
				srand(time(NULL));
				randomNumber = rand() % 100;	
				printf("Generated random number: %d\n", randomNumber);

				clientCount = 0;
				memset(clients, 0, sizeof(clients));

                timeout = INITIAL_TIMEOUT_SECONDS;
				firstGuessReceived = 0;

				currentState = AWAITING_GUESS;
				break;

			case AWAITING_GUESS:

				FD_ZERO(&readfds);
				FD_SET(internet_socket, &readfds);
			
				if (!firstGuessReceived) {
					// Wait up to 2 minutes for the first guess
					tv.tv_sec = 120; // 2 minutes
					tv.tv_usec = 0;
			
					int selectResult = select(internet_socket + 1, &readfds, NULL, NULL, &tv);
					if (selectResult > 0) {
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
			
							// After first guess received, start normal timeout
							firstGuessReceived = 1;
							timeout = INITIAL_TIMEOUT_SECONDS;
						}
					} else if (selectResult == 0) {
						// Timeout -> no first guess received
						printf("No first guess received within 2 minutes. Shutting down server.\n");
						// Exit main loop
						runGame = 0;
					} else {
						perror("select error");
						runGame = 0;
					}
				} else {
					// After first guess, regular timeout handling
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
			
							timeout = INITIAL_TIMEOUT_SECONDS / 2;
						}
					} else {
						printf("Guessing timeout, moving to TIMEOUT phase\n");
						firstGuessReceived = 0; // Reset for next round
						currentState = TIMEOUT;
					}
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