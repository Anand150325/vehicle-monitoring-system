
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 5000
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

    // Create a UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

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

    printf("Vehicle ECU started\n");
    printf("Protocol: UDP\n");
    printf("Sending data to %s:%d\n", SERVER_IP, PORT);

    while (1)
    {
        // Format the vehicle data
        snprintf(message, sizeof(message),
                 "seq=%d,speed=%d,rpm=%d,steering=%.1f",
                 sequence, speed, rpm, steering);

        // Send the UDP datagram
        ssize_t bytes_sent = sendto(
            sockfd,
            message,
            strlen(message),
            0,
            (struct sockaddr *)&dashboard_addr,
            sizeof(dashboard_addr)
        );

        if (bytes_sent < 0)
        {
            perror("Data transmission failed");
            break;
        }

        printf("Sent: %s\n", message);

        // Change values to simulate a moving vehicle
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

        // Transmit every second
        sleep(1);
    }

    close(sockfd);
    return 0;
}
