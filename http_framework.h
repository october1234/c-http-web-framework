#include "http_parser.h"
#include <stdio.h>
#include <sys/socket.h>

#define BUFFSIZE 65535
#define CALLBACK_FUNCTION_TYPE void (*callback)(const http_request_t *req, http_response_t *res)

int PORT = 8080;

typedef struct {
    int code;
    char* body;
} http_response_t;

typedef struct {
    char* route;
    CALLBACK_FUNCTION_TYPE;
} route_t;

route_t *routes = NULL;
unsigned int route_count = 0;

/*
    TODO:
    1. Parse params
    2. Make hashmap or tree for storing route callback functions
    3. Create an http response struct for responding different data
*/

void add_route(char *route, CALLBACK_FUNCTION_TYPE) {
    route_count++;
    routes = realloc(routes, route_count * sizeof(route_t));
    routes[route_count - 1].route = route;
    routes[route_count - 1].callback = callback;
}

int fw_start(int port) {
    PORT = port;
    // Initializing server socket
    int socket_fd;
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("failed to create socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in address = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(PORT)
    };
    socklen_t addresslen = sizeof(address);
    if (bind(socket_fd, &address, addresslen) < 0) {
        perror("failed to bind socket");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(socket_fd, 3) < 0) {
        perror("failed to listen");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    // Create buffer for incoming connections
    char buffer[BUFFSIZE] = "";

    for (;;) {
        // Accept new socket connection
        int new_socket;
        if ((new_socket = accept(socket_fd, (struct sockaddr*)&address, (socklen_t*) &addresslen)) < 0) {
            perror("failed to accept socket");
            exit(EXIT_FAILURE);
        }

        // Read from the socket
        int valread = read(new_socket, buffer, BUFFSIZE);

        // // Displaying the buffer for debugging
        // printf("%s\n", buffer);

        // Initialize some variables
        http_request_t *req = malloc(sizeof(http_request_t));

        // Parsing the request
        parse(buffer, req);

        http_response_t *res = malloc(sizeof(http_response_t));

        int found_route = 0;
        for (int i = 0; i < route_count; i++) {
            if (strcmp(routes[i].route, req->path) == 0) {
                routes[i].callback(req, res);
                if (res->code == 0) res->code = 200;
                found_route = 1;
                break;
            }
        }
        if (found_route != 1) {
            res->code = 404;
            res->body = strdup("404 Not Found");
        }

        if (res->body == NULL) res->body = strdup("");
        size_t write_size = snprintf(NULL, 0, "HTTP/1.0 %d OK\r\nContent-Type: application/json\r\n\r\n%s", res->code, res->body);
        char *write_buffer = malloc(write_size + 1);
        snprintf(write_buffer, write_size + 1, "HTTP/1.0 %d OK\r\nContent-Type: application/json\r\n\r\n%s", res->code, res->body);
        send(new_socket, write_buffer, write_size + 1, 0);
        close(new_socket);
        printf("%s\n", write_buffer);

        // Freeing allocated memory
        for (int i = 0; i < req->header_count; i++) {
            header_t header = req->headers[i];
            free(header.key);
            free(header.value);
        }
        free(req->headers);
        free(req->path);
        free(req->body);
        free(req);
        free(res->body);
        free(res);
        free(write_buffer);

        // Empty buffer
        memset(buffer, '\0', strlen(buffer));
    }
    return 0;
}
