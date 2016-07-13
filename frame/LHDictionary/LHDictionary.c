//
//  LHDictionary.c
//  LHDictionary
//
//  Created by 3wchina01 on 16/6/15.
//  Copyright © 2016年 3wchina01. All rights reserved.
//

#include "LHDictionary.h"
#include <string.h>
#include <stdlib.h>

static int maxNodeLength = 9;
static const float _lh_map_resize_factor = 0.75;

struct _lh_map_node {
    void* key;
    void* value;
    struct _lh_map_node* next;
};

typedef struct _lh_map_bucket _lh_map_bucket_t;

struct _lh_map_bucket {
    u_int count;
    struct _lh_map_node* first;
    struct _lh_map_node* tail;
};

struct _lh_map_t {
    u_long count;
    u_long bucket_count;
    u_long resize_count;
    _lh_map_bucket_t** buckets;
    map_key_callback key_callback;
    map_value_callback value_callback;
};

static inline bool map_default_equal_callback(const void* key1,const void* key2)
{
    return strcmp(key1, key2) == 0;
}

static inline u_long map_default_hash_callback(const void* key)
{
    char* c = (char*)key;
    unsigned long ret=0;
    long n;
    unsigned long v;
    int r;
    
    if ((c == NULL) || (*c == '\0'))
        return(ret);
    
    n=0x100;
    while (*c)
    {
        v=n|(*c);
        n+=0x100;
        r= (int)((v>>2)^v)&0x0f;
        ret=(ret<<r)|(ret>>(32-r));
        ret&=0xFFFFFFFFL;
        ret^=v*v;
        c++;
    }
    return((ret>>16)^ret);
}

static const uint64_t _lh_map_size_prime[] = {
    7ULL, // 1<<3 Minimum Capacity
    13ULL, // 1<<4
    31ULL, // 1<<5
    61ULL, // 1<<6
    127ULL, // 1<<7
    251ULL, // 1<<8
    509ULL, // 1<<9
    1021ULL, // 1<<10
    2039ULL, // 1<<11
    4093ULL, // 1<<12
    8191ULL, // 1<<13
    16381ULL, // 1<<14
    32749ULL, // 1<<15
    65521ULL, // 1<<16
    131071ULL, // 1<<17
    262139ULL, // 1<<18
    524287ULL, // 1<<19
    1048573ULL, // 1<<20
    2097143ULL, // 1<<21
    4194301ULL, // 1<<22
    8388593ULL, // 1<<23
    16777213ULL, // 1<<24
    33554393ULL, // 1<<25
    67108859ULL, // 1<<26
    134217689ULL, // 1<<27
    268435399ULL, // 1<<28
    536870909ULL, // 1<<29
    1073741789ULL, // 1<<30
    2147483647ULL, // 1<<31
    4294967291ULL, // 1<<32
    8589934583ULL, // 1<<33
    17179869143ULL, // 1<<34
    34359738337ULL, // 1<<35
    68719476731ULL, // 1<<36
    137438953447ULL, // 1<<37
    274877906899ULL, // 1<<38
    549755813881ULL, // 1<<39
    1099511627689ULL, // 1<<40
    2199023255531ULL, // 1<<41
    4398046511093ULL, // 1<<42
    8796093022151ULL, // 1<<43
    17592186044399ULL, // 1<<44
    35184372088777ULL, // 1<<45
    70368744177643ULL, // 1<<46
    140737488355213ULL, // 1<<47
    281474976710597ULL, // 1<<48
    562949953421231ULL, // 1<<49
    1125899906842597ULL, // 1<<50
    2251799813685119ULL, // 1<<51
    4503599627370449ULL, // 1<<52
    9007199254740881ULL, // 1<<53
    18014398509481951ULL, // 1<<54
    36028797018963913ULL, // 1<<55
    72057594037927931ULL, // 1<<56
    144115188075855859ULL, // 1<<57
    288230376151711717ULL, // 1<<58
    576460752303423433ULL, // 1<<59
    1152921504606846883ULL, // 1<<60
    2305843009213693951ULL, // 1<<61
    4611686018427387847ULL, // 1<<62
    9223372036854775783ULL, // 1<<63
};

map_key_callback lh_default_key_callback = {
    (map_retain_callback)strdup,
    (map_release_callback)free,
    map_default_equal_callback,
    map_default_hash_callback
};

