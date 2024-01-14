#include "debug.h"
#include "client_registry.h"
#include "transaction.h"
#include "store.h"
#include "global.h"
#include <unistd.h>
#include <sys/socket.h>
#include "server.h"

static void terminate(int status);
CLIENT_REGISTRY *client_registry;
int main(int argc, char *argv[])
{
    if (argc <= 1)
    {
        exit(EXIT_SUCCESS);
    }
    struct sigaction action_sighup;
    memset(&action_sighup, 0, sizeof(action_sighup));
    action_sighup.sa_handler = sighup_signal_handler;
    if (sigaction(SIGHUP, &action_sighup, NULL) == -1)
    {
        exit(EXIT_FAILURE);
    }

    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.
    char *port;
    int opt;
    while ((opt = getopt(argc, argv, "p:")) != -1)
    {
        switch (opt)
        {
        case 'p':
            port = optarg;
            break;
        default:
            fprintf(stderr, "Invalid option: %s\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    if (port <= 0)
    {
        fprintf(stderr, "Invalid port number.\n");
        exit(EXIT_FAILURE);
    }
    // Perform required initializations of the client_registry,
    // transaction manager, and object store.
    client_registry = creg_init();
    trans_init();
    store_init();

    // TODO: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function xacto_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.
    int listenfd = Open_listenfd(port);
    int *connfd;
    pthread_t tid;
    while (!flag_sighup)
    {
        struct sockaddr_in clientaddr;
        socklen_t clientaddrlen = sizeof(clientaddr);
        memset(&clientaddr, 0, sizeof(clientaddr));

        connfd = malloc(sizeof(int));
        *connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientaddrlen);
        if (*connfd == -1)
        {
            free(connfd);
            continue;
        }
        pthread_create(&tid, NULL, xacto_client_service, connfd);
        // pthread_create(&tid2, NULL, xacto_client_service, connfd);
    }
    terminate(EXIT_SUCCESS);
    fprintf(stderr, "You have to finish implementing main() "
                    "before the Xacto server will function.\n");
    terminate(EXIT_FAILURE);
}

/*
 * Function called to cleanly shut down the server.
 */
void terminate(int status)
{
    // Shutdown all client connections.
    // This will trigger the eventual termination of service threads.
    creg_shutdown_all(client_registry);

    debug("Waiting for service threads to terminate...");
    creg_wait_for_empty(client_registry);
    debug("All service threads terminated.");

    // Finalize modules.
    creg_fini(client_registry);
    trans_fini();
    store_fini();

    debug("Xacto server terminating");
    exit(status);
}
