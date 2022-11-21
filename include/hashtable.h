typedef struct pair {
    char* key;
    void* value;
} pair;

typedef struct {
    pair* data;
    int size;
    int capacity;
} item;

typedef struct {
    item *items;
    int capacity;
} hashtable;


hashtable *hm_create(int capacity);
void hm_destroy(hashtable *hm);
int hm_add(hashtable *hm, char *key, void *value);
void* hm_get(hashtable *hm, char *key);
int hm_rm(hashtable *hm, char *key);
int hm_visualize(hashtable *hm);
hashtable* hm_init(void* kvlist[][2],int size);

unsigned int hm_hash(hashtable *hm, char *key);;