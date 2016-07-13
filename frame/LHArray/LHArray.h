//
//  LHArray.h
//  LHArray
//
//  Created by 李浩 on 16/6/4.
//  Copyright © 2016年 李浩. All rights reserved.
//

#ifndef LHArray_h
#define LHArray_h

#include <stdio.h>
#include <MacTypes.h>
#include <stdbool.h>

typedef unsigned int lh_int;

typedef struct lh_array LHArrayRef;

typedef void (*lh_arrayApplierFunction)(const void *value, void *context);

typedef void* (*lh_arrayValueRetain)(const void* value);

typedef void (*lh_arrayValueRelease)(const void* value);

typedef bool (*lh_arrayValueEqual)(const void* value1,const void* value2);

struct lh_array {
    lh_int count;
    struct lh_LinkNode* header;
    struct lh_LinkNode* tail;
};

struct lh_LinkNode {
    struct lh_LinkNode* prev;
    struct lh_LinkNode* next;
    void* value;
};

typedef struct {
    lh_arrayValueRetain retain;
    lh_arrayValueRelease release;
    lh_arrayValueEqual equal;
}LHArrayCallBacks;

/**
 *@dis 创建数组
 */
LHArrayRef lh_arrayCreate();

/**
 *@dis 创建数组
 *@para 原数组
 *@return 返回的新数组
 */
LHArrayRef lh_arrayCreateWithArray(LHArrayRef* arrayRef);

/**
 *@dis  返回数组长度
 */
lh_int lh_arrayGetCount(LHArrayRef* arrayRef);

/**
 *@dis 遍历数组
 *@para applier 函数指针 遍历会回调applier
 *@para context 上下文
 */
void lh_arrayApplyFunction(LHArrayRef* arrayRef,lh_arrayApplierFunction applier,void* context);

/**
 *@dis 在数组尾部插入数据
 */
void lh_arrayAppentValue(LHArrayRef* arrayRef,void* value);

/**
 *@dis 在指定的位置插入数据
 */
void lh_arrayInsertValueAtIndex(LHArrayRef* arrayRef,lh_int index,void* value);

/**
 *@dis 获取固定下标的数据
 */
void* lh_arrayGetValueWithIndex(LHArrayRef* arrayRef,lh_int index);

/**
 *@dis 获取数组第一个元素
 */
void* lh_arrayGetFirstValue(LHArrayRef* arrayRef);

/**
 *@dis 获取数组最后一个元素
 */
void* lh_arrayGetLastValue(LHArrayRef* arrayRef);

/**
 *@dis 删除数组中指定下标的元素
 */
void lh_arrayRemoveValueAtIndex(LHArrayRef* arrayRef,lh_int index);

/**
 *@dis 删除数组中最后一个元素
 */
void lh_arrayRemoveLastValue(LHArrayRef* arrayRef);

/**
 *@dis 删除数组中第一个元素
 */
void lh_arrayRemoveFirstValue(LHArrayRef* arrayRef);

/**
 *@dis  删除数组中所有元素
 */
void lh_arrayRemoveAllValue(LHArrayRef* arrayRef);

/**
 *@dis 交换数组中两个元素
 */
void lh_arrayExchangeValuesAtIndexWithIndex(LHArrayRef* arrayRef,lh_int fromIndex,lh_int toIndex);


/**
 *@dis 替换数组中指定下标的元素
 */
void lh_arrayReplaceValueAtIndex(LHArrayRef* arrayRef,lh_int index,void* value);

/**
 *@dis  释放数组占用的内存 释放后无法再次使用
 */
void lh_release(LHArrayRef* arrayRef);

#endif /* LHArray_h */
