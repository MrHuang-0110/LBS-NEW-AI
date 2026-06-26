#include "strlist.h"

static ListNode *USER_LIST;

/*初始化链表*/
 ListNode* ListInit(void)
{
	    ListNode *p = (ListNode*)pvPortMalloc(sizeof(ListNode));
	if (p == NULL)
	{
		return NULL;
	}
	ListNode* phead = p;
	phead->next = phead;
	phead->prev = phead;
	return phead;
}

/*创建节点*/
static ListNode* BuyListNode(char *x)
{
 
			ListNode *p = (ListNode*)pvPortMalloc(sizeof(ListNode));
	if (p == NULL)
	{
 
		return NULL;
	}
	ListNode* newNode = p;
	strcpy(newNode->data,x);
 
	//newNode->data = x;
	newNode->next = NULL;
	newNode->prev = NULL;
	return newNode;
}

/*尾插*/
static void ListPushBack(ListNode* phead, char *x)//尾插数据
{
	ListNode* tail = phead->prev;
	ListNode* newnode = BuyListNode(x);
    
	if(newnode == NULL){ 
 
      return;
	} 
 
	//第一个链接
	tail->next = newnode;
	newnode->prev = tail;
	//第二个链接
	phead->prev = newnode;
	newnode->next = phead;
}
 
/*查找节点位置*/
static ListNode* ListFind(ListNode* phead, char* x)
{
	 
	ListNode* cur = phead->next;
	while (cur != phead)
	{
		 if(strcmp(cur->data,x) == 0)
		 { 
			return cur;
		 }
		cur = cur->next;
	}
	return NULL;
}

/*在Pos位置之前插入*/
static void ListInsert(ListNode* pos, char *x)//pos位置之前插入
{
 
	ListNode* newNode = BuyListNode(x);
	ListNode* posPrev = pos->prev;

	// posPrev  newnode pos
	posPrev->next = newNode;
	newNode->prev = posPrev;
	newNode->next = pos;
	pos->prev = newNode;
}

static void ListErase(ListNode* pos)//删除pos位置
{
 
	ListNode* prev = pos->prev;
	ListNode* next = pos->next;
  
	vPortFree(pos);
 
	pos = NULL;
	prev->next = next;
	next->prev = prev;
}

 
/*2.尾部插入*/
void LB_ListPushData(ListNode *List,char*x)
{
    if(List==NULL)
	    return;

	ListPushBack(List,x);
}
/*3.删除链表指定项*/
void LB_ListDeletIndex(ListNode *List,char *indexStr)
{ 
	uint8_t i = 0;
	uint8_t index = abs(atoi(indexStr));

    if(List==NULL)
	    return;
	if(index == 0)
	   return;

	ListNode* cur = List->next;
    
	for(i = 0;i<index;i++)
	{
		if(cur->next!=List)
		{ 
			cur = cur->next;
		}
	}
	ListErase(cur);
}

/*4.删除链表所有内容*/
void LB_ListDeletAll(ListNode *List)
{ 
    if(List==NULL)
	    return;

	ListNode* cur = List->next;
	while (cur != List)
	{
		ListErase(cur);
		cur = cur->next;
	}
} 

/*5.在第n项前插入某数据*/
void LB_ListIndexPullData(ListNode *List,char *indexStr,char *x)
{ 
	uint8_t i = 0;
	uint8_t index = abs(atoi(indexStr));

    if(List==NULL)
	    return;
	if(index == 0)return;

	ListNode* cur = List->next;
    
	for(i = 0;i<index;i++)
	{
		if(cur->next!=List)
		{ 
			cur = cur->next;
		}
	}

	ListInsert(cur,x);	
}

/*6.将第N项的数据替换为某数据*/
void LB_ListIndexChangerData(ListNode *List,char *indexStr, char *x)
{ 
	uint8_t i = 0;
	uint8_t index = abs(atoi(indexStr));
	ListNode* cur = List->next;

	if(index == 0)return;
    
	for(i = 0;i<index;i++)
	{
		if(cur->next!=List)
		{ 
			cur = cur->next;
		}
	}
    memset(cur->data,0,strlen(cur->data));
	strcpy(cur->data,x);	
}
 
/*7.返回第N项的数据*/
char *LB_ListGetIndexData(ListNode *List,char *indexStr)
{ 
	uint8_t i = 0;
	uint8_t index = abs(atoi(indexStr));
	ListNode* cur = List->next;

    if(List==NULL)return "0";

	for(i = 0;i<index;i++)
	{
		if(cur->next!=List)
		{ 
			cur = cur->next;
		}
	}

	return cur->data;	
}

/*8.返回数据的编号*/
char *LB_ListGetDataIndex(ListNode *List,char*x)
{ 
	static uint8_t index;
	static char indexStr[5];
	memset(indexStr,0,sizeof(indexStr));
	ListNode* cur = List->next;
	index = 0;

	if(List==NULL)return "0";

	while (cur != List)
	{
		 if(strcmp(cur->data,x) == 0)
		 { 
			sprintf(indexStr,"%d",index);
			return indexStr;
		 }
		 index++;	 
		cur = cur->next;
	}
	return "0";
}

/*9.返回链表所有数据个数*/
char *LB_ListGetNumber(ListNode *List)
{ 
	uint8_t index = 0;
	static char indexStr[5];
	memset(indexStr,0,sizeof(indexStr));
    if(List==NULL)return "0";
	ListNode* cur = List->next;
	while (cur != List)
	{
		index++;
		cur = cur->next;
	}
	sprintf(indexStr,"%d",index);
	return indexStr;
}

/*10.判断链表内是否包含数据*/
char *LB_ListCmpData(ListNode *List,char *x)
{
	if(NULL!=ListFind(List,x))
	{ 
		return "1";
	}
	
  return "0";
}

void LB_ListPrint(ListNode *List)//打印数据
{
	ListNode* cur = List->next;
	while (cur != List)
	{ 
		cur = cur->next;
		 
		vTaskDelay(100);
	}
}
ListNode *getuserlisthandle(void)
{ 
   return USER_LIST;
}	
void create_user_list(void)
{ 
	 if(USER_LIST == NULL)
	 { 
	    USER_LIST = ListInit();
	 }
   else
	 {
	    vPortFree(USER_LIST);
		  USER_LIST = ListInit();
	 }
}
void delete_user_list(void)
{ 
   vPortFree(USER_LIST);
	 USER_LIST = NULL;
}
