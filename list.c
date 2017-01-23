#include "list.h"
#include <stdlib.h>
#include <stdio.h>
#include "enums.h"


void append(Players_t* list, Player_t new_data) {
    Node_t* new_node = (Node_t*) malloc(sizeof(Node_t));
 
    new_node->data  = new_data;
    new_node->next = NULL;
 
    if (list->head == NULL)
    {
       list->head  = new_node;
       return;
    }  
      
    
    while (list->tail->next != NULL)
        list->tail = list->tail->next;

    list->tail->next = new_node;
    return;    
}

void remove_node(Players_t* list, int id) {
    Node_t* curr_node = (Node_t*) malloc(sizeof(Node_t));
    Node_t* prev_node = (Node_t*) malloc(sizeof(Node_t));
    curr_node = list->head;
    prev_node = list->head;

    while (curr_node != NULL) {
        if (curr_node->data.id == id) {
            if (curr_node == list->head) {
                list->head = curr_node->next;
                free(list->head);
            }
            else {
                prev_node->next = curr_node->next;
                free(curr_node);
            }
            return;
        }
        else{
            prev_node = curr_node;
            curr_node = curr_node->next;
        }
    }

    return;
}

void print_list(Players_t* list) {
    printf("List: ");
    Node_t* i;
    for (i = list->head; i != NULL; i = i->next) {
        printf("%i ", i->data.id);
    }
    printf("\n");
}