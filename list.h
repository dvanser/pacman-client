#ifndef LIST_HEADER
#define LIST_HEADER

#include "enums.h"

typedef struct Player {
    int id;
	char nick[20];
	int x;
	int y;
	int prev_x;
	int prev_y;
	enum Player_state state;
	enum Player_type type;
} Player_t;

typedef struct Node {
	Player_t data;
	struct Node* next;
} Node_t;

typedef struct Players {
    struct Node* head;
    struct Node* tail;
} Players_t;


//Pieliek saraksta beigās jaunu mezglu
void append(Players_t* list, Player_t new_data);

//Izņem no saraksta elementu, uz kuru norāda "node"
void remove_node(Players_t* list, int id);

//Atkļūdošanai
void print_list(Players_t* list);

#endif