map_value_callback lh_string_value_callback = {
    (map_retain_callback)strdup,
    (map_release_callback)free,
    map_default_equal_callback
};

LHDictionaryRef lh_dictionary_create()
{
    return lh_dictionary_create_with_options(0, NULL, NULL);
}

LHDictionaryRef lh_dictionary_create_with_options(uint capacity,map_key_callback* keyCallback,map_value_callback* valueCallback)
{
    if (capacity<3) {
        capacity = 3;
    }
    struct _lh_map_t* map = (struct _lh_map_t*)calloc(1, sizeof(struct _lh_map_t));
    if (map == NULL) {
        return NULL;
    }
    int index = 0;
    while (_lh_map_size_prime[index] < capacity) {
        index ++;
    }
    u_long bucketCount = _lh_map_size_prime[index];
    map->bucket_count = bucketCount;
    map->resize_count = bucketCount*_lh_map_resize_factor;
    map->count = 0;
    map->buckets = calloc(bucketCount, sizeof(struct _lh_map_bucket*));
    if (map->buckets == NULL) {
        free(map);
        map = NULL;
        return NULL;
    }

    keyCallback ? (map->key_callback = *keyCallback) : (map->key_callback = lh_default_key_callback);
    if (valueCallback) {
        map->value_callback = *valueCallback;
    }
    map->key_callback.hash ? NULL:(map->key_callback.hash = map_default_hash_callback);
    return map;
}

uint lh_dictionaryGetCount(LHDictionaryRef dictionaryRef)
{
    if (dictionaryRef == NULL) {
        return 0;
    }
    return (uint)dictionaryRef->count;
}

bool lh_dictionaryContainKey(LHDictionaryRef dictionaryRef,const void* key)
{
    char* c = (char*)key;
    
    u_long hash_num = (dictionaryRef->key_callback).hash(c) % dictionaryRef->bucket_count;
    
    struct _lh_map_bucket* bucket = dictionaryRef->buckets[hash_num];
    if (bucket->count > 0) {
        return 1;
    }
    return 0;
}

void lh_map_value_set(LHDictionaryRef dictionaryRef,struct _lh_map_bucket* bucket,const void* key,const void* value)
{
    struct _lh_map_node* headerNode = bucket->first;
    while (1) {
        if (headerNode == NULL) {
            struct _lh_map_node* node = calloc(1, sizeof(struct _lh_map_node));
            dictionaryRef->key_callback.retain ? (node->key =(dictionaryRef->key_callback.retain(key))) : (node->key = (char*)key);
            (dictionaryRef->value_callback).retain ? node->value = ((dictionaryRef->value_callback).retain(value)):(node->value = (void*)value);
            bucket->first = node;
            bucket->tail = node;
            bucket->count += 1;
            dictionaryRef->count += 1;
            break;
        }
        if ((dictionaryRef->key_callback).equal(key,headerNode->key)) {
            (dictionaryRef->value_callback).release ? (dictionaryRef->value_callback).release(headerNode->value) : NULL;
            (dictionaryRef->value_callback).retain ? (headerNode->value) = (dictionaryRef->value_callback).retain(value):(headerNode->value = (void*)value);
            break;
        }
        if (headerNode->next == NULL) {
            struct _lh_map_node* node = calloc(1, sizeof(struct _lh_map_node));
            dictionaryRef->key_callback.retain ? (node->key =(dictionaryRef->key_callback.retain(key))) : (node->key = (char*)key);
            (dictionaryRef->value_callback).retain ? node->value = ((dictionaryRef->value_callback).retain(value)):(node->value = (void*)value);
            headerNode->next = node;
            bucket->tail = headerNode->next;
            bucket->count += 1;
            dictionaryRef->count += 1;
            break;
        }
        headerNode = headerNode->next;
    }
}

