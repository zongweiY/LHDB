//
//  LHDictionary.h
//  LHDictionary
//
//  Created by 3wchina01 on 16/6/15.
//  Copyright © 2016年 3wchina01. All rights reserved.
//

#ifndef LHDictionary_h
#define LHDictionary_h

#include <stdio.h>
#include <stdbool.h>

typedef void (*LHDictionaryApplierFunction)(const void *key, const void *value, void *context);

typedef void *(*map_retain_callback)(const void* value);

typedef void (*map_release_callback)(const void* value);

typedef bool (*map_equal_callback)(const void* value1,const void* value2);

typedef u_long (*map_hash_callback)(const void* key);

typedef struct _lh_map_key_callback {
    map_retain_callback retain;
    map_release_callback release;
    map_equal_callback equal;
    map_hash_callback hash;
}map_key_callback;

typedef struct _lh_map_value_callback {
    map_retain_callback retain;
    map_release_callback release;
    map_equal_callback equal;
}map_value_callback;

typedef struct _lh_map_t* LHDictionaryRef;

extern map_key_callback lh_default_key_callback;

extern map_value_callback lh_string_value_callback;

/**
 @dis 创建字典对象 提供默认的capactity,hash函数
 */
LHDictionaryRef lh_dictionary_create();

/**
 @dis 创建字典对象
 @param capacity 预处理的个数  合适的capacity可能提升插入删除的效率
 @param keyCallback 定义字典键的结构体 可以提供hash函数  equal函数及retain和release函数
 @param valueCallback 定义字典值的结构体 equal函数及retain和release函数
 */

LHDictionaryRef lh_dictionary_create_with_options(uint capacity,map_key_callback* keyCallback,map_value_callback* valueCallback);

/**
 @dis 获取字典长度
 */
uint lh_dictionaryGetCount(LHDictionaryRef dictionaryRef);

/**
 @dis 判断字典中是否存在此键 会根据keyCallback中的equal函数判断
 */
bool lh_dictionaryContainKey(LHDictionaryRef dictionaryRef,const void* key);

/**
 @dis  插入键值对
 */
void lh_dictionarySetValueForKey(LHDictionaryRef dictionaryRef,const void* key,const void* value);

/**
 @dis 获取数据
 */
void* lh_dictionaryGetValueForKey(LHDictionaryRef dictionaryRef,const void* key);

/**
 @dis 删除数据
 */
void lh_dictionaryRemoveValueForKey(LHDictionaryRef dictionaryRef,const void* key);

/**
 @dis 获取所有的key  
 @param keys 一个void* 类型的数组   需要足够长度  否则c数组越界会造成不可预估的错误
 */
void lh_dictionaryGetAllKeys(LHDictionaryRef dictionaryRef,const void** keys);

/**
 @dis 获取所有的值
 @param values 存储所有值得数组
 */
void lh_dictionaryGetAllValues(LHDictionaryRef dictionaryRef,const void** values);

/**
 @dis 删除所有数据
 */
void lh_dictionaryRemoveAllValues(LHDictionaryRef dictionary);

/**
 @dis 遍历字典
 @param applier 遍历字典所需要的函数指针 
 @param context 函数指针可以传的参数 可以为空
 */
void lh_dictionaryApplyFunction(LHDictionaryRef dictionary,LHDictionaryApplierFunction applier,void* context);

/**
 @dis 释放字典占用的内存  释放后不可继续使用该字典 否则会造成不可预估的错误
 */
void lh_dictionaryRelease(LHDictionaryRef dictionary);
#endif /* LHDictionary_h */
