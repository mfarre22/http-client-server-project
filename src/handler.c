/* handler.c: HTTP Request Handlers */

#include "spidey.h"

#include <errno.h>
#include <limits.h>
#include <string.h>

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

/* Internal Declarations */
Status handle_browse_request(Request *request);
Status handle_file_request(Request *request);
Status handle_cgi_request(Request *request);
Status handle_error(Request *request, Status status);

/**
 * Handle HTTP Request.
 *
 * @param   r           HTTP Request structure
 * @return  Status of the HTTP request.
 *
 * This parses a request, determines the request path, determines the request
 * type, and then dispatches to the appropriate handler type.
 *
 * On error, handle_error should be used with an appropriate HTTP status code.
 **/
Status  handle_request(Request *r) {
    Status result = HTTP_STATUS_OK;
    struct stat s;

    /* Parse request */
    int request_stat = parse_request(r);
    if (request_stat < 0) {
        result = handle_error(r, HTTP_STATUS_BAD_REQUEST);
        return result;
    }
    
    /* Determine request path */
    char * path = determine_request_path(r->uri);
    r->path = path;

    if (!r->path) {
        result = handle_error(r, HTTP_STATUS_BAD_REQUEST);
        return result;
    }

    debug("HTTP REQUEST PATH: %s", r->path);

    /* Dispatch to appropriate request handler type based on file type */ 
    debug("r->path is: %s", r->path); 
    if(stat(r->path, &s) < 0) {
       fprintf(stderr, "stat failed: %s\n", strerror(errno));
       result = handle_error(r, HTTP_STATUS_NOT_FOUND);
       return result;
    }

    else if (stat (r->path, &s) == 0) {   
         if (S_ISDIR(s.st_mode)) {
                result = handle_browse_request(r);
                return result;
         }
    }

    if (access(r->path, X_OK) == 0) { 
        result = handle_cgi_request(r);
    }

    else if (access(r->path, R_OK) == 0) {
        result = handle_file_request(r);
    }

    log("HTTP REQUEST STATUS: %s", http_status_string(result));

    return result;
}

/**
 * Handle browse request.
 *
 * @param   r           HTTP Request structure.
 * @return  Status of the HTTP browse request.
 *
 * This lists the contents of a directory in HTML.
 *
 * If the path cannot be opened or scanned as a directory, then handle error
 * with HTTP_STATUS_NOT_FOUND.
 **/
Status  handle_browse_request(Request *r) {
    struct dirent **entries;          

    /* Open a directory for reading or scanning */
    int n = scandir(r->path, &entries, 0, alphasort);
    if(n < 0) {
        debug("scandir failed: %s\n", strerror(errno));
        return HTTP_STATUS_NOT_FOUND;
    }
        
    /* Write HTTP Header with OK Status and text/html Content-Type */
    fprintf(r->stream, "HTTP/1.0 200 OK\r\n");
    fprintf(r->stream, "Content-Type: text/html\r\n");
    fprintf(r->stream, "\r\n");

    /* For each entry in directory, emit HTML list item */
    fprintf(r->stream, "<ul>\n");

    for(int i = 0; i < n ; i++) {
        if(streq(entries[i]->d_name, ".")) {
            free(entries[i]);
            continue;
        }

        /* Format differently if uri is "/" */
        if(streq(r->uri, "/")) {
            fprintf(r->stream, "<li> <a href=\"/%s\"> %s </a> </li>\n", entries[i]->d_name, entries[i]->d_name);
        }
    
        else {
            fprintf(r->stream, "<li> <a href=\"%s/%s\"> %s </a> </li>\n", r->uri, entries[i]->d_name, entries[i]->d_name);
        }

        free(entries[i]);
    }

    fprintf(r->stream, "</ul>\n");

    free(entries);

    /* Return OK */
    return HTTP_STATUS_OK;
}

/**
 * Handle file request.
 *
 * @param   r           HTTP Request structure.
 * @return  Status of the HTTP file request.
 *
 * This opens and streams the contents of the specified file to the socket.
 *
 * If the path cannot be opened for reading, then handle error with
 * HTTP_STATUS_NOT_FOUND.
 **/
