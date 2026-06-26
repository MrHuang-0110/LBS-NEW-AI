#ifndef __STR_LIST_H
#define __STR_LIST_H
 #include "sys.h"
/*链表节点*/
typedef struct ListNode
{   
	char data[32];
	struct ListNode* prev;
	struct ListNode* next;
}ListNode;

/*链表*/
ListNode* ListInit(void);
void LB_ListDeletIndex(ListNode *List,char *indexStr);
void LB_ListDeletAll(ListNode *List);
void LB_ListIndexPullData(ListNode *List,char *indexStr,char *x);
void LB_ListIndexChangerData(ListNode *List,char *indexStr, char *x);
char *LB_ListGetIndexData(ListNode *List,char *indexStr);
char *LB_ListGetDataIndex(ListNode *List,char*x);
char *LB_ListGetNumber(ListNode *List);
char *LB_ListCmpData(ListNode *List,char *x);
void LB_ListPushData(ListNode *List,char*x);
void LB_ListPrint(ListNode *List);//打印数据
ListNode *getuserlisthandle(void);
void create_user_list(void);
void delete_user_list(void);
#endif

