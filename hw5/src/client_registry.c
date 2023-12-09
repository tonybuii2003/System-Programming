#include "client_registry.h"
#include "csapp.h"
#include "debug.h"

typedef struct client_registry
{
    int client_count;
    int *client_fds;
    pthread_mutex_t mutex;
    sem_t semaphore;
} CLIENT_REGISTRY;

CLIENT_REGISTRY *creg_init()
{
    CLIENT_REGISTRY *cr = Malloc(sizeof(CLIENT_REGISTRY));
    if (cr == NULL)
    {
        return NULL;
    }
    cr->client_count = 0;
    cr->client_fds = Malloc(sizeof(int) * FD_SETSIZE);
    if (cr->client_fds == NULL)
    {
        Free(cr);
        return NULL;
    }
    for (int i = 0; i < FD_SETSIZE; i++)
    {
        cr->client_fds[i] = -1;
    }
    pthread_mutex_init(&cr->mutex, NULL);
    sem_init(&cr->semaphore, 0, 0);
    debug("Initialize client registry");
    return cr;
}
void creg_fini(CLIENT_REGISTRY *cr)
{
    pthread_mutex_destroy(&cr->mutex);
    sem_destroy(&cr->semaphore);
    Free(cr->client_fds);
    Free(cr);
    debug("Finalize client registry");
}
int creg_register(CLIENT_REGISTRY *cr, int fd)
{
    pthread_mutex_lock(&cr->mutex);
    if (cr->client_count == FD_SETSIZE)
    {
        pthread_mutex_unlock(&cr->mutex);
        return -1;
    }
    for (int i = 0; i < FD_SETSIZE; i++)
    {
        if (cr->client_fds[i] == -1)
        {
            cr->client_fds[i] = fd;
            cr->client_count++;
            break;
        }
    }
    pthread_mutex_unlock(&cr->mutex);
    debug("Register client fd %d (total connected: %d)", fd, cr->client_count);
    return 0;
}
int creg_unregister(CLIENT_REGISTRY *cr, int fd)
{
    pthread_mutex_lock(&cr->mutex);
    for (int i = 0; i < FD_SETSIZE; i++)
    {
        if (cr->client_fds[i] == fd)
        {
            cr->client_fds[i] = -1;
            cr->client_count--;
            if (cr->client_count == 0)
            {
                V(&cr->semaphore);
            }
            break;
        }
    }
    pthread_mutex_unlock(&cr->mutex);
    debug("Unregister client fd %d (total connected: %d)", fd, cr->client_count);
    return 0;
}
void creg_wait_for_empty(CLIENT_REGISTRY *cr)
{
    P(&cr->semaphore);
    Sem_init(&cr->semaphore, 0, 0);
}
void creg_shutdown_all(CLIENT_REGISTRY *cr)
{
    if (cr == NULL)
    {
        return;
    }
    pthread_mutex_lock(&cr->mutex);
    for (int i = 0; i < FD_SETSIZE; i++)
    {
        if (cr->client_fds[i] != -1)
        {
            shutdown(cr->client_fds[i], SHUT_RDWR);
        }
    }
    pthread_mutex_unlock(&cr->mutex);
}
