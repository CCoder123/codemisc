#include <stdio.h>
#include "adlist.h"

struct Data{
    int a;
    int b;
    int c;
};

int DataMatch(void* key1, void* key2)
{
    printf("DataMatch......\n");
    if(!key1 || !key2) return 0;

    if(((struct Data*)key1)->a == ((struct Data*)key2)->a) return 1;

    return 0;
}

void DataFree(void *ptr)
{
    if(ptr){
        printf("DataFree\n");
        zfree(ptr);
    }
}

int main()
{
    list *t = listCreate();
    listSetMatchMethod(t, DataMatch); 
    listSetFreeMethod(t, DataFree);
   
    struct Data *d = zmalloc(sizeof(*d));
    if(d == NULL) listRelease(t);
    d->a = 1;
    d->b = 2;
    d->c = 3;
    listAddNodeHead(t, d);
    
    d = zmalloc(sizeof(*d));
    if(d == NULL) listRelease(t);
    d->a = 100;
    d->b = 200;
    d->c = 300;
    t = listAddNodeHead(t, d);
    
    listIter *iter = listGetIterator(t, AL_START_HEAD);
    listNode *node = NULL;
    while((node = listNext(iter)))
    {
        struct Data *b = (struct Data*)node->value;
        printf("a:%d   b:%d   c:%d\n", b->a, b->b, b->c);
    }
    listReleaseIterator(iter);

    node = listSearchKey(t, d);
    if(node)
    {
        struct Data *b = (struct Data*)node->value;
        printf("a:%d   b:%d   c:%d\n", b->a, b->b, b->c);
    }
    else
    {
        printf("Not find\n");
    }
    listDelNode(t, node); 
    
    node = listSearchKey(t, d);
    if(node)
    {
        struct Data *b = (struct Data*)node->value;
        printf("a:%d   b:%d   c:%d\n", b->a, b->b, b->c);
    }
    else
    {
        printf("Not find\n");
    }

    listRelease(t);

    return 0;
}
