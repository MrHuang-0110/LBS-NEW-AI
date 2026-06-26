#include "dblist.h"
//#include "malloc.h"


// 创建新节点
Node* create_node(int data) {
    Node* new_node = (Node*)pvPortMalloc(sizeof(Node));
    if (new_node == NULL) {
				return NULL;
    }
    new_node->data = data;
    new_node->prev = NULL;
    new_node->next = NULL;
    return new_node;
}

// 初始化链表
DoublyLinkedList* create_list() {
    DoublyLinkedList* list = (DoublyLinkedList*)pvPortMalloc(sizeof(DoublyLinkedList));
    if (list == NULL) {
				return NULL;
    }
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    return list;
}

// 在链表头部插入节点
void insert_head(DoublyLinkedList* list, int data) {
    Node* new_node = create_node(data);
    
    if (list->size == 0) {
        // 空链表
        list->head = new_node;
        list->tail = new_node;
    } else {
        // 非空链表
        new_node->next = list->head;
        list->head->prev = new_node;
        list->head = new_node;
    }
    list->size++;
}

// 在链表尾部插入节点
void insert_tail(DoublyLinkedList* list, int data) {
    Node* new_node = create_node(data);
    
    if (list->size == 0) {
        list->head = new_node;
        list->tail = new_node;
    } else {
        new_node->prev = list->tail;
        list->tail->next = new_node;
        list->tail = new_node;
    }
    list->size++;
}

// 在指定位置插入节点
void insert_at(DoublyLinkedList* list, int index, int data) {
    if (index < 0 || index > list->size) {
        return;
    }
    
    if (index == 0) {
        insert_head(list, data);
        return;
    }
    
    if (index == list->size) {
        insert_tail(list, data);
        return;
    }
    
    // 找到插入位置的前一个节点
    Node* current = list->head;
    for (int i = 0; i < index - 1; i++) {
        current = current->next;
    }
    
    Node* new_node = create_node(data);
    new_node->prev = current;
    new_node->next = current->next;
    current->next->prev = new_node;
    current->next = new_node;
    list->size++;
}

// 删除头节点
void delete_head(DoublyLinkedList* list) {
    if (list->size == 0) {
        return;
    }
    
    Node* temp = list->head;
    if (list->size == 1) {
        list->head = NULL;
        list->tail = NULL;
    } else {
        list->head = list->head->next;
        list->head->prev = NULL;
    }
    
    vPortFree(temp);
    list->size--;
}

// 删除尾节点
void delete_tail(DoublyLinkedList* list) {
    if (list->size == 0) {
        return;
    }
    
    Node* temp = list->tail;
    if (list->size == 1) {
        list->head = NULL;
        list->tail = NULL;
    } else {
        list->tail = list->tail->prev;
        list->tail->next = NULL;
    }
    
    vPortFree(temp);
    list->size--;
}

// 删除指定位置的节点
void delete_at(DoublyLinkedList* list, int index) {
    if (index < 0 || index >= list->size) {
        return;
    }
    
    if (index == 0) {
        delete_head(list);
        return;
    }
    
    if (index == list->size - 1) {
        delete_tail(list);
        return;
    }
    
    // 找到要删除的节点
    Node* current = list->head;
    for (int i = 0; i < index; i++) {
        current = current->next;
    }
    
    current->prev->next = current->next;
    current->next->prev = current->prev;
		vPortFree(current);
    list->size--;
}

// 查找节点
Node* find(DoublyLinkedList* list, int data) {
    Node* current = list->head;
    while (current != NULL) {
        if (current->data == data) {
            return current;
        }
        current = current->next;
    }
    return NULL;
		
}
int get_at_index(DoublyLinkedList* list, int index) {
    if (index < 0 || index >= list->size) {
        return -1; 
    }
    Node* current;
    if (index < list->size / 2) {
        current = list->head;
        for (int i = 0; i < index; i++) {
            current = current->next;
        }
    } else {
        current = list->tail;
        for (int i = list->size - 1; i > index; i--) {
            current = current->prev;
        }
    }
    return current->data;
}

// 根据数据值查找节点的索引位置（从0开始）
// 如果找到返回索引，否则返回-1
int find_index_by_data(DoublyLinkedList* list, int data) {
    if (list == NULL || list->size == 0) {
        return -1;
    }
    
    Node* current = list->head;
    int index = 0;
    
    while (current != NULL) {
        if (current->data == data) {
            return index;
        }
        current = current->next;
        index++;
    }
    
    return -1; // 未找到
}
// 正向遍历打印链表
void print_forward(DoublyLinkedList* list) {
    Node* current = list->head;
    while (current != NULL) {
        current = current->next;
    }
}

// 反向遍历打印链表
void print_backward(DoublyLinkedList* list) {
    Node* current = list->tail;
    while (current != NULL) {
        current = current->prev;
    }
}

// 销毁链表
void destroy_list(DoublyLinkedList* list) {
    Node* current = list->head;
    while (current != NULL) {
        Node* temp = current;
        current = current->next;
			  vPortFree(temp);	  
    }
		vPortFree(list);	
}
