// Author: Ahmet Atar (22290230)
// Description: Persistent TCP server that calculates Shannon capacity and stays alive for multiple clients.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <math.h>

#define BUFFER_SIZE 1024

// Prototypes
double calcShannonCapacity(double bandwidth, double snr);
void handleClient(int clientFd);

int main(int argc, char *argv[])
{
    int port = 0;

    // Command-line argument parsing
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc)
        {
            port = atoi(argv[i + 1]);
        }
    }

    if (port == 0)
    {
        fprintf(stderr, "Usage: %s -p <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int serverFd;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t clientLength = sizeof(clientAddress);

    // Create socket
    if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Reuse port immediately after restart
    int opt = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    // Bind
    if (bind(serverFd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        perror("Bind failed");
        close(serverFd);
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(serverFd, 5) < 0)
    {
        perror("Listen failed");
        close(serverFd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d... (press Ctrl+C to terminate)\n", port);

    // Infinite loop in order for server to stay alive
    while (1 == 1)
    {
        int clientFd = accept(serverFd, (struct sockaddr *)&clientAddress, &clientLength);
        if (clientFd < 0)
        {
            perror("Accept failed");
            continue;
        }

        printf("New client connected.\n");
        handleClient(clientFd);
        printf("Client disconnected.\n");
        close(clientFd);
    }

    close(serverFd);
    return 0;
}

void handleClient(int clientFd)
{
    char buffer[BUFFER_SIZE];

    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);
        int bytesRead = read(clientFd, buffer, sizeof(buffer) - 1);

        if (bytesRead <= 0)
        {
            break;
        }

        // Trim newline if present
        buffer[strcspn(buffer, "\r\n")] = 0;

        // Check for finish
        if (strncmp(buffer, "finish", 6) == 0)
        {
            break;
        }

        double bandwidth, snr;
        if (sscanf(buffer, "%lf %lf", &bandwidth, &snr) == 2)
        {
            double result = calcShannonCapacity(bandwidth, snr);
            char res[BUFFER_SIZE];
            snprintf(res, sizeof(res), "%.2f\n", result);
            write(clientFd, res, strlen(res));
        }
    }
}

double calcShannonCapacity(double bandwidth, double snr)
{
    return bandwidth * (log2(1 + snr));
}
