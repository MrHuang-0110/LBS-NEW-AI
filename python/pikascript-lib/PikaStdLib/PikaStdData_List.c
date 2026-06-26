#include "PikaStdData_List.h"
#include "BaseObj.h"
#include "PikaObj.h"
#include "PikaStdData_Tuple.h"
#include "dataStrs.h"

void PikaStdData_List_append(PikaObj* self, Arg* arg) {
    pikaList_append(self, arg_copy(arg));
}

void PikaStdData_List_set(PikaObj* self, int i, Arg* arg) {
    pikaList_set(self, i, arg_copy(arg));
}

void PikaStdData_List___setitem__(PikaObj* self, Arg* __key, Arg* __val) {
    PikaStdData_List_set(self, obj_getInt(self, "__key"),
                         obj_getArg(self, "__val"));
}

void PikaStdData_List___init__(PikaObj* self) {
    pikaList_init(self);
}

pika_float PikaStdData_List_num(PikaObj *self)
{ 
    return (pika_float)pikaList_getSize(self);
}
pika_bool PikaStdData_List_if_data(PikaObj *self, Arg* arg)
{
    if (NULL == arg) {
        return pika_false;
    }
    for (size_t i = 0; i < pikaList_getSize(self); i++) {
        Arg* arg_now = pikaList_get(self, i);
        if (arg_isEqual(arg_now, arg)) {
            return pika_true;
        }
    }
    return pika_false;
}
void PikaStdData_List_remove_all(PikaObj *self)
{ 
    while (pikaList_getSize(self) > 0) {
        Arg* arg_now = pikaList_get(self, 0);
        if (NULL == arg_now) {
            break;
        }
        pikaList_remove(self, arg_now);
    }
}
void PikaStdData_List_remove_index(PikaObj *self, int i)
{ 
    int top = (int)pikaList_getSize(self);
    if (i < 0) {
        i = top + i;
    }
    if (i < 0 || i >= top) {
        obj_setErrorCode(self, PIKA_RES_ERR_INDEX);
        obj_setSysOut(self, "IndexError: index out of range");
        return;
    }
    Arg* res = pikaList_pop_withIndex(self, i);
    if (NULL != res) {
        arg_deinit(res);
    }
}
pika_float PikaStdData_List_dataToindex(PikaObj *self, Arg* arg)
{ 
    if (NULL == arg) {
        return -1;
    }
    for (size_t i = 0; i < pikaList_getSize(self); i++) {
        Arg* arg_now = pikaList_get(self, i);
        if (arg_isEqual(arg_now, arg)) {
            return (pika_float)i;
        }
    }
    return -1;
}

char* builtins_str(PikaObj* self, Arg* arg);
typedef struct {
    Arg* buf;
    int count;
} ListToStrContext;

int32_t listToStrEachHandle(PikaObj* self,
                            int itemIndex,
                            Arg* itemEach,
                            void* context) {
    ListToStrContext* ctx = (ListToStrContext*)context;

    char* item_str = builtins_str(self, itemEach);
    if (ctx->count != 0) {
        ctx->buf = arg_strAppend(ctx->buf, ", ");
    }
    if (arg_getType(itemEach) == ARG_TYPE_STRING) {
        ctx->buf = arg_strAppend(ctx->buf, "'");
    }
    ctx->buf = arg_strAppend(ctx->buf, item_str);
    if (arg_getType(itemEach) == ARG_TYPE_STRING) {
        ctx->buf = arg_strAppend(ctx->buf, "'");
    }

    ctx->count++;

    return 0;
}

char* PikaStdData_List___str__(PikaObj* self) {
    ListToStrContext context;
    context.buf = arg_newStr("[");
    context.count = 0;

    pikaList_forEach(self, listToStrEachHandle, &context);

    context.buf = arg_strAppend(context.buf, "]");
    obj_setStr(self, "_buf", arg_getStr(context.buf));
    arg_deinit(context.buf);
    return obj_getStr(self, "_buf");
}

void PikaStdData_List_reverse(PikaObj* self) {
    pikaList_reverse(self);
}

PikaObj* PikaStdData_List___add__(PikaObj* self, PikaObj* others) {
    PikaObj* res = New_PikaList();
    for (size_t i = 0; i < pikaList_getSize(self); i++) {
        Arg* arg = pikaList_get(self, i);
        pikaList_append(res, arg_copy(arg));
    }
    for (size_t i = 0; i < pikaList_getSize(others); i++) {
        Arg* arg = pikaList_get(others, i);
        pikaList_append(res, arg_copy(arg));
    }
    return res;
}

void PikaStdData_List_insert(PikaObj* self, int i, Arg* arg) {
    if (PIKA_RES_OK != pikaList_insert(self, i, arg)) {
        obj_setErrorCode(self, 1);
        obj_setSysOut(self, "Error: index exceeded lengh of list.");
    }
}

Arg* PikaStdData_List_pop(PikaObj* self, PikaTuple* index) {
    int i = 0;
    if (pikaTuple_getSize(index) == 1) {
        if (pikaTuple_getType(index, 0) == ARG_TYPE_INT) {
            i = pikaTuple_getInt(index, 0);
        } else {
            obj_setErrorCode(self, 1);
            obj_setSysOut(self, "Error: index must be int.");
            return NULL;
        }
        return pikaList_pop_withIndex(self, i);
    } else {
        return pikaList_pop(self);
    }
}

void PikaStdData_List_remove(PikaObj* self, Arg* val) {
    pikaList_remove(self, val);
}

PikaObj* PikaStdData_List___mul__(PikaObj* self, int n) {
    PikaObj* res = New_PikaList();
    for (int i = 0; i < n; i++) {
        for (size_t j = 0; j < pikaList_getSize(self); j++) {
            Arg* arg = pikaList_get(self, j);
            pikaList_append(res, arg_copy(arg));
        }
    }
    return res;
}
