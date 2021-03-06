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


// char map[100][100] = {{1,1,1,1,1,1}, {1,2,3,1,1,1}, {1,0,0,0,0,1}, {1,5,5,4,2,1}, {1,1,1,1,1,1}};
char map[100][100];
char map_height, map_width;
int players_count = 0;
Players_t players;


void error(const char *msg)
{
    perror(msg);
    exit(0);
}

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
    if (!player) 
        error("ERROR cannot allocate memory for player in refresh_map");
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

//Serach player in players array by id. If not found return -1
Node_t* find_player(int id) {

    Node_t* curr_node = (Node_t*) malloc(sizeof(Node_t));
    if (!curr_node) 
        error("ERROR cannot allocate memory for curr_node in find_player");
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

//function prints and updates players score
void print_score(int id, int score, int x) {

    Node_t* player = (Node_t*) malloc(sizeof(Node_t));
    if (!player) 
        error("ERROR cannot allocate memory for player in print_score");

    player = find_player(id);
    player->data.score = score;

    mvprintw(x, 1, player->data.nick);
    char message[4];
    sprintf(message, "%d", score);
    mvprintw(x, sizeof(player->data.nick) + 1, " ");
    mvprintw(x + 1, 1, message);
}

void print_message(int id, char *message) {

    Node_t* player = (Node_t*) malloc(sizeof(Node_t));
    if (!player) 
        error("ERROR cannot allocate memory for player in print_score");

    //message from player
    if (id > 0) {
        player = find_player(id);
        mvprintw(map_height + 1, 1, player->data.nick);
        mvprintw(map_height + 1, sizeof(player->data.nick) + 1, message);
    }
    else {
        //message from server
        if (id == 0) {
             mvprintw(map_height + 1, 1, "Server:");
             mvprintw(map_height + 1, 9, message);
        }
    }
}

//if client starts typing message
char* write_message() {

    char *message = malloc(256);
    getstr(message);
    return message;
}

int key_pressed() {
    int ch = getch();

    if (ch != ERR) {
        ungetch(ch); //return character to queue
        return 1;
    } else {
        return 0;
    }
}

int make_move() {
    int ch = getch();
    if (ch == '\033') { // if the first value is esc
        getch(); // skip the [
        switch(getch()) { // the real value
            case 'A':
                // code for arrow up
                if (players.head->data.y > 0 && map[players.head->data.x][players.head->data.y-1] != 1)
                    players.head->data.y--;
                return Up;
                break;
            case 'B':
                // code for arrow down
                if (players.head->data.y < map_height - 1 && map[players.head->data.x][players.head->data.y+1] != 1)
                    players.head->data.y++;
                return Down;
                break;
            case 'C':
                // code for arrow right
                if (players.head->data.x < map_width - 1 && map[players.head->data.x+1][players.head->data.y] != 1)
                    players.head->data.x++;
                return Right;
                break;
            case 'D':
                // code for arrow left
                if (players.head->data.x > 0 && map[players.head->data.x-1][players.head->data.y] != 1)
                    players.head->data.x--;
                return Left;
                break;
        }
    }
    else {
        if (ch == 'q') //client eneters q to quit
            return 4;
        if (ch == 'm') //client eneters m to write message
            return 5;
    }
}

char *get_command_name(char *string)
{
    char *res = (char*) malloc(256);
    if (!res) 
        error("ERROR cannot allocate memory for res in get_command_name");
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
    if (!res) 
        error("ERROR cannot allocate memory for res in trim");
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

int main(int argc, char *argv[])
{
    int tcp_socket, udp_socket, portno, n;
    struct sockaddr_in server_addr;
    struct hostent *server;
    Player_t player;

    players.head = NULL;
    players.tail = NULL;

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

        char nick[21];
        bzero(buffer,256);
        fgets(nick,20,stdin);

        if (strnlen(nick, 20) > 0) {
            strncpy(player.nick, nick, 20);

            buffer[0] = 0;
            strncpy(&buffer[1], nick, 20);
            n = write(tcp_socket, &buffer, 21); //send join message to server
            if (n < 0)
                error("ERROR writing join message to socket");

            //get acknowledge from server
            bzero(buffer,5);
            n = read(tcp_socket, buffer, 5);
            if (n >= 0 && buffer[0] == ACK) {
                player.id = 0;
                memcpy(&player.id, &buffer[1], 4);
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

                append(&players, player);
                players_count++;
                joined = '1';
                printf("%s\n", "You are successfully joined to the game");
            }
            else {
                if (n < 0)
                    error("ERROR reading acknowledge from socket");
                if (buffer[0] != ACK)
                    error("ERROR joining to the game");
            }
        }
    }

    char buff[2505];
    bzero(buff, 2505);
    int player_id = 0;
    int packet_size = 1;
    char player_x;
    char player_y;
    int want_bytes = 0;
    while (1) {

        //Read first byte from server
        n = read(tcp_socket, buff, 1);
        if(n == 1){
            switch (buff[0]) {
                case JOINED:
                    want_bytes = 0;
                    while(want_bytes != 24){
                        n = read(tcp_socket, buff, 24 - want_bytes);
                        if(n == -1){
                            printf("error");
                            break;
                        }
                        want_bytes+=n;
                    }

                    memcpy(&player_id, &buff[0], 4);
                    memcpy(&player.nick, &buff[4], 20);

                    //Initialize player
                    player.id = player_id;
                    player.x = -1;
                    player.y = -1;
                    player.state = Dead;
                    player.type = Pacman;
                    append(&players, player);
                    players_count++;
                    bzero(buff, 25);

                    refresh_map();
                    break;

                case PLAYER_DISCONNECTED:
                    want_bytes = 0;

                    while(want_bytes != 4){
                        n = read(tcp_socket, buff, 4 - want_bytes);
                        if(n == -1){
                            printf("error");
                            break;
                        }
                        want_bytes+=n;
                    }

                    memcpy(&player_id, &buff[0], 4);
                    remove_node(&players, player_id);
                    bzero(buff, 5);
                    break;

                case START:
                    want_bytes = 0;
                    while(want_bytes != 4){
                        n = read(tcp_socket, buff, 4 - want_bytes);
                        if(n == -1){
                            printf("error");
                            break;
                        }
                        want_bytes+=n;
                    }
                    map_width = buff[0];
                    map_height = buff[1];
                    player_x = buff[2];
                    player_y = buff[3];
                    bzero(buff, 4);
                    break;

                case END:
                    bzero(buff, 5);
                    mvprintw(1, 1, "Game Over!");
                    destroy_screen();
                    break;

                case MAP:
                    
                    if (map_width == 0 || map_height == 0) {
                        printf("ERROR no map deminssions");
                        break;
                    }

                    want_bytes = 0;
                    while(want_bytes != map_width*map_height){
                        n = read(tcp_socket, buff, map_width*map_height - want_bytes);
                        if(n == -1){
                            printf("error");
                            break;
                        }
                        want_bytes+=n;
                    }
                    memcpy(&map, &buff, map_width*map_height); //copu map from buffer
                    draw_map(); //first draw of map
                    bzero(buffer, map_width*map_height);
                    break;

                case PLAYERS:
                    want_bytes = 0;
                    while(want_bytes != 4){
                        n = read(tcp_socket, buff,  4);
                        if(n == -1){
                            printf("error");
                            break;
                        }
                        want_bytes+=n;
                    }

                    float x, y;
                    enum Player_state state;
                    enum Player_type type;
                    Node_t* curr_player; //pointer to player, which sould be updated

                    memcpy(&packet_size, &buff, 4);

                    want_bytes = 0;
                    while(want_bytes != packet_size*14) {
                        n = read(tcp_socket, buff,  14);
                        if(n == -1){
                            printf("error");
                            break;
                        }            

                        memcpy(&player_id, &buff, 4);  
                        memcpy(&x, &buff[4], 4); 
                        memcpy(&y, &buff[8], 4); 
                        memcpy(&state, &buff[12], 1);  
                        memcpy(&type, &buff[13], 1);                     

                        //update player info
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

                    bzero(buff,packet_size*14 + 4);
                    break;

                case SCORE:
                    want_bytes = 0;
                    while(want_bytes != 4){
                        n = read(tcp_socket, buff,  4);
                        if(n == -1){
                            printf("error");
                            break;
                        }
                        want_bytes+=n;
                    }

                    player_id = 0;
                    packet_size = 0;
                    int score = 0;
                    int i = 2;

                    memcpy(&packet_size, &buff, 4);

                    want_bytes = 0;
                    while(want_bytes != packet_size*8) {
                        n = read(tcp_socket, buff, 8);
                        if(n == -1){
                            printf("error");
                            break;
                        }            

                        memcpy(&score, &buff, 4);  
                        memcpy(&player_id, &buff[4], 4);   
                        print_score(player_id, score, map_height+i); 
                        i += 2;   
                        want_bytes += n;       
                    }
                    bzero(buff,packet_size*14 + 4);
                    break;

                case MESSAGE:
                    want_bytes = 0;
                    while(want_bytes != 4){
                        n = read(tcp_socket, buff,  4);
                        if(n == -1){
                            printf("error");
                            break;
                        }
                        want_bytes+=n;
                    }
                    player_id = -1;
                    memcpy(&player_id, &buff, 4);
                        
                    want_bytes = 0;
                    while(want_bytes != 4){
                        n = read(tcp_socket, buff,  4);
                        if(n == -1){
                            printf("error");
                            break;
                        }
                        want_bytes+=n;
                    }
                    packet_size = 0;
                    memcpy(&packet_size, &buff, 4);

                    char *message = malloc(256); 
                    want_bytes = 0;
                    while(want_bytes != packet_size) {
                        n = read(tcp_socket, buff, packet_size);
                        if(n == -1){
                            printf("error");
                            break;
                        }            
                       want_bytes+=n;        
                    }
                    memcpy(&message, &buff, packet_size);

                    print_message(player_id, message);

                    bzero(buff,packet_size + 8);
                    break;
                default:
                    if (key_pressed()) {
                        char move = make_move();
                        if (move >= Up && move <= Left) {
                            bzero(buff, 6);
                            buff[0] = MOVE;
                            memcpy(&buff[1], &players.head->data.id, 4);
                            memcpy(&buff[5], &move, 1);
                            n = write(tcp_socket, &buff, 6); //send move message to server
                            if (n < 0)
                                error("ERROR MOVE writing to socket");
                            refresh_map();
                        }
                        else {
                            //client wants to quit game
                            if (move == 4) {
                                destroy_screen();
                                buff[0] = QUIT;
                                memcpy(&buff[1], &players.head->data.id, 4);
                                n = write(tcp_socket, &buff, 6); //send move message to server
                                if (n < 0)
                                    error("ERROR QUIT writing to socket");
                                close(tcp_socket);
                            }
                            // if (move == 5) {
                            //     // buff[0] = 
                            //     memcpy(&buff, &players.head->data.id, 4);
                            //     n = write(tcp_socket, &buff, 6); //send move message to server
                            //     if (n < 0)
                            //         error("ERROR QUIT writing to socket");
                            //     close(tcp_socket);
                            // }
                        }
                    }
                    break;
            }
        }
        else {
            if (n < 0)
                error("ERROR reading socket");
        }

    }

    close(tcp_socket);
    return 0;
}