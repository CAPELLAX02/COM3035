// Author: Ahmet Atar (22290230)
// Description: TCP client that reads (B, SNR) from stdin, sends them to server, receives Shannon capacity, and outputs results.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <math.h>

#define BUFFER_SIZE 1024

// Prototypes
void parseArgs(int argc, char *argv[], char **host, int *port);
void talkToServer(const char *host, int port);

int main(int argc, char *argv[])
{
    char *host = NULL;
    int port = 0;

    parseArgs(argc, argv, &host, &port);

    if (host == NULL || port == 0)
    {
        fprintf(stderr, "Usage: %s -h <host> -p <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    talkToServer(host, port);
    return 0;
}

void parseArgs(int argc, char *argv[], char **host, int *port)
{
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-h") == 0 && i + 1 < argc)
        {
            *host = argv[i + 1];
        }
        else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc)
        {
            *port = atoi(argv[i + 1]);
        }
    }
}

void talkToServer(const char *host, int port)
{
    int sockFd;
    struct sockaddr_in serverAddress;
    char sendBuffer[BUFFER_SIZE], recvBuffer[BUFFER_SIZE];

    if ((sockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);

    if (inet_pton(AF_INET, host, &serverAddress.sin_addr) <= 0)
    {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }

    if (connect(sockFd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    double maxValue = -1.0;
    int bestChannel = 0, channelCount = 0;

    // Read from stdin line by line
    while (fgets(sendBuffer, sizeof(sendBuffer), stdin) != NULL)
    {
        if (strncmp(sendBuffer, "finish", 6) == 0)
        {
            write(sockFd, sendBuffer, strlen(sendBuffer));
            break;
        }

        // Send data to server
        write(sockFd, sendBuffer, strlen(sendBuffer));

        // Receive response
        memset(recvBuffer, 0, sizeof(recvBuffer));
        int bytesReceived = read(sockFd, recvBuffer, sizeof(recvBuffer) - 1);

        if (bytesReceived <= 0)
        {
            break;
        }

        double capacity = atof(recvBuffer);
        printf("%.2f\n", capacity);

        channelCount++;

        if (capacity > maxValue)
        {
            maxValue = capacity;
            bestChannel = channelCount;
        }
    }

    printf("Selected Channel: %d\n", bestChannel);

    close(sockFd);
}