Status  handle_file_request(Request *r) {

    debug("handle_file_request");

    FILE *fs;               
    char buffer[BUFSIZ];
    char *mimetype = NULL;
    size_t nread;
    Status status;

    /* Open file for reading */
    debug("about to open file");
    fs = fopen(r->path, "r");
    if(!fs) {
        fprintf(stderr, "error opening file: %s\n", strerror(errno));
        status = handle_error(r, HTTP_STATUS_NOT_FOUND);
        return status;
    }

    /* Determine mimetype */
    mimetype = determine_mimetype( r->path );
    debug("mimetype: %s", mimetype);

    /* Write HTTP Headers with OK status and determined Content-Type */
    fprintf(r->stream, "HTTP/1.0 %s\r\n", http_status_string(HTTP_STATUS_OK));
    fprintf(r->stream, "Content-Type: %s\r\n", mimetype);
    fprintf(r->stream, "\r\n");

    /* Read from file and write to socket in chunks */
    nread = fread(buffer, 1, BUFSIZ, fs);
    debug("about to read from file");
    while(nread > 0) {
        fwrite(buffer, 1, nread, r->stream);
        nread = fread(buffer, 1, BUFSIZ, fs);
    }

    debug("Done reading from file");

    /* Close file, deallocate mimetype, return OK */
    fclose(fs);
    if(mimetype) {
        free(mimetype);
    }

    return HTTP_STATUS_OK;

    /* Close file, free mimetype, return INTERNAL_SERVER_ERROR */
    return HTTP_STATUS_INTERNAL_SERVER_ERROR;
}

/**
 * Handle CGI request
 *
 * @param   r           HTTP Request structure.
 * @return  Status of the HTTP file request.
 *
 * This popens and streams the results of the specified executables to the
 * socket.
 *
 * If the path cannot be popened, then handle error with
 * HTTP_STATUS_INTERNAL_SERVER_ERROR.
 **/
Status  handle_cgi_request(Request *r) {
    Status status;

    /* Export CGI environment variables from request:
     * http://en.wikipedia.org/wiki/Common_Gateway_Interface */
    if(RootPath) {
        setenv("DOCUMENT_ROOT", RootPath, 1);
    }
    if(r->query) {
        setenv("QUERY_STRING", r->query, 1);
    }
    if(r->host) {
        setenv("REMOTE_ADDR", r->host, 1);
    }
    if(r->port) {
        setenv("REMOTE_PORT", r->port, 1);
    }
    if(r->method) {
        setenv("REQUEST_METHOD", r->method, 1);
    }
    if(r->uri) {
        setenv("REQUEST_URI", r->uri, 1);
    }
    if(r->path) {
        setenv("SCRIPT_FILENAME", r->path, 1);
    }
    if(Port) {
        setenv("SERVER_PORT", Port, 1);
    }

    /* Export CGI environment variables from request headers */ 
    for(Header *header = r->headers; header; header = header->next) {
        if(streq(header->name, "Host")) {
            setenv("HTTP_HOST", header->name, 1);
        }
        else if(streq(header->name, "Accept")) {
            setenv("HTTP_ACCEPT", header->name, 1);
        }
        else if(streq(header->name, "Accept-Language")) {
            setenv("HTTP_ACCEPT_LANGUAGE", header->name, 1);
        }
        else if(streq(header->name, "Accept-Encoding")) {
            setenv("HTTP_ACCEPT_ENCODING", header->name, 1);
        }
        else if(streq(header->name, "Connection")) {
            setenv("HTTP_CONNECTION", header->name, 1);
        }
        else if(streq(header->name, "User-Agent")) {
            setenv("HTTP_USER_AGENT", header->name, 1);
        }
    }

    /* POpen CGI Script */
    FILE *process_stream = popen(r->path, "r");
    if(!process_stream) {
        debug("error opening path with popen: %s\n", strerror(errno));
        status = handle_error(r, HTTP_STATUS_INTERNAL_SERVER_ERROR);
        return status;
    }

    /* Copy data from popen to socket */
    char buffer[BUFSIZ];
    size_t nread = fread(buffer, 1, BUFSIZ, process_stream);

    debug("CGI: reading from process stream\n");
    while(nread > 0) {
        fwrite(buffer, 1, nread, r->stream);
        nread = fread(buffer, 1, BUFSIZ, process_stream);
    }

    /* Close popen, return OK */
    pclose(process_stream);

    return HTTP_STATUS_OK;
}

/**
 * Handle displaying error page
 *
 * @param   r           HTTP Request structure.
 * @return  Status of the HTTP error request.
 *
 * This writes an HTTP status error code and then generates an HTML message to
 * notify the user of the error.
 **/
Status  handle_error(Request *r, Status status) {
    const char *status_string = http_status_string(status);

    /* Write HTTP Header */  
    debug("Handling error\n");

    fprintf(r->stream, "HTTP/1.0 %s\r\n", status_string);
    fprintf(r->stream, "Content-Type: text/html\r\n"); 
    fprintf(r->stream, "\r\n");

    /* Write HTML Description of Error*/
    fprintf(r->stream, " <h1>%s </h1>\r\n", status_string);
    fprintf(r->stream, "Something bad has happened. You're really screwed this time </body> \r\n"); 
        
    /* Return specified status */
    return status;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
