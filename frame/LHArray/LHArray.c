//
//  LHArray.c
//  LHArray
//
//  Created by 李浩 on 16/6/4.
//  Copyright © 2016年 李浩. All rights reserved.
//

#include "LHArray.h"
#include <stdlib.h>

#define lh_array_malloc (struct lh_array*)malloc(sizeof(struct lh_array))

#define lh_linknode_malloc (struct lh_LinkNode*)malloc(sizeof(struct lh_LinkNode))

LHArrayRef lh_arrayCreate()
{
    struct lh_array* array = lh_array_malloc;
    array->count = 0;

    array->header = lh_linknode_malloc;
    array->tail = lh_linknode_malloc;
    
    array->header->next = array->tail;
    array->header->prev = NULL;
    array->tail->prev = array->header;
    array->tail->next = NULL;
    
    return *array;
}

LHArrayRef lh_arrayCreateWithArray(LHArrayRef* arrayRef)
{
    struct lh_array* array = lh_array_malloc;
    array->count = 0;
    array->header = lh_linknode_malloc;
    array->tail = lh_linknode_malloc;
    array->header->next = array->tail;
    array->header->prev = NULL;
    array->tail->prev = array->header;
    array->tail->next = NULL;
    
    struct lh_LinkNode* node = arrayRef->header;
    while (node&&node!=arrayRef->tail) {
        node = node->next;
        lh_arrayAppentValue(array, node->value);
    }
    return *array;
}

lh_int lh_arrayGetCount(LHArrayRef* arrayRef)
{
    if (arrayRef == NULL) {
        return 0;
    }
    
    
    return arrayRef->count;
}

void lh_arrayApplyFunction(LHArrayRef* arrayRef,lh_arrayApplierFunction applier,void* context)
{
    if (arrayRef == NULL) {
        return;
    }
    struct lh_LinkNode* node = arrayRef->header;
    while (node) {
        node = node->next;
        if (node == arrayRef->tail) {
            return;
        }
        applier(node->value,context);
    }
}

void lh_arrayInsertValueAtIndex(LHArrayRef* arrayRef,lh_int index,void* value)
{
    if (arrayRef == NULL) {
        return;
    }
    lh_int count = arrayRef->count;
    if (index > count) {
        return;
    }
    struct lh_LinkNode* insertNode = lh_linknode_malloc;
    insertNode->value = value;
    struct lh_LinkNode* node = NULL;
    if (index >= count/2) {
        node = arrayRef->tail;
        int i = count-index;
        while (i) {
            node = node->prev;
            i--;
        }
        if (node) {
            node->prev->next = insertNode;
            insertNode->prev = node->prev;
            insertNode->next = node;
            node->prev = insertNode;
        }else {
            arrayRef->tail->prev->next = insertNode;
            insertNode->prev = arrayRef->tail->prev;
            insertNode->next = arrayRef->tail;
            arrayRef->tail->prev = insertNode;
        }
        
    }else {
        node = arrayRef->header->next;
        while (index) {
            node = node->next;
            index--;
        }
        if (node) {
            node->prev->next = insertNode;
            insertNode->prev = node->prev;
            insertNode->next = node;
            node->prev = insertNode;
        }else {
            arrayRef->header->next->prev = insertNode;
            insertNode->next = arrayRef->header->next;
            insertNode->prev = arrayRef->header;
            arrayRef->header->next = insertNode;
        }
    }
    
    arrayRef->count += 1;
}

void lh_arrayAppentValue(LHArrayRef* arrayRef,void* value)
{
    lh_int index = arrayRef->count;
    lh_arrayInsertValueAtIndex(arrayRef, index, value);
}

void lh_arrayPreppentValue(LHArrayRef* arrayRef,void* value)
{
    lh_int index = 0;
    lh_arrayInsertValueAtIndex(arrayRef, index, value);
}

void* lh_arrayGetValueWithIndex(LHArrayRef* arrayRef,lh_int index)
{
    if (arrayRef == NULL) {
        return NULL;
    }
    lh_int count = lh_arrayGetCount(arrayRef);
    if (index >= count) {
        return NULL;
    }
    
    if (index >= count/2) {
        int i = count-index;
        struct lh_LinkNode* node = arrayRef->tail;
        while (i) {
            node = node->prev;
            i--;
        }
        if (node) {
            return node->value;
        }else
            return NULL;
    }else {
        struct lh_LinkNode* node = arrayRef->header->next;
        while (index) {
            node = node->next;
            index--;
        }
        if (node) {
            return node->value;
        }else
            return NULL;
    }
}

