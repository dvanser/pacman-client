#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ncurses.h>
#include "list.h"
#include "enums.h"

const int MAX_PLAYERS = 10;


char map[100][100] = {{1,1,1,1,1,1}, {1,2,3,1,1,1}, {1,0,0,0,0,1}, {1,5,5,4,2,1}, {1,1,1,1,1,1}};
char map_height, map_width;
int players_count = 0;
Players_t players;

void init_screen() {
	initscr(); // Initialize the window
	noecho(); // Don't echo any keypresses
	curs_set(FALSE); // Don't display a cursor
	start_color();
	cbreak();
    nodelay(stdscr, TRUE);
    scrollok(stdscr, TRUE);
}

void destroy_screen() {
	refresh();
	sleep(3);
	endwin(); // Restore normal terminal behavior
} 

void draw_map() {

	int x, y;
	for (x=0; x<map_height; x++) {
		for (y=0; y<map_width; y++) {

			switch(map[x][y]) {
				case None:
					mvprintw(x, y, " ");
					break;
				case Dot:
					mvprintw(x, y, ".");
					break;
				case Wall:
					mvprintw(x, y, "#");
					break;
				case PowerPellet:
					mvprintw(x, y, "$");
					break;
				case Invincibility:
					mvprintw(x, y, "i");
					break;
				case Score:
					mvprintw(x, y, "+");
					break;
				
			}
				
		}
	}
	
}

void refresh_map() {

	draw_map();

	Node_t* player = (Node_t*) malloc(sizeof(Node_t));
    player = players.head;

    init_pair(1, COLOR_RED, COLOR_BLACK); //color for clients pacman or spoke
    init_pair(2, COLOR_WHITE, COLOR_BLACK); //color for other clients pacmans or spokes

    while (player != NULL) {

    	if (player == players.head)
    		attron(COLOR_PAIR(1));
		else
			attron(COLOR_PAIR(2));

    	if (player->data.type == Pacman) {
    		switch (player->data.state) {
    			case Normal:
					mvprintw(player->data.y, player->data.x, "p");
					break;
				case Dead:
					//mvprintw(player->data.y, player->data.x, " ");
					break;
				case Power:
					mvprintw(player->data.y, player->data.x, "P");
					break;
				case Invincibil:
					mvprintw(player->data.y, player->data.x, " ");
					break;
	   		}
    	}
    	else {
    		switch (player->data.state) {
	    		case Normal:
					mvprintw(player->data.y, player->data.x, "s");
					break;
				case Dead:
					//mvprintw(player->data.x, player->data.y, " ");
					break;
				case Power:
					mvprintw(player->data.y, player->data.x, "S");
					break;
				case Invincibil:
					mvprintw(player->data.y, player->data.x, " ");
					break;
			}
    	}

    	player = player->next;
    }

    attron(COLOR_PAIR(2));
	
}

int key_pressed()
{
    int ch = getch();

    if (ch != ERR) {
        ungetch(ch);
        return 1;
    } else {
        return 0;
    }
}

int make_move() {
	//TODO: check if wall
	int ch = getch();
	if (ch == '\033') { // if the first value is esc
	    getch(); // skip the [
	    switch(getch()) { // the real value
	        case 'A':
	            // code for arrow up
	        	if (players.head->data.y > 0)
	        		players.head->data.y--;
	        	return Up;
	            break;
	        case 'B':
	            // code for arrow down
		        if (players.head->data.y < map_height - 1)
		        	players.head->data.y++;
		        return Down;
	            break;
	        case 'C':
	            // code for arrow right
	        	if (players.head->data.x < map_width - 1)
	        		players.head->data.x++;
	        	return Right;
	            break;
	        case 'D':
	            // code for arrow left
	        	if (players.head->data.x > 0)
	        		players.head->data.x--;
	        	return Left;
	            break;
	    }
	}
	else {
		if (ch == 'q')
			return 4;
	}
}


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

//Serach player in players array by id. If not found return -1
Node_t* find_player(int id) {

	Node_t* curr_node = (Node_t*) malloc(sizeof(Node_t));
    curr_node = players.head;

    while (curr_node != NULL) {
        if (curr_node->data.id == id) {
            return curr_node;
        }
        else{
            curr_node = curr_node->next;
        }
    }
    return NULL;
}


