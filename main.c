#include "http_framework.h"

void root(const http_request_t *req, http_response_t *res) {
    // Displaying the request for debugging
    printf("method: %s\n", req->method);
    printf("path: %s\n", req->path);
    for (int i = 0; i < req->header_count; i++) {
        header_t header = req->headers[i];
        printf("HEADER - %d:\n", i);
        printf("%s\n", header.key);
        printf("%s\n", header.value);
    }
}
void hello(const http_request_t *req, http_response_t *res) {
    res->body = strdup("hello");
}

int main() {
    add_route("/", &root);
    add_route("/hello", &hello);
    fw_start(8080);
}
