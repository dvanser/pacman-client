#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

char map[100][100];

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

char *get_command_name(char *string)
{
	char *res = (char*) malloc(256);
	int i;

	for (i=0; i<strlen(string); i++) {
		if (string[i] != ' ')
			res[i] = string[i];
		else {
			res[i] = '\0';
			return res;
		}
	}

	return "";
}

char *trim(char *string)
{
	char *res = (char*) malloc(strlen(string));
	int i;

	for (i=0; i<strlen(string); i++) {
		if (string[i] != '\0' && string[i] != '\n')
			res[i] = string[i];
		else {
			res[i] = '\0';
			return res;
		}
	}

	return res;
}

// void draw_map(int height, int width, )

int main(int argc, char *argv[])
{
    int tcp_socket, udp_socket, portno, n;
    struct sockaddr_in server_addr;
    struct hostent *server;

    //Connecting to server
    char buffer[256];
    if (argc < 3) {
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
        exit(0);
    }
    portno = atoi(argv[2]);
    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socket < 0)
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&server_addr.sin_addr.s_addr,
          server->h_length);
    server_addr.sin_port = htons(portno);
    if (connect(tcp_socket,(struct sockaddr *) &server_addr,sizeof(server_addr)) < 0)
        error("ERROR connecting");

    //Client try to join to the game
    char enumber;
	int player_id = -1;
    char joined = '0';
    while (joined == '0') {
	    //Read message from clients input
	    printf("Please enter the message: ");
	    bzero(buffer,256);
	    fgets(buffer,255,stdin);

	    char *nick = malloc(20);
	   
	    if (strcmp(get_command_name(buffer), "JOIN") == 0) {
	    	strncpy(nick, &buffer[5], 20);
	    	strncpy(nick, trim(nick), 20);

	    	char c = 0;
	    	n = write(tcp_socket, &c, 1); //send join message to server
	    	if (n < 0)
        		error("ERROR writing join message to socket");
	    	n = write(tcp_socket, nick, strlen(nick)); //send nickname to server
    		if (n < 0)
        		error("ERROR writing nick to socket");

	    	//get acknowledge from server
	    	bzero(buffer,14);
		    n = read(tcp_socket, &enumber, 1);
		    if (n >= 0 && enumber == 1) {
		    	n = read(tcp_socket, &player_id, 4);
		    	if (n < 0)
	        		error("ERROR reading player_id from socket");
	        	else {
		    		if (player_id == -1) {
		    			printf("%s\n", "Nick is already taken. Send new JOIN");
		    			continue; //go to start of loop
		    		}
		    		else if (player_id == -2) {
		    			printf("%s\n", "Server is full. Please try again later");
		    			continue;
		    		}
		    		else if (player_id == -3) {
		    			printf("%s\n", "Something is wrong");
		    			continue;
		    		}
		    	}
		    	joined = '1';
		    	printf("%s\n", "You are successfully joined to the game");
		    }
		    else {
		    	if (n < 0)
	        		error("ERROR reading acknowledge from socket");
	        	if (enumber != '1')
	        		error("ERROR joining to the game");
		    }
	    }
	}

    while (1) {
	    
	    //Read game begining message from server
	   	n = read(tcp_socket, &enumber, 1);
	   	//printf("%i\n", enumber);
	    if (n >= 0) {
	    	if (enumber == 2) {

	    		char map_height, map_width, player_x, player_y;
		    	if (read(tcp_socket, &map_height, 1) >= 0 
		    		&& read(tcp_socket, &map_width, 1) >= 0 
		    		&& read(tcp_socket, &player_x, 1) >= 0 
		    		&& read(tcp_socket, &player_y, 1) >= 0) {

		    	}
		    	else
	        		error("ERROR reading from socket");

	        	enumber = -1;
	        	
		    }

		    //END of the game
		    if (enumber == 3) {
		    	//If game ends, then go back and wait until we recieve START packet
		    	n = 0;
		    	enumber = -1;
		    	continue;
		    }

		    //MAP object
		    if (enumber == 4) {
		    	printf("%s\n", "asd");
		    	if (read(tcp_socket, &map, 10000) >= 0 ) {
		    		printf("%s\n", "I got MAP");
		    	}
		    	else {
		    		printf("%s\n", "I did not get MAP");
		    	}
		    }


	    }
	  
	}

	    
 //    if (n < 0)
 //        error("ERROR writing to socket");
 //    bzero(buffer,256);
 //    n = read(con_socket,buffer,255);
 //    if (n < 0)
 //        error("ERROR reading from socket");
 //    printf("%s\n",buffer);
    close(tcp_socket);
    close(udp_socket);

    return 0;
}