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
    
    struct sockaddr raddr;
    socklen_t rlen = sizeof(struct sockaddr);

    /* Allocate request struct (zeroed) */
    Request *r = calloc( 1, sizeof(Request));
    if ( !r) {
        debug("Cannot allocate request: %s", strerror(errno));
        return NULL;
        }

    /* Accept a client */
    r->fd = accept( sfd, &raddr, &rlen);
    if ( r->fd < 0) {
        debug("Unable to accept: %s", strerror(errno));
        goto fail;
    }

    /* Lookup client information */
                                        
    int client_stat = getnameinfo( &raddr, rlen, r->host, sizeof(r->host), r->port, sizeof(r->port), NI_NUMERICHOST | NI_NUMERICSERV);

    if (client_stat) {
        debug( "Couldn't get client name %s", gai_strerror( client_stat));
        goto fail;
        }

    /* Open socket stream */
    r->stream = fdopen( r->fd, "w+");
    if ( !r->stream ) {
        debug( "Unable to fdopen: %s", strerror(errno));
        goto fail;
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
    close ( r->fd);
    /* Free allocated strings */
    if ( r->stream) { fclose(r->stream); }
    if ( r->method) {  free( r->method);  }
    if ( r->uri ) {  free(r->uri);  }
    if ( r->path) {  free( r->path); }
    if (r->query) {  free(r->query); }

    /* Free headers */
    Header *curr = r->headers;
    Header *temp;
    
    while(curr) {
        temp = curr->next;
        free(curr->name);
        free(curr->data);
        free(curr);
        curr = temp;
    }

    free(temp);
  
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
        debug("Unable to parse request method: %s", strerror(errno));
        return -1;
        }

    /* Parse HTTP Requet Headers*/
    int http_head = parse_request_headers( r );
    if ( http_head < 0 ) {
        debug("Unable to parse request headers: %s", strerror(errno));
        return -1;
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

    /* Read line from socket */
    if ( !fgets( buffer, BUFSIZ, r->stream)) {
        return HTTP_STATUS_BAD_REQUEST;
        }    

    /* Parse method and uri */      
    method = strtok( buffer, WHITESPACE);
    uri = strtok( NULL, WHITESPACE);

    if ( !method || !uri ) {
        return HTTP_STATUS_BAD_REQUEST;
        }

    /* Parse query from uri */
    char *query = strchr(uri, '?');
    if (!query) {
        query = "";
    }
    else{               // advance past the ? character
        *query = '\0';
        query++;
    }

    r->method = strdup(method);
    r->uri    = strdup(uri);
    r->query  = strdup(query);
    /* Record method, uri, and query in request struct */
    debug("HTTP METHOD: %s", r->method);
    debug("HTTP URI:    %s", r->uri);
    debug("HTTP QUERY:  %s", r->query);

    return 0;

//fail:
  //  return -1;
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

          //  Add a '\r\n' to the end of headers?????
        debug("headers??");

    if(!r->stream) {
        goto fail;
    }

    while( fgets( buffer, BUFSIZ, r->stream) && strlen(buffer) > 2){
        data = strchr(buffer, ':');
        //name = strtok(buffer, WHITESPACE);
        //data = strchr(buffer, ':');

        if(!data) {
            goto fail;
        }

        *data = '\0';
        data++;
        data = skip_whitespace(data);
        chomp(data);

        name = strdup(buffer);
        if(!name) {
            goto fail;
        }

        Header *newHeader = calloc(1, sizeof(Header));
        if(!newHeader) {
            fprintf(stderr, "error calloc newHeader: %s\n", strerror(errno));
            goto fail;
        }
        
        newHeader->name = name; //strdup( name );
        newHeader->data = strdup( data );

        if(!newHeader->data) {
            goto fail;
        }

        if(!r->headers) {
            r->headers = newHeader;
        }
        else {
            curr->next = newHeader;
        }

        //curr->next = newHeader;         // add it to the back
        curr = newHeader;
        curr->next = NULL;

        debug("Header: %s  %s", newHeader->name, newHeader->data);
        
    }


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