void* lh_arrayGetFirstValue(LHArrayRef* arrayRef)
{
    lh_int index = 0;
    return lh_arrayGetValueWithIndex(arrayRef, index);
}

void* lh_arrayGetLastValue(LHArrayRef* arrayRef)
{
    lh_int index = arrayRef->count-1;
    return lh_arrayGetValueWithIndex(arrayRef, index);
}

void lh_arrayRemoveValueAtIndex(LHArrayRef* arrayRef,lh_int index)
{
    if (arrayRef == NULL) {
        return;
    }
    lh_int count = arrayRef->count;
    if (index >= count) {
        return;
    }
    if (index >= count/2) {
        lh_int i = count-index;
        struct lh_LinkNode* node = arrayRef->tail;
        while (i) {
            node = node->prev;
            i--;
        }
        if (node) {
            node->prev->next = node->next;
            node->next->prev = node->prev;
            free(node);
        }else
            return;
    }else {
        struct lh_LinkNode* node = arrayRef->header->next;
        while (index) {
            node = node->next;
            index--;
        }
        if (node) {
            node->prev->next = node->next;
            node->next->prev = node->prev;
            free(node);
        }else
            return;
    }
    arrayRef->count -= 1;
}

void lh_arrayRemoveLastValue(LHArrayRef* arrayRef)
{
    lh_int index = arrayRef->count-1;
    lh_arrayRemoveValueAtIndex(arrayRef, index);
}

void lh_arrayRemoveFirstValue(LHArrayRef* arrayRef)
{
    lh_int index = 0;
    lh_arrayRemoveValueAtIndex(arrayRef, index);
}

void lh_arrayRemoveAllValue(LHArrayRef* arrayRef)
{
    struct lh_LinkNode* node = arrayRef->header;
    while (node&&node!=arrayRef->tail) {
        node = node->next;
        if (node) {
            free(node);
        }
    }
    arrayRef->header->next = arrayRef->tail;
    arrayRef->tail->prev = arrayRef->header;
    arrayRef->count = 0;
}

void lh_arrayExchangeValuesAtIndexWithIndex(LHArrayRef* arrayRef,lh_int fromIndex,lh_int toIndex)
{
    if (arrayRef == NULL) {
        return;
    }
    lh_int count = arrayRef->count;
    if (fromIndex>=count || toIndex>=count) {
        return;
    }
    
    struct lh_LinkNode* node = arrayRef->header;
    struct lh_LinkNode* fromNode = NULL;
    struct lh_LinkNode* toNode = NULL;
    int i = 0;
    while (node) {
        node = node->next;
        if (node == arrayRef->tail) {
            break;
        }
        if (i == fromIndex) {
            fromNode = node;
        }
        if (i == toIndex) {
            toNode = node;
        }
        i++;
    }
    
    if (fromNode == NULL || toNode == NULL) {
        return;
    }
    
    void* value = fromNode->value;
    fromNode->value = toNode->value;
    toNode->value = value;
}

void lh_arrayReplaceValueAtIndex(LHArrayRef* arrayRef,lh_int index,void* value)
{
    if (arrayRef == NULL) {
        return;
    }
    lh_int count = lh_arrayGetCount(arrayRef);
    if (index>=count) {
        return;
    }
    struct lh_LinkNode* node = NULL;
    if (index>=count/2) {
        int i = count-index;
        node = arrayRef->tail;
        while (i) {
            node = node->prev;
            i--;
        }
        if (node) {
            node->value = value;
        }
    }else {
        node = arrayRef->header->next;
        while (index) {
            node = node->next;
            index--;
        }
        if (node) {
            node->value = value;
        }
    }
}

void lh_release(LHArrayRef* arrayRef)
{
    if (arrayRef == NULL) {
        return;
    }
    struct lh_LinkNode* node = arrayRef->header;
    while (node) {
        free(node);
        node = node->next;
        if (node == arrayRef->tail) {
            free(node);
            return;
        }
    }
}
