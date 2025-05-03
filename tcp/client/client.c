#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "system_functions.h"

int main( int argc, char * argv[] )
{
	//////////////////
	//Initialization//
	//////////////////

	OSInit();

	int internet_socket = initialization();

    if(internet_socket != -1){
        char buffer[1000];
        char input[1000];

        /////////////
        //Execution//
        /////////////

        while(1)
        {
            memset(input, 0, sizeof(input));
            memset(buffer, 0, sizeof(buffer));

            printf("Enter your guess: ");
            char *ret = fgets(input, sizeof(input), stdin);
            if(ret == NULL) { 
                perror("fgets");
                continue;
            }

            input[strcspn(input, "\n")] = '\0';
            
            if(strcmp(input, "exit") == 0) {
                printf("Exiting\n");
                break;
            }

            char *endptr;
            uint32_t guess = strtol(input, &endptr, 10);
            if(endptr == input || *endptr != '\0') {
                printf("Invalid input");
                continue;
            }

            uint32_t network_byte_order = htonl(guess);
            send_message(internet_socket, (char *)&network_byte_order, sizeof(network_byte_order));        
            listen_for_data(internet_socket, buffer, sizeof(buffer));
        }

        ////////////
        //Clean up//
        ////////////

        cleanup( internet_socket );
    }

	OSCleanup();

	return 0;
}