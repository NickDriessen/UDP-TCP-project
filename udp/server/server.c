#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "system_functions.h" // Include the header file

#define WAIT_TIMEOUT_SECONDS 120
#define INITIAL_TIMEOUT_SECONDS 8
#define EXTENDED_TIMEOUT_SECONDS 16
#define MAX_CLIENTS 100

typedef enum {
    INITIAL,
    AWAITING_GUESS,
    TIMEOUT,
	RESULT
} ServerState;

typedef struct ClientGuess {
	struct sockaddr_storage address;
	socklen_t address_length;
	int guess;
	int disqualified;
	struct ClientGuess *next;
} ClientGuess;

int main(int argc, char *argv[]) {
    // Initialization
    OSInit();
    int internet_socket = initialization();

	char buffer[1000];
	ServerState currentState = INITIAL;

	int randomNumber = 0;
	ClientGuess *clientListHead = NULL;
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

                timeout = INITIAL_TIMEOUT_SECONDS;
				firstGuessReceived = 0;

				currentState = AWAITING_GUESS;
				break;

			case AWAITING_GUESS:

				FD_ZERO(&readfds);
				FD_SET(internet_socket, &readfds);
			
				tv.tv_sec = firstGuessReceived ? timeout : WAIT_TIMEOUT_SECONDS;
				tv.tv_usec = 0;
			
				int selectResult = select(internet_socket + 1, &readfds, NULL, NULL, &tv);
			
				if (selectResult > 0) {
					if (FD_ISSET(internet_socket, &readfds)) {
						struct sockaddr_storage client_internet_address;
						memset(&client_internet_address, 0, sizeof(client_internet_address));
						socklen_t client_internet_address_length = sizeof(client_internet_address);
			
						listen_for_data(internet_socket, &client_internet_address, &client_internet_address_length, buffer, sizeof(buffer));
			
						int guess;
						if (sscanf(buffer, "%d", &guess) == 1) {
							printf("User guessed: %d\n", guess);
						} else {
							printf("Invalid guess\n");
							break;
						}
			
						int alreadyPlayed = 0;
						ClientGuess *current = clientListHead;
						while (current != NULL) {
							if (memcmp(&current->address, &client_internet_address, sizeof(struct sockaddr_storage)) == 0) {
								alreadyPlayed = 1;
								break;
							}
							current = current->next;
						}
			
						if (!alreadyPlayed) {
							ClientGuess *newClient = malloc(sizeof(ClientGuess));
							if (!newClient) {
								perror("Memory allocation failed");
								exit(1);
							}
							newClient->address = client_internet_address;
							newClient->address_length = client_internet_address_length;
							newClient->guess = guess;
							newClient->disqualified = 0;
							newClient->next = clientListHead;
							clientListHead = newClient;
						}
			
						if (!firstGuessReceived) {
							firstGuessReceived = 1;
							timeout = INITIAL_TIMEOUT_SECONDS;
						} else {
							timeout = INITIAL_TIMEOUT_SECONDS / 2;
						}
					}
				} else if (selectResult == 0) {
					if (!firstGuessReceived) {
						printf("No first guess received within 2 minutes. Shutting down server.\n");
						runGame = 0;
					} else {
						printf("Guessing timeout, moving to TIMEOUT phase\n");
						firstGuessReceived = 0;

						ClientGuess *potentialWinner = NULL;
						int closestDiff = 1000;

						ClientGuess *current = clientListHead;
						while (current != NULL) {
							int diff = abs(current->guess - randomNumber);
							if (diff < closestDiff) {
								closestDiff = diff;
								potentialWinner = current;
							}
							current = current->next;
						}

						if (potentialWinner != NULL) {
							send_response(internet_socket, &potentialWinner->address, potentialWinner->address_length, "You won?", strlen("You won?"));
						}

						currentState = TIMEOUT;
					}
				} else {
					perror("select error");
					runGame = 0;
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
							memset(&temp_client_address, 0, sizeof(temp_client_address));
							socklen_t temp_client_address_length = sizeof(temp_client_address);
							listen_for_data(internet_socket, &temp_client_address, &temp_client_address_length, buffer, sizeof(buffer));

							int knownClient = 0;
							ClientGuess *current = clientListHead;
							while (current != NULL) {
								if (memcmp(&current->address, &temp_client_address, sizeof(struct sockaddr_storage)) == 0) {
									knownClient = 1;
									break;
								}
								current = current->next;
							}

							if (!knownClient) {
								send_response(internet_socket, &temp_client_address, temp_client_address_length, "You lost!", strlen("You lost!"));
							} else
							{
								current->disqualified = 1;
							}
						}
					} else {
						break;
					}
				}
				currentState = RESULT;
				break;
			case RESULT:
				ClientGuess *winner = NULL;
				int closestDiff = 1000;
				
				ClientGuess *current = clientListHead;
				while (current != NULL) {
					if (!current->disqualified) { // Only consider non-disqualified clients
						int diff = abs(current->guess - randomNumber);
						if (diff < closestDiff) {
							closestDiff = diff;
							winner = current;
						}
					}
					current = current->next;
				}
				
				// Stuur reactie naar alle clients
				current = clientListHead;
				while (current != NULL) {
					if (current == winner) {
						send_response(internet_socket, &current->address, current->address_length, "You won!", strlen("You won!"));
					} else {
						send_response(internet_socket, &current->address, current->address_length, "You lost!", strlen("You lost!"));
					}

					ClientGuess *temp = current;
					current = current->next;
					free(temp);
				}
				clientListHead = NULL;

                currentState = INITIAL;
                break;
		}
	}

    // Clean up
    cleanup(internet_socket);
    OSCleanup();

    return 0;
}