/* forking.c: Forking HTTP Server */

#include "spidey.h"

#include <errno.h>
#include <signal.h>
#include <string.h>

#include <unistd.h>

/**
 * Fork incoming HTTP requests to handle the concurrently.
 *
 * @param   sfd         Server socket file descriptor.
 * @return  Exit status of server (EXIT_SUCCESS).
 *
 * The parent should accept a request and then fork off and let the child
 * handle the request.
 **/
int forking_server(int sfd) {
    Request *r;

    /* Accept and handle HTTP request */
    while (true) {
    	/* Accept request */
        r = accept_request(sfd);
        if(!r) {
            return EXIT_FAILURE;
        }

	/* Ignore children */
        signal(SIGCHLD, SIG_IGN);

	/* Fork off child process to handle request */
        pid_t pid = fork();

        if(pid < 0) {
            fprintf(stderr, "Unable to fork %s\n", strerror(errno));
            close(r->fd);
        }
        else if(pid == 0) { // child
            close( r->fd );
            handle_request( r );
            exit(EXIT_SUCCESS);
        }
        else{               
            free_request(r);
        }

            
         }

    /* Close server socket */       
        close( r->fd);

    return EXIT_SUCCESS;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
