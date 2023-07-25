#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct {
    const char* key;
    const char* value;
} header_t;

typedef struct {
    char* key;
    char* value;
} param_t;

typedef struct {
    char method[10];
    char* path;
    header_t* headers;
    uint16_t header_count;
    // param_t* params;
    // uint16_t params_count;
    char* body;
} http_request_t;

int parse(char* buffer, http_request_t* req) {
    // Initialise http request struct
    req->header_count = 0;
    req->headers = NULL;

    // Separate body from buffer
    char* body = strstr(buffer, "\r\n\r\n");
    if (body != NULL) {
        body += 4;
    }
    // printf("body: %s\n", body);
    req->body = strdup(body);
    if (req->body == NULL) {
        printf("Failed to allocate memory for body\n");
        return 1;
    }

    // Separate header from buffer
    char* start = buffer;
    char* end = body - 4;
    char* raw_header = (char*) calloc(1, end - start + 1);
    if (raw_header == NULL) {
        printf("Failed to allocate memory for raw header\n");
        return 1;
    }
    memcpy(raw_header, start, end - start + 1);
    // printf("raw header: \n%s\n", raw_header);

    // Separate first line from header
    char* saveptr_1;
    char* first_line = strdup(strtok_r(raw_header, "\r\n", &saveptr_1));

    char* saveptr_2;
    strcpy(req->method, strtok_r(first_line, " ", &saveptr_2));
    req->path = strdup(strtok_r(NULL, " ", &saveptr_2));
    // char* version = strtok_r(first_line, " ", &saveptr_2);
    free(first_line);

    // Create a struct for each header and add them to the req struct
    char *token_tmp;
    for(int i = 0;; i++) {
        // Get a line from header
        token_tmp = strtok_r(NULL, "\r\n", &saveptr_1);
        if (token_tmp == NULL) {
            break;
        }

        // Separate key and value, using strdup to avoid bugs caused by breaking the original string
        char* tmp = strdup(token_tmp);
        char* saveptr_2;
        // using strdup so the strings will still live after we free "tmp"
        header_t header = {
            .key = strdup(strtok_r(tmp, ":", &saveptr_2)),
            // + 1 to remove leading space
            .value = strdup(strtok_r(NULL, ":", &saveptr_2) + 1)
        };
        free(tmp);

        req->header_count++;
        req->headers = (header_t*) realloc(req->headers, req->header_count * sizeof(header_t));
        req->headers[i] = header;
    }

    free(raw_header);

    return 0;
}
