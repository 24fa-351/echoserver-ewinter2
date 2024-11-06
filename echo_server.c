#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void *handle_client(void *arg) {
    int client_socket = *((int *)arg);
    free(arg);
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    printf("Connected to client on thread %lu\n", (unsigned long)pthread_self());

    while(1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) {
            break;
        }

        if (strncmp(buffer, "GET ", 4) == 0 || strncmp(buffer, "POST ", 5) == 0) {
            sprintf(response, 
                "HTTP/1.1 200 OK\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: %d\r\n"
                "\r\n"
                "%s", bytes_received, buffer);
            send(client_socket, response, strlen(response), 0);
        } else {
            send(client_socket, buffer, bytes_received, 0);
        }
   }

   printf("Client on thread %ld disconnected\n", (unsigned long)pthread_self());
   close(client_socket);
   return NULL;


}

int main(int argc, char *argv[]) {
    int opt;
    int port = 0;

    while ((opt = getopt(argc, argv, "p:")) != -1) {
        switch (opt) {
            case 'p':
                port = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s -p [port]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (port == 0) {
        fprintf(stderr, "Port number is required. \n Usage: %s -p [port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int server_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        error("Error opening socket");
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        close(server_socket);
        error("Error binding socket");
    }

    if (listen(server_socket, 5) < 0) {
        close(server_socket);
        error("Error listening on socket");
    }

    printf("Server listening on port %d\n", port);
    
    while(1) {
        int *client_socket = malloc(sizeof(int));
        if (!client_socket) {
            error("Error allocating memory for client socket");
            continue;
        }

        *client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (*client_socket < 0) {
            perror("Error accepting client connection");
            free(client_socket);
            continue;
        }

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, (void *)client_socket) != 0) {
            perror("Error creating thread");
            free(client_socket);
            continue;
        }
        else {
            pthread_detach(thread_id);
        }
    }

    close(server_socket);
    return 0;
}