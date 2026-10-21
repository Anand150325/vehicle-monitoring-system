
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 5001
#define BUFFER_SIZE 1024

int main(void)
{
    int server_fd;
    int client_fd;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // Create a TCP socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    // Allow quick reuse of the server address after restart
    int reuse = 1;

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
                   &reuse, sizeof(reuse)) < 0)
    {
        perror("setsockopt failed");
        close(server_fd);
        return 1;
    }

    // Configure the server address
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    // Bind to TCP port 5001
    if (bind(server_fd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        return 1;
    }

    // Listen for incoming TCP connections
    if (listen(server_fd, 5) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        return 1;
    }

    printf("Dashboard ECU started\n");
    printf("Protocol: TCP\n");
    printf("Listening on port %d...\n", PORT);
    fflush(stdout);

    while (1)
    {
        // Accept a Vehicle ECU connection
        client_fd = accept(server_fd,
                           (struct sockaddr *)&client_addr,
                           &client_len);

        if (client_fd < 0)
        {
            perror("Accept failed");
            continue;
        }

        printf("Vehicle ECU connected\n");
        fflush(stdout);

        // Receive data until the client disconnects
        while (1)
        {
            ssize_t bytes_received = recv(
                client_fd,
                buffer,
                sizeof(buffer) - 1,
                0
            );

            if (bytes_received > 0)
            {
                buffer[bytes_received] = '\0';

                printf("Received: %s", buffer);

                // Ensure output appears immediately
                fflush(stdout);
            }
            else if (bytes_received == 0)
            {
                printf("Vehicle ECU disconnected\n");
                fflush(stdout);
                break;
            }
            else
            {
                perror("Receive failed");
                break;
            }
        }

        close(client_fd);
        client_len = sizeof(client_addr);
    }

    close(server_fd);
    return 0;
}
