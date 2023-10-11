#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <sys/socket.h>

/**
 * Project 1 starter code
 * All parts needed to be changed/added are marked with TODO
 */

#define BUFFER_SIZE 1024
#define DEFAULT_SERVER_PORT 8081
#define DEFAULT_REMOTE_HOST "131.179.176.34"
#define DEFAULT_REMOTE_PORT 5001


#define DEBUG
#ifdef DEBUG
    #define DEBUG_PRINT(fmt, ...) fprintf(stderr, "DEBUG: %s:%d:%s(): " fmt "\n", \
        __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
    #define DEBUG_PRINT(fmt, ...) // Do nothing in non-debug mode
#endif


struct server_app {
    // Parameters of the server
    // Local port of HTTP server
    uint16_t server_port;

    // Remote host and port of remote proxy
    char *remote_host;
    uint16_t remote_port;
};


/** 
 * @brief Struct for easy manipulation of file metadata.
 */
struct local_file {
    char *type;
    char *content;
    long int size;
};

// The following function is implemented for you and doesn't need
// to be change
void parse_args(int argc, char *argv[], struct server_app *app);

// The following functions need to be updated
void handle_request(struct server_app *app, int client_socket);
void serve_local_file(int client_socket, const char *path);
void proxy_remote_file(struct server_app *app, int client_socket, const char *path);


// creates a local_file struct pointer and returns it.
struct local_file *create_local_file(const char *type, const char *content, const long int size);
// parses a file given its name
struct local_file *parse_file(const char *file_name);
// helper function for parse_file()
char *get_file_type(const char *file_name);

// The main function is provided and no change is needed
int main(int argc, char *argv[])
{
    struct server_app app;
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    int ret;

    parse_args(argc, argv, &app);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(app.server_port);

    // The following allows the program to immediately bind to the port in case
    // previous run exits recently
    int optval = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 10) == -1) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", app.server_port);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket == -1) {
            perror("accept failed");
            continue;
        }
        
        printf("Accepted connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        handle_request(&app, client_socket);
        close(client_socket);
    }

    close(server_socket);
    return 0;
}

void parse_args(int argc, char *argv[], struct server_app *app)
{
    int opt;

    app->server_port = DEFAULT_SERVER_PORT;
    app->remote_host = NULL;
    app->remote_port = DEFAULT_REMOTE_PORT;

    while ((opt = getopt(argc, argv, "b:r:p:")) != -1) {
        switch (opt) {
        case 'b':
            app->server_port = atoi(optarg);
            break;
        case 'r':
            app->remote_host = strdup(optarg);
            break;
        case 'p':
            app->remote_port = atoi(optarg);
            break;
        default: /* Unrecognized parameter or "-?" */
            fprintf(stderr, "Usage: server [-b local_port] [-r remote_host] [-p remote_port]\n");
            exit(-1);
            break;
        }
    }

    if (app->remote_host == NULL) {
        app->remote_host = strdup(DEFAULT_REMOTE_HOST);
    }
}

void handle_request(struct server_app *app, int client_socket) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    // Read the request from HTTP client
    // Note: This code is not ideal in the real world because it
    // assumes that the request header is small enough and can be read
    // once as a whole.
    // However, the current version suffices for our testing.
    bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read <= 0) {
        return;  // Connection closed or error
    }

    buffer[bytes_read] = '\0';
    // copy buffer to a new string
    char *request = malloc(strlen(buffer) + 1);
    strcpy(request, buffer);

    DEBUG_PRINT("Request is:\n%s", request);

    // ***** NOTE *****
    // I have done the bare minimum to get something running.
    // we still need to do more than just parse the filename. Sorry !

    // get the start and end of the file name
    int start = 0;
    while (request[start++] != '/');
    int end = start;
    while (request[end++] != ' ');

    // Get the file name using the start/end pointers.
    char *file_name =  (char *)malloc(end - start);
    strlcpy(file_name, request + start, end - start);

    DEBUG_PRINT("Filename before: %s", file_name);
    DEBUG_PRINT("Start and End indices (start, end): (%d, %d)", start, end);

    // if the path is "/" (root), default to index.html
    if (!strcmp(file_name, "")) {
        // we need to reallocate file_name so it's the correct size
        free(file_name);
        file_name = (char *)malloc(strlen("index.html") + 1);
        file_name = strdup("index.html");
    }

    DEBUG_PRINT("Filename after: %s", file_name);




    // TODO: Parse the header and extract essential fields, e.g. file name
    // Hint: if the requested path is "/" (root), default to index.html
    // char file_name[] = "index.html";

    // TODO: Implement proxy and call the function under condition
    // specified in the spec
    // if (need_proxy(...)) {
    //    proxy_remote_file(app, client_socket, file_name);
    // } else {
    serve_local_file(client_socket, file_name);
    //}
    free(file_name);
}

