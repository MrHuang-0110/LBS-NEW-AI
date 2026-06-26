#include "_list.h"
#include "strlist.h"
#include "PikaVM.h"



void _list_changer_data(PikaObj *self, pika_float index, Arg* data)

{ 
  #if 0
   ListNode *list = getuserlisthandle();
	 if(list == NULL)return;
	
   char str[16];
	 memset(str,0,sizeof(str));
	 sprintf(str,"%d",(int)index);
	 if(list!=NULL)
	 { 
	    LB_ListIndexChangerData(list,str,data);
	 } 
   #else
     
       pikaList_set(self,index,data);
   #endif
}

pika_float _list_cmp_data(PikaObj *self, char* data)
{ 
	 ListNode *list = getuserlisthandle();
	 if(list == NULL)return 0;
	
   return atof(LB_ListCmpData(list,data));
}

pika_float _list_data_index(PikaObj *self, char* data)
{ 
   ListNode *list = getuserlisthandle();
	 if(list == NULL)return 0;
	
   return atof(LB_ListGetDataIndex(list,data));
}

void _list_delet_all(PikaObj *self)
{ 
   ListNode *list = getuserlisthandle();
	 if(list == NULL)return;
	
	 LB_ListDeletAll(list);
	
}
void _list_delet_index(PikaObj *self, pika_float index)
{ 
   ListNode *list = getuserlisthandle();
	 if(list == NULL)return;
	
	 char str[16];
	 memset(str,0,sizeof(str));
	 sprintf(str,"%d",(int)index);
 
	  LB_ListDeletIndex(list,str);
 
}
char* _list_index_data(PikaObj *self, pika_float index)
{   
   ListNode *list = getuserlisthandle();
	 if(list == NULL)return NULL;
   char str[16];
	 memset(str,0,sizeof(str));
	 sprintf(str,"%d",(int)index);
   return LB_ListGetIndexData(list,str);
}
pika_float _list_num(PikaObj *self)
{ 
   ListNode *list = getuserlisthandle();
	 if(list == NULL)return 0;	
	 
	 return atof(LB_ListGetNumber(list)); 
}
void _list_pul(PikaObj *self, pika_float index, char* data)
{ 
   ListNode *list = getuserlisthandle();
	 if(list == NULL)return;
	 char str[16];
	 memset(str,0,sizeof(str));
	 sprintf(str,"%d",(int)index);
 
	 LB_ListIndexPullData(list,str,data);
 
}
void _list_push(PikaObj *self, Arg* data)

{ 
#if 0
   ListNode *list = getuserlisthandle();
	 if(list == NULL)return;
	
	 LB_ListPushData(list,data);
 #endif
    pikaList_append(self,data);
    
}

