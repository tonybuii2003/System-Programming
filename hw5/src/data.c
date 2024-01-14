#include "csapp.h"
#include "data.h"
#include "transaction.h"
#include "debug.h"
BLOB *blob_create(char *content, size_t size)
{
    if (content == NULL || size == 0)
        return NULL;
    BLOB *bp = Malloc(sizeof(BLOB));
    bp->content = Malloc(size);
    bp->prefix = Malloc(size + 1);
    memcpy(bp->content, content, size);
    memcpy(bp->prefix, bp->content, size);
    bp->prefix[size] = '\0';
    bp->size = size;
    bp->refcnt = 1;
    pthread_mutex_init(&(bp->mutex), NULL);
    debug("Create blob with content %p, size %zu -> %p", &bp->content, bp->size, bp);
    debug("Increase reference count on blob %p [%s] (%d -> %d) %s", bp, bp->content, bp->refcnt - 1, bp->refcnt, "newly created blob");
    return bp;
}
BLOB *blob_ref(BLOB *bp, char *why)
{

    if (bp == NULL)
        return NULL;
    pthread_mutex_lock(&(bp->mutex));
    bp->refcnt++;
    debug("Increase reference count on blob %p [%s] (%d -> %d) %s", bp, bp->content, bp->refcnt - 1, bp->refcnt, why);
    pthread_mutex_unlock(&(bp->mutex));

    return bp;
}
void blob_unref(BLOB *bp, char *why)
{
    if (bp == NULL)
        return;
    pthread_mutex_lock(&(bp->mutex));
    bp->refcnt--;
    debug("Decrease reference count on blob %p [%s] (%d -> %d) %s", bp, bp->content, bp->refcnt + 1, bp->refcnt, why);
    if (bp->refcnt <= 0)
    {
        pthread_mutex_unlock(&(bp->mutex));
        pthread_mutex_destroy(&(bp->mutex));
        Free(bp->content);
        Free(bp->prefix);
        Free(bp);
    }
    else
    {
        pthread_mutex_unlock(&(bp->mutex));
    }
}
int blob_compare(BLOB *bp1, BLOB *bp2)
{
    if (bp1->size != bp2->size)
        return -1;
    return memcmp(bp1->content, bp2->content, bp1->size);
}
int blob_hash(BLOB *bp)
{
    unsigned long hash = 2003;
    char *tmp = bp->content;
    for (size_t i = 0; i < bp->size; i++)
    {
        hash = (hash * 5 + hash) + tmp[i];
        hash %= 150239;
    }
    return hash;
}
KEY *key_create(BLOB *bp)
{
    if (bp == NULL)
        return NULL;
    KEY *key = Malloc(sizeof(KEY));
    key->blob = bp;
    key->hash = blob_hash(bp);
    debug("Create key from blob %p -> %p [%s]", bp, bp + key->hash, bp->content);
    return key;
}
void key_dispose(KEY *kp)
{
    if (kp == NULL)
        return;
    blob_unref(kp->blob, "for blob in key");
    debug("Dispose of key %p [%s]", kp, kp->blob->content);
    Free(kp);
}
int key_compare(KEY *kp1, KEY *kp2)
{
    return blob_compare(kp1->blob, kp2->blob);
}
VERSION *version_create(TRANSACTION *tp, BLOB *bp)
{
    if (tp == NULL || bp == NULL)
        return NULL;
    VERSION *vp = Malloc(sizeof(VERSION));
    vp->creator = tp;
    vp->blob = bp;
    vp->next = NULL;
    trans_ref(tp, "as creator of version");
    debug("Create version of blob %p for transaction %p -> %p [%s]", bp, tp, vp, bp->content);
    debug("Increase reference count on blob %p [%s] (%d -> %d) %s", bp, bp->content, bp->refcnt - 1, bp->refcnt, "as creator of version");
    return vp;
}
void version_dispose(VERSION *vp)
{
    debug("Dispose of version %p", vp);
    if (vp == NULL)
        return;
    trans_unref(vp->creator, "as creator of version");
    if (vp->blob == NULL)
        return;
    blob_unref(vp->blob, "as creator of version");

    Free(vp);
}