void serve_local_file(int client_socket, const char *path) {
    // TODO: Properly implement serving of local files
    // The following code returns a dummy response for all requests
    // but it should give you a rough idea about what a proper response looks like
    // What you need to do 
    // (when the requested file exists):
    // * Open the requested file
    // * Build proper response headers (see details in the spec), and send them
    // * Also send file content
    // (When the requested file does not exist):
    // * Generate a correct response

    // char response[] = "HTTP/1.0 200 OK\r\n"
    // "Content-Type: text/plain; charset=UTF-8\r\n"
    // "Content-Length: 15\r\n"
    // "\r\n"
    // "Sample response";

    // ***** NOTE *****
    // I HAVE NOT IMPLEMENTED A LOT OF THE RESPONSES. THIS IS JUST THE BARE MINIMUM
    // TO SEE SOMETHING RUNNING.


    // @todo this is hardcoded in. should we leave it?
    char not_found_header[] = "HTTP/1.0 404 Not Found\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "Content-Length: 153\r\n"
        "\r\n"
        "<html>\r\n"
        "<head><title>404 Not Found</title></head>\r\n"
        "<body>\r\n"
        "<h1>Not Found</h1>\r\n"
        "<p>The requested URL was not found on this server.</p>\r\n"
        "</body>\r\n"
        "</html>\r\n";


    char ok_header[] = "HTTP/1.0 200 OK\r\n";


    // get the file's metadata
    struct local_file *file = parse_file(path);
    // we want to dynamically build our response
    char *response = NULL;

    // there is no file with the specified path on the server
    if (!file) {
        response = (char *)malloc(strlen(not_found_header));
        sprintf(response, "%s", not_found_header);

        DEBUG_PRINT("404 error for file: %s", path);
    }
    else {
        // @todo WE NEED TO CHANGE THE 10000 IT'S JUST A PLACEHOLDER
        response = (char *)malloc(file->size + strlen(file->type) + 10000);
        sprintf(response, "%s"
                "Content-Type: %s\r\n"
                "Content-Length: %ld\r\n"
                "\r\n"
                "%s\r\n", ok_header, file->type, file->size, file->content);

        DEBUG_PRINT("Response was successful for file: %s", path);

        // free the file since we own it
        free(file->type);
        free(file->content);
        free(file);
    }


    DEBUG_PRINT("Response is:\n%s", response);
    send(client_socket, response, strlen(response), 0);

    free(response);
}

void proxy_remote_file(struct server_app *app, int client_socket, const char *request) {
    // TODO: Implement proxy request and replace the following code
    // What's needed:
    // * Connect to remote server (app->remote_server/app->remote_port)
    // * Forward the original request to the remote server
    // * Pass the response from remote server back
    // Bonus:
    // * When connection to the remote server fail, properly generate
    // HTTP 502 "Bad Gateway" response

    char response[] = "HTTP/1.0 501 Not Implemented\r\n\r\n";
    send(client_socket, response, strlen(response), 0);
}

/**
 * @brief Creates a local_file struct pointer
 *
 * This function acts like a constructor for the local_file struct.
 *
 * @param type: File type (extension/Content-Type).
 * @param content: The actual content of the file. 
 * @param size: The size of the file (Content-length). 
 *
 * @return The local_file struct pointer
 *
 * @note This function isn't completely necessary.
 */
struct local_file *create_local_file(const char *type, const char *content, const long int size) {
    struct local_file *file = (struct local_file*)malloc(sizeof(struct local_file));

    file->type = (char *)type;
    file->content = (char *)content;
    file->size = size;

    return file;
}


/**
 * @brief Extract the file's metadata
 *
 * This function extracts the file's metadata and creates a local_file pointer.
 *
 * @param file_name: The name of the file.
 *
 * @return The local_file struct pointer filled in.
 *
 * @todo Check to see if I am covering all potential memory leaks.
 */
struct local_file *parse_file(const char *file_name) {
    char *type = strdup(get_file_type(file_name));
    char *content = NULL;
    long int size = 0;
    struct local_file *file = NULL;
    

    // check for a valid type
    if (!strcmp(type, "")) {
        free(type);
        perror("invalid type");
        return NULL;
    }

    // open the file (we assume it's valid here)
    FILE *fp = fopen(file_name, "r");
    if (!fp) {
        free(type);
        perror("fopen failed");
        return NULL;
    }
    
    // get file size
    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);


    // get file content
    content = (char *)malloc(size + 1);
    if (!content) {
        free(type);
        free(content);
        perror("malloc failed");
        fclose(fp);
        return NULL;
    }

    // read the file contents
    fread(content, 1, size, fp);
    content[size] = '\0';
    fclose(fp);

    file = create_local_file(type, content, size);
    if (!file) {
        free(type);
        free(content);
        perror("malloc failed");
        return NULL;
    }

    return file;
}


/**
 * @brief Helper function for parse_file().
 *
 * This function returns the file type for the Content-Type field of the HTTP header.
 *
 * @param file_name: The name of the file.
 *
 * @return The Content-Type OR the empty string if it's an invalid type.
 *
 * @todo We may need to add extensions.
 * @todo I guesseed and put image/jpeg. It could be wrong.
 */
char *get_file_type(const char *file_name) {
    char *extension = strrchr(file_name, '.');

    if (extension && *(extension + 1)) {
        extension++;
        if (strcmp(extension, "txt") == 0)
            return "text/plain; charset=UTF-8";
        else if (strcmp(extension, "html") == 0)
            return "text/html; charset=UTF-8";
        else if (strcmp(extension, "jpg") == 0)
            return "image/jpeg";
        else if (strcmp(extension, "m3u8") == 0)
            return "audio/mpegurl";
    }
    return "";
}
