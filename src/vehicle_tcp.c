
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 5001
#define SERVER_IP "127.0.0.1"

int main(void)
{
    int sockfd;
    struct sockaddr_in dashboard_addr;
    char message[256];

    int speed = 60;
    int rpm = 2200;
    float steering = 5.5f;
    int sequence = 1;

    // Create a TCP socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    // Configure the Dashboard ECU address
    memset(&dashboard_addr, 0, sizeof(dashboard_addr));

    dashboard_addr.sin_family = AF_INET;
    dashboard_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP,
                  &dashboard_addr.sin_addr) != 1)
    {
        fprintf(stderr, "Invalid dashboard IP address\n");
        close(sockfd);
        return 1;
    }

    // Establish a TCP connection
    printf("Connecting to Dashboard ECU...\n");

    if (connect(sockfd, (struct sockaddr *)&dashboard_addr,
                sizeof(dashboard_addr)) < 0)
    {
        perror("Connection failed");
        close(sockfd);
        return 1;
    }

    printf("Connected to Dashboard ECU\n");
    printf("Protocol: TCP\n");

    while (1)
    {
        // Format one vehicle-data record per line
        int length = snprintf(
            message,
            sizeof(message),
            "seq=%d,speed=%d,rpm=%d,steering=%.1f\n",
            sequence, speed, rpm, steering
        );

        if (length < 0 || length >= (int)sizeof(message))
        {
            fprintf(stderr, "Message formatting failed\n");
            break;
        }

        // Send the complete record, handling partial writes
        size_t total_sent = 0;

        while (total_sent < (size_t)length)
        {
            ssize_t bytes_sent = send(
                sockfd,
                message + total_sent,
                (size_t)length - total_sent,
                0
            );

            if (bytes_sent <= 0)
            {
                perror("Data transmission failed");
                close(sockfd);
                return 1;
            }

            total_sent += (size_t)bytes_sent;
        }

        printf("Sent: %s", message);
        fflush(stdout);

        // Update simulated vehicle values
        speed += 2;
        rpm += 100;
        steering += 0.5f;

        if (speed > 120)
            speed = 60;

        if (rpm > 4000)
            rpm = 2200;

        if (steering > 45.0f)
            steering = -45.0f;

        sequence++;

        sleep(1);
    }

    // Close the TCP connection
    close(sockfd);
    return 0;
}