int main(int argc, char *argv[])
{
    int tcp_socket, udp_socket, portno, n;
    struct sockaddr_in server_addr;
    struct hostent *server;
    Player_t player;

	// players->head = (Node_t*) malloc(sizeof(Node_t));
	// players->tail = (Node_t*) malloc(sizeof(Node_t));
    players.head = NULL;
    players.tail = NULL;

 //    for (int i=0; i<MAX_PLAYERS; i++) {
 //        bzero(players[i].nick, 20);
 //        players[i].state = Dead;
 //        players[i].type = Pacman;
	// }

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
    char joined = '0';
    while (joined == '0') {
	    //Read message from clients input
	    printf("Please enter the message: ");
	    bzero(buffer,256);
	    fgets(buffer,255,stdin);

	    // char *nick = malloc(20);
	   
	    if (strcmp(get_command_name(buffer), "JOIN") == 0) {
	    	strncpy(player.nick, &buffer[5], 20);
	    	// strncpy(nick, trim(nick), 20);

	    	// char c = 0;
	    	n = write(tcp_socket, JOIN, 1); //send join message to server
	    	if (n < 0)
        		error("ERROR writing join message to socket");
	    	n = write(tcp_socket, player.nick, strlen(player.nick)); //send nickname to server
    		if (n < 0)
        		error("ERROR writing nick to socket");

	    	//get acknowledge from server
	    	bzero(buffer,14);
		    n = read(tcp_socket, &enumber, 1);
		    if (n >= 0 && enumber == ACK) {
		    	n = read(tcp_socket, &player.id, 4);
		    	if (n < 0)
	        		error("ERROR reading player_id from socket");
	        	else {
		    		if (player.id == -1) {
		    			printf("%s\n", "Nick is already taken. Send new JOIN");
		    			continue; //go to start of loop
		    		}
		    		else if (player.id == -2) {
		    			printf("%s\n", "Server is full. Please try again later");
		    			continue;
		    		}
		    		else if (player.id == -3) {
		    			printf("%s\n", "Something is wrong");
		    			continue;
		    		}
		    	}
		    	players_count++;
		    	joined = '1';
		    	printf("%s\n", "You are successfully joined to the game");
		    }
		    else {
		    	if (n < 0)
	        		error("ERROR reading acknowledge from socket");
	        	if (enumber != ACK)
	        		error("ERROR joining to the game");
		    }
	    }
	}

    while (1) {
	    
	    //Read game begining message from server
	   	n = read(tcp_socket, &enumber, 1);
	   	
	    if (n >= 0) {

	    	//JOINED packet
		    if (enumber == JOINED) {
		    	int player_id;
		    	if (read(tcp_socket, &player_id, 4) >= 0 ) {
				    n = read(tcp_socket, &player.nick, 20);
				    if (n >= 0) {
				    	player.id = player_id;
				    	player.x = -1;
				    	player.y = -1;
				    	player.state = Dead;
						player.type = Pacman;
						append(&players, player);
						players_count++;
						refresh_map();
				    }
				    else
				    	error("ERROR reading JOINED player nick");
		    	}
		    	else
		    		error("ERROR reading JOINED player id");

		    }

		    //PLAYER_DISCONNECTED packet. Delete player from players list
		    if (enumber == PLAYER_DISCONNECTED) {

		    	int player_id;
		    	if (read(tcp_socket, &player_id, 4) >= 0 ) {
				    remove_node(&players, player_id);
		    	}
		    	else
		    		error("ERROR reading JOINED player id");

		    }

	    	if (enumber == START) {

	    		char player_x, player_y;
		    	if (!(read(tcp_socket, &map_height, 1) >= 0 
		    		&& read(tcp_socket, &map_width, 1) >= 0 
		    		&& read(tcp_socket, &player_x, 1) >= 0 
		    		&& read(tcp_socket, &player_y, 1) >= 0)) {
	        		error("ERROR reading from socket");
	        	}
	        	else {
	        		players.head->data.x = player_x;
	        		players.head->data.y = player_y;
	        		enumber = -1;
	        		refresh_map();
	        		continue;
	        	}
	        	
	        	
		    }

		    //END of the game
		    if (enumber == END) {
		    	//If game ends, then go back and wait until we recieve START packet
		    	n = 0;
		    	enumber = -1;
		    	destroy_screen();
		    	continue;
		    }

		    //MAP object
		    if (enumber == MAP) {
		    	if (read(tcp_socket, &map, map_height*map_width) < 0 ) {
		    		error("ERROR reading map");
		    	}
		    	else {
		    		init_screen();
					draw_map();
		    		enumber = -1;
		    		continue;
		    	}

		    }

		    //PLAYERS packet
		    if (enumber == PLAYERS) {
		    	int packet_size = 0;
		    	if (read(tcp_socket, &packet_size, 4) >= 0 ) {
		    		int player_id;
		    		float x, y;
		    		enum Player_state state;
		    		enum Player_type type;
		    		Node_t* curr_player;

		    		for (int i=1; i<=packet_size; i++) {
		    			if (read(tcp_socket, &player_id, 4) >= 0 && read(tcp_socket, &x, 4) >= 0 &&
				    		read(tcp_socket, &y, 4) >= 0 && read(tcp_socket, &state, 1) >= 0 && 
				    		read(tcp_socket, &type, 1) >= 0) {
		    				curr_player = find_player(player_id);
		    				if (curr_player != NULL) {
				    			curr_player->data.x = x;
				    			curr_player->data.y = y;
				    			curr_player->data.state = state;
				    			curr_player->data.type = type;
				    		}
				    		else
				    			error("ERROR player not found");
				    	}
				    	else
				    		error("ERROR reading player packet");

				    	enumber = -1;
				    	refresh_map();
				    }

		    	}
		    	else
		    		error("ERROR reading players count");

		    }

	        if (key_pressed()) {
	        	char move = make_move();
	           	if (move >= Up && move <= Left) {
	           		n = write(tcp_socket, (char *)MOVE, 1); //send MOVE message to server
			    	if (n < 0)
		        		error("ERROR writing MOVE message to socket");
			    	n = write(tcp_socket, &players.head->data.id, 4); //send clients player id to server
		    		if (n < 0)
		        		error("ERROR MOVE writing player id to socket");
		        	n = write(tcp_socket, &move, 1); //send clients player movement to server
		    		if (n < 0)
		        		error("ERROR MOVE writing player movement to socket");
	           		refresh_map();
	           	}
	           	else {
	           		//client wants to quit game
	           		if (move == 4) {
	           			destroy_screen();
	           			//TODO: send quit message to server
	           		}
	           	}
	        }

	        

	    }

	
	  
	}

	    
    // if (n < 0)
    //     error("ERROR writing to socket");
    // bzero(buffer,256);
    // n = read(con_socket,buffer,255);
    // if (n < 0)
    //     error("ERROR reading from socket");
    // printf("%s\n",buffer);
    // close(tcp_socket);

    return 0;
}