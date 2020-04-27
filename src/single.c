/* single.c: Single User HTTP Server */

#include "spidey.h"

#include <errno.h>
#include <string.h>

#include <unistd.h>

/**
 * Handle one HTTP request at a time.
 *
 * @param   sfd         Server socket file descriptor.
 * @return  Exit status of server (EXIT_SUCCESS).
 **/
int single_server(int sfd) {
    //Status status;
    /* Accept and handle HTTP request */

    while (true) {
    	/* Accept request */
        Request *r = accept_request(sfd);
        if ( !r ) {
            log("Cannot accept request: %s", strerror(errno));
            continue;               // try again if request handling fails
        }

	/* Handle request */
        status = handle_request(r);

	/* Free request */
        //if (status == 0 ){
             free_request(r);
       // }
    }
    printf("exited while loop");

    /* Close server socket */
        close( sfd );
    return EXIT_SUCCESS;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
