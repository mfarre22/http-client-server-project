#include<stdio.h>
#include<string.h>



int main() {

    char line1[50] = "GET /script.cgi?q=monkeys HTTP/1.0";
    char line2[50] = "Host: xavier.h4x0r.space:9898";
    char * name, uri, query, data, method;
    
    printf("Original strings:\n  %s\n%s\n", line1, line2);

    name = strtok( line2, whitepace);
    data = strtok( line2, ':');

    method = strtok( line1, WHITESPACE);
    uri = strtok( NULL, WHITESPACE);
    query = strchr( uri, '?');

    printf("Method: %s\n", method);
    printf("Name: %s\n", name);
    printf("query: %s\n", query);
    printf("Data: %s\n", data);
    printf("uri: %s\n", uri);

    return 0;
}
