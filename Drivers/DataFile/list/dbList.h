#ifndef __DBLIST_H
#define __DBLIST_H
#include "sys.h"

typedef struct Node {
    int data;           
    struct Node* prev;  
    struct Node* next;  
}Node;

typedef struct {
    Node* head;        
    Node* tail;        
    int size;          
} DoublyLinkedList;

Node* create_node(int data);
DoublyLinkedList* create_list(void);
void insert_head(DoublyLinkedList* list, int data);
void insert_tail(DoublyLinkedList* list, int data);
void insert_at(DoublyLinkedList* list, int index, int data);
void delete_head(DoublyLinkedList* list);
void delete_tail(DoublyLinkedList* list);
void delete_at(DoublyLinkedList* list, int index);
Node* find(DoublyLinkedList* list, int data);
void print_forward(DoublyLinkedList* list);
void print_backward(DoublyLinkedList* list);
void destroy_list(DoublyLinkedList* list);
int get_at_index(DoublyLinkedList* list, int index);
int find_index_by_data(DoublyLinkedList* list, int data);
#endif
