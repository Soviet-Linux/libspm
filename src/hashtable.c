#include "stdio.h"
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"
#include "cutils.h"

hashtable* hm_create(int capacity)
{
    if (capacity == 0) capacity = 1024;

    hashtable *hm = calloc(1,sizeof(hashtable));
    hm->items = calloc(capacity, sizeof(item));
    if (hm->items == NULL) {
        fprintf(stderr, "Out of memory");
        exit(1);
    }
    hm->capacity = capacity;
    return hm;
}

void hm_destroy(hashtable *hm)
{   
    for (int i = 0; i < hm->capacity; i++) {
        if (hm->items[i].data != NULL) {
            free(hm->items[i].data);
        }
    }
    free(hm->items);
    free(hm);
}

int hm_add(hashtable *hm, char *key, void *value)
{

    int index = hm_hash(hm,key);
    item x = hm->items[index];
    if (x.data == NULL) {
        x.data = calloc(8, sizeof(pair));
        x.size = 0;
        x.capacity = 8;
    }
    else if (x.size == x.capacity) {
        x.capacity += 8;
        //printf("reallocating %d\n", x.capacity);
        x.data = realloc(x.data, sizeof(pair)*(x.capacity));
    }
    
    pair apair = {key, value};
    x.data[x.size++] = apair;
    hm->items[index] = x;

    return 0;
}

void* hm_get(hashtable *hm, char *key)
{
    int index = hm_hash(hm,key);
    item x = hm->items[index];
    if (x.data == NULL) return NULL;
    for (int i = 0; i < x.size; i++) {
        if (strcmp(x.data[i].key,key) == 0) {
            return x.data[i].value;
        }
    }
    return NULL;
}

// here we are converting a pair list into a hash table
hashtable* hm_init(void* kvlist[][2],int size) {
    hashtable* hm = hm_create(size*2);
    for (int i = 0; i < size; i++) {
        hm_add(hm, kvlist[i][0], kvlist[i][1]);
    }
    return hm;
}

int hm_rm(hashtable *hm, char *key)
{
    int index = hm_hash(hm,key);
    item *x = &hm->items[index];
    if (x->data == NULL) return 1;
    for (int i = 0; i < x->size; i++) {
        if (strcmp(x->data[i].key,key) == 0) {
            x->data[i] = x->data[x->size-1];
            //printf("replacing %s with %s\n", x->data[i].key, x->data[x->size-1].key);
            x->size--;
            if (x->size == 0) {
                //printf("freeing %s\n", x->data[i].key);
                free(x->data);
                x->data = NULL;
            }
            return 0;
        }
    }
    return 1;
}

int hm_visualize(hashtable *hm)
{
    for (int i = 0; i < hm->capacity; i++) {
        item x = hm->items[i];
        if (x.data == NULL) continue;
        printf("===========================\n");
        printf("%d: ", i);
        for (int j = 0; j < x.size; j++) {
            printf("%s ", x.data[j].key);
        }
        printf("\n");
        printf("===========================\n");
    }
    return 0;
}

unsigned int hm_hash(hashtable *hm, char *key)
{
    if (key == NULL) return 0;

    unsigned int len = strlen(key); 

    unsigned char *p = (unsigned char*) key;
    unsigned int h = 0;

    while(len--) {
        h += *p++;
        h += (h << 9);
        h ^= (h >> 19);
    }

    h += (h << 3);
    h ^= (h >> 17);
    h += (h << 11);

    //printf("hash: %d\n", h % hm->capacity);

    return h % hm->capacity;
}