void lh_dictionaryResizeCount(LHDictionaryRef dictionaryRef,u_long new_count)
{
    _lh_map_bucket_t** new_buckets = calloc(new_count, sizeof(_lh_map_bucket_t*));
    if (new_buckets == NULL) {
        return;
    }
    for (int i=0; i<dictionaryRef->bucket_count; i++) {
        _lh_map_bucket_t* now_bucket = dictionaryRef->buckets[i];
        if (now_bucket) {
            struct _lh_map_node* node = now_bucket->first;
            while (node) {
                struct _lh_map_node* sub_node = calloc(1, sizeof(struct _lh_map_node));
                sub_node->key = node->key;
                sub_node->value = node->value;
                sub_node->next = NULL;
                u_long new_index = (dictionaryRef->key_callback).hash(sub_node->key) % new_count;
                _lh_map_bucket_t* new_bucket = new_buckets[new_index];
                if (new_bucket == NULL) {
                    new_bucket = calloc(1, sizeof(_lh_map_bucket_t));
                    new_bucket->count += 1;
                    new_bucket->first = sub_node;
                    new_bucket->tail = sub_node;
                    new_buckets[new_index] = new_bucket;

                }else {
                    new_bucket->tail->next = sub_node;
                    new_bucket->count += 1;
                    new_bucket->tail = sub_node;
                }
                free(node);
                node = node->next;
            }
            free(now_bucket);
        }
    }
    
    free(dictionaryRef->buckets);
    dictionaryRef->buckets = new_buckets;
    dictionaryRef->bucket_count = new_count;
    dictionaryRef->resize_count = new_count*_lh_map_resize_factor;
}

void lh_dictionarySetValueForKey(LHDictionaryRef dictionaryRef,const void* key,const void* value)
{
    if (dictionaryRef == NULL||value == NULL||key == NULL) {
        return;
    }
    
    char* c = (char*)key;
    u_long hash_num = (dictionaryRef->key_callback).hash(c) % dictionaryRef->bucket_count;
    struct _lh_map_bucket* bucket = dictionaryRef->buckets[hash_num];
    if (bucket == NULL) {
        bucket = calloc(1, sizeof(_lh_map_bucket_t));
        dictionaryRef->buckets[hash_num] = bucket;
        lh_map_value_set(dictionaryRef,bucket,key,value);
        return;
    }
    if (dictionaryRef->count > dictionaryRef->resize_count) {
        int index = 0;
        while (_lh_map_size_prime[index] < dictionaryRef->bucket_count) {
            index++;
        }
        if (index>59) {
            index = 59;
        }
        lh_dictionaryResizeCount(dictionaryRef, _lh_map_size_prime[index+1]);
        u_long hash_num = (dictionaryRef->key_callback).hash(c) % dictionaryRef->bucket_count;
        bucket = dictionaryRef->buckets[hash_num];
        if (bucket == NULL) {
            bucket = calloc(1, sizeof(_lh_map_bucket_t));
            dictionaryRef->buckets[hash_num] = bucket;
        }
    }
    
//    if (bucket->count > maxNodeLength) {
//        int index = 0;
//        while (_lh_map_size_prime[index] < dictionaryRef->bucket_count) {
//            index++;
//        }
//        if (index > 59) {
//            index = 59;
//        }
//        lh_dictionaryResizeCount(dictionaryRef, _lh_map_size_prime[index+1]);
//        
//        u_long hash_num = (dictionaryRef->key_callback).hash(c) % dictionaryRef->bucket_count;
//        bucket = dictionaryRef->buckets[hash_num];
//        if (bucket == NULL) {
//            bucket = calloc(1, sizeof(_lh_map_bucket_t));
//            dictionaryRef->buckets[hash_num] = bucket;
//        }
//        
//    }
    lh_map_value_set(dictionaryRef, bucket, key, value);
}

void* lh_dictionaryGetValueForKey(LHDictionaryRef dictionaryRef,const void* key)
{
    if (dictionaryRef == NULL) {
        return NULL;
    }
    
    char* c = (char*)key;
    u_long hash_num = (dictionaryRef->key_callback).hash(c) % dictionaryRef->bucket_count;
    
    struct _lh_map_bucket* bucket = dictionaryRef->buckets[hash_num];
    if (bucket == NULL) {
        return NULL;
    }
    if (bucket->count == 0) {
        return NULL;
    }
    struct _lh_map_node* node = bucket->first;
    while (1) {
        if (node == NULL) {
            return NULL;
        }
        if ((dictionaryRef->key_callback).equal(c,(char*)node->key)) {
            return node->value;
        }
        node = node->next;
    }
}

