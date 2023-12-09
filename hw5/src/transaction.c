#include "csapp.h"
#include "transaction.h"
TRANSACTION *the_transaction;
void trans_init(void)
{
    the_transaction = Malloc(sizeof(TRANSACTION));
    the_transaction->id = 0;
    the_transaction->refcnt = 1;
    the_transaction->status = TRANS_PENDING;
    the_transaction->depends = NULL;
    the_transaction->waitcnt = 0;
    pthread_mutex_init(&the_transaction->mutex, NULL);
    sem_init(&the_transaction->sem, 0, 0);
}
void trans_fini(void)
{
    pthread_mutex_destroy(&the_transaction->mutex);
    sem_destroy(&the_transaction->sem);
    Free(the_transaction);
}
TRANSACTION *trans_create(void)
{
    trans_init();
    return the_transaction;
}
TRANSACTION *trans_ref(TRANSACTION *tp, char *why)
{

    if (tp == NULL)
        return NULL;

    pthread_mutex_lock(&(tp->mutex));
    tp->refcnt++;
    pthread_mutex_unlock(&(tp->mutex));
    return tp;
}
void trans_unref(TRANSACTION *tp, char *why)
{
    if (tp == NULL)
        return;
    pthread_mutex_lock(&(tp->mutex));
    tp->refcnt--;
    pthread_mutex_unlock(&tp->mutex);
    if (tp->refcnt <= 0)
    {
        trans_fini();
    }
}
void trans_add_dependency(TRANSACTION *tp, TRANSACTION *dtp)
{
    pthread_mutex_lock(&(tp->mutex));
    DEPENDENCY *dep = Malloc(sizeof(DEPENDENCY));
    if (dep == NULL)
    {
        pthread_mutex_unlock(&tp->mutex);
        return;
    }
    dep->trans = dtp;
    dep->next = tp->depends;
    tp->depends = dep;
    pthread_mutex_unlock(&tp->mutex);
    trans_ref(dtp, "transaction in dependency");
}
TRANS_STATUS trans_abort(TRANSACTION *tp)
{
    pthread_mutex_lock(&tp->mutex);
    tp->status = TRANS_ABORTED;
    pthread_mutex_unlock(&tp->mutex);
    trans_unref(tp, "for attempting to commit transaction");
    return tp->status;
}
TRANS_STATUS trans_commit(TRANSACTION *tp)
{
    pthread_mutex_lock(&tp->mutex);
    DEPENDENCY *dep = tp->depends;
    while (dep != NULL)
    {
        P(&dep->trans->sem);
        if (trans_get_status(dep->trans) == TRANS_ABORTED)
        {
            pthread_mutex_unlock(&tp->mutex);
            return trans_abort(tp);
        }
        dep = dep->next;
    }

    tp->status = TRANS_COMMITTED;
    pthread_mutex_unlock(&tp->mutex);
    V(&tp->sem);
    trans_unref(tp, "attempting to commit transaction");
    return tp->status;
}
TRANS_STATUS trans_get_status(TRANSACTION *tp)
{
    pthread_mutex_lock(&tp->mutex);
    TRANS_STATUS status = tp->status;
    pthread_mutex_unlock(&tp->mutex);
    return status;
}
void trans_show(TRANSACTION *tp)
{
}

void trans_show_all(void)
{
}