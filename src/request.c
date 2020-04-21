/* request.c: HTTP Request Functions */

#include "spidey.h"

#include <errno.h>
#include <string.h>

#include <unistd.h>

int parse_request_method(Request *r);
int parse_request_headers(Request *r);

/**
 * Accept request from server socket.
 *
 * @param   sfd         Server socket file descriptor.
 * @return  Newly allocated Request structure.
 *
 * This function does the following:
 *
 *  1. Allocates a request struct initialized to 0.
 *  2. Initializes the headers list in the request struct.
 *  3. Accepts a client connection from the server socket.
 *  4. Looks up the client information and stores it in the request struct.
 *  5. Opens the client socket stream for the request struct.
 *  6. Returns the request struct.
 *
 * The returned request struct must be deallocated using free_request.
 **/
Request * accept_request(int sfd) {
    Request *r;
    struct sockaddr raddr;
    socklen_t rlen;

    /* Allocate request struct (zeroed) */
    Request * r = calloc( 1, sizeof(Request));
        r->headers = NULL;
   /* if ( parse_request_headers( r ) < 0 ) {
       debug( "Unable to parse headers");
       return EXIT_FAILURE;
        } */

    /* Accept a client */
    FILE * client_file = accept_client( sfd);

    /* Lookup client information */
    memset(&raddr, 0, sizeof(raddr));
                                        // ASK BUI: do we care about the service or just host?
    int client_num = getnameinfo( &raddr, rlen, r->host, NI_MAXHOST, NULL, 0, NI_NAMEREQD);
    if (client_num) {
        debug( "Couldn't get client name");
        EXIT_FAILURE;
        }

    /* Open socket stream */
    FILE * newconnection = fdopen( r->fd, "w+");
    if ( !newconnection ) {
        fprintf( stderr, "Unable to fdopen: %s\n", strerr(errno));
        close( r->fd);
        } 

    log("Accepted request from %s:%s", r->host, r->port);
    return r;

fail:
    /* Deallocate request struct */
    free_request( r);
    return NULL;
}

/**
 * Deallocate request struct.
 *
 * @param   r           Request structure.
 *
 * This function does the following:
 *
 *  1. Closes the request socket stream or file descriptor.
 *  2. Frees all allocated strings in request struct.
 *  3. Frees all of the headers (including any allocated fields).
 *  4. Frees request struct.
 **/
void free_request(Request *r) {
    if (!r) {
    	return;
    }

    /* Close socket or fd */
    close ( r->rd);
    /* Free allocated strings */
    if ( stream) {      free(stream);  }
    if ( method) {      free(method);  }
    if ( uri ) {        free(uri);  }
    if ( path) {        free(path); }
    if (query) {        free(query); }

    /* Free headers */
   /*  Header * curr = r->headers.name;
        while( curr->next) {
            curr = curr->next;
            free(curr);
        }
        /*
  
    /* Free request */
    free(r);
}

/**
 * Parse HTTP Request.
 *
 * @param   r           Request structure.
 * @return  -1 on error and 0 on success.
 *
 * This function first parses the request method, any query, and then the
 * headers, returning 0 on success, and -1 on error.
 **/
int parse_request(Request *r) {
    /* Parse HTTP Request Method */
    int http_met = parse_request_method( r );
    if ( http_met < 0){
        EXIT_FAILURE;
        }

    /* Parse HTTP Requet Headers*/
    int http_head = parse_request_headers( r );
    if ( http_head < 0 ) {
        EXIT_FAILURE;
        }

    return 0;
}

/**
 * Parse HTTP Request Method and URI.
 *
 * @param   r           Request structure.
 * @return  -1 on error and 0 on success.
 *
 * HTTP Requests come in the form
 *
 *  <METHOD> <URI>[QUERY] HTTP/<VERSION>
 *
 * Examples:
 *
 *  GET / HTTP/1.1
 *  GET /cgi.script?q=foo HTTP/1.0
 *
 * This function extracts the method, uri, and query (if it exists).
 **/
int parse_request_method(Request *r) {
    char buffer[BUFSIZ];
    char *method;
    char *uri;
    char *query;

    /* Read line from socket */
    ssize_t line1 = read( r->fd, buffer, BUFSIZ);
    if ( line1 < 0 ) {  
        return -1;
        }
    /* Parse method and uri */          // NEED TO FINISH
    method = skip_whitespace(buffer);
   // method = 
    r->method = strdup(method);

    /* Parse query from uri */

        
    /* Record method, uri, and query in request struct */
    debug("HTTP METHOD: %s", r->method);
    debug("HTTP URI:    %s", r->uri);
    debug("HTTP QUERY:  %s", r->query);

    return 0;

fail:
    return -1;
}

/**
 * Parse HTTP Request Headers.
 *
 * @param   r           Request structure.
 * @return  -1 on error and 0 on success.
 *
 * HTTP Headers come in the form:
 *
 *  <NAME>: <DATA>
 *
 * Example:
 *
 *  Host: localhost:8888
 *  User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:29.0) Gecko/20100101 Firefox/29.0
 *  Accept: text/html,application/xhtml+xml
 *  Accept-Language: en-US,en;q=0.5
 *  Accept-Encoding: gzip, deflate
 *  Connection: keep-alive
 *
 * This function parses the stream from the request socket using the following
 * pseudo-code:
 *
 *  while (buffer = read_from_socket() and buffer is not empty):
 *      name, data  = buffer.split(':')
 *      header      = new Header(name, data)
 *      headers.append(header)
 **/
int parse_request_headers(Request *r) {
    Header *curr = NULL;
    char buffer[BUFSIZ];
    char *name;
    char *data;

    /* Parse headers from socket */

#ifndef NDEBUG
    for (Header *header = r->headers; header; header = header->next) {
    	debug("HTTP HEADER %s = %s", header->name, header->data);
    }
#endif
    return 0;

fail:
    return -1;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