void lh_dictionaryRemoveValueForKey(LHDictionaryRef dictionaryRef,const void* key)
{
    if (dictionaryRef == NULL || key == NULL) {
        return;
    }
    u_long hash_num = (dictionaryRef->key_callback).hash(key) % (dictionaryRef->bucket_count);
    _lh_map_bucket_t* bucket = dictionaryRef->buckets[hash_num];
    if (bucket == NULL) {
        return;
    }
    
    struct _lh_map_node* node = bucket->first;
    if (dictionaryRef->key_callback.equal(key,node->key)) {
        !(dictionaryRef->key_callback.release) ? NULL:(dictionaryRef->key_callback.release(node->key));
        if (&(dictionaryRef->value_callback)&&(dictionaryRef->value_callback).release) {
            dictionaryRef->value_callback.release(node->value);
        }
        bucket->first = node->next;
        free(node);
        if (bucket->first == NULL) {
            free(bucket);
            bucket = NULL;
        }else {
            bucket->count -= 1;
        }
        dictionaryRef->count -= 1;
        node = NULL;
        return;
    }
    
    while (node) {
        struct _lh_map_node* next_node = node->next;
        
        if (next_node == NULL) {
            return;
        }else {
            if (dictionaryRef->key_callback.equal(key,next_node->key)) {
                !(dictionaryRef->key_callback.release) ? NULL:(dictionaryRef->key_callback.release(next_node->key));
                if (&(dictionaryRef->value_callback)&&(dictionaryRef->value_callback.release)) {
                    dictionaryRef->value_callback.release(next_node->value);
                }
                node->next = next_node->next;
                if (node->next == NULL) {
                    bucket->tail = node;
                }
                bucket->count -= 1;
                free(next_node);
                next_node = NULL;
                dictionaryRef->count -= 1;
                return;
            }
        }
        node = node->next;
    }
}

void lh_dictionaryGetAllKeys(LHDictionaryRef dictionaryRef,const void** keys)
{
    for (int i=0; i<dictionaryRef->bucket_count; i++) {
        struct _lh_map_bucket* bucket = dictionaryRef->buckets[i];
        if (bucket == NULL) {
            continue;
        }
        struct _lh_map_node* node = bucket->first;
        while (node) {
            *keys = node->key;
            node = node->next;
            keys ++;
        }
    }
}

void lh_dictionaryGetAllValues(LHDictionaryRef dictionaryRef,const void** values)
{
    if (dictionaryRef == NULL || values == NULL) {
        return;
    }
    for (int i=0; i<dictionaryRef->bucket_count; i++) {
        _lh_map_bucket_t* bucket = dictionaryRef->buckets[i];
        if (bucket) {
            struct _lh_map_node* node = bucket->first;
            while (node) {
                *values = node->value;
                values++;
                if (values == NULL) {
                    return;
                }
                node = node->next;
            }
        }
        
    }
}

void lh_dictionaryRemoveAllValues(LHDictionaryRef dictionary)
{
    if (dictionary == NULL) {
        return;
    }
    for (int i=0; i<dictionary->bucket_count; i++) {
        _lh_map_bucket_t* bucket = dictionary->buckets[i];
        if (bucket) {
            struct _lh_map_node* node = bucket->first;
            while (node) {
                !(dictionary->key_callback.release) ? NULL:(dictionary->key_callback.release(node->key));
                if (&(dictionary->value_callback)&&dictionary->value_callback.release) {
                    dictionary->value_callback.release(node->value);
                }
                free(node);
                struct _lh_map_node* now_node = node;
                node = node->next;
                now_node = NULL;
            }
            free(bucket);
            dictionary->buckets[i] = NULL;
            bucket = NULL;
        }
    }
    dictionary->count = 0;
}

void lh_dictionaryApplyFunction(LHDictionaryRef dictionary,LHDictionaryApplierFunction applier,void* context)
{
    if (dictionary == NULL && applier == NULL) {
        return;
    }
    for (int i=0; i<dictionary->bucket_count; i++) {
        _lh_map_bucket_t* bucket = dictionary->buckets[i];
        if (bucket) {
            struct _lh_map_node* node = bucket->first;
            while (node) {
                applier(node->key,node->value,context);
                node = node->next;
            }
        }
    }
}

void lh_dictionaryRelease(LHDictionaryRef dictionary)
{
    lh_dictionaryRemoveAllValues(dictionary);
    free(dictionary->buckets);
    dictionary->buckets = NULL;
    free(dictionary);
    dictionary = NULL;
}
