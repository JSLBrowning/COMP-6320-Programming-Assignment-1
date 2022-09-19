#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT 10010
#define MAXMSGLEN 1024
#define MAXPACKET 1038

struct packet
{
    uint32_t seqNum;
    uint64_t transTime;
    char message[MAXMSGLEN];
    uint16_t msgLen;
};

void assemblePacket(struct packet *packetStruct, uint32_t seqNum, char *message);
void disassemblePacket(struct packet *packetStruct, char *message);

int main()
{
    // Initialize client address variables.
    struct sockaddr_in cliaddr;
    socklen_t clilen;

    // Initialize packet content variables.
    int msgLen = 0;
    long int seqNum = 0;
    long long int transTime = 0;
    char messageReceived[MAXMSGLEN];

    // Declare buffer for received message.
    struct packet buffer[MAXPACKET] = {0};

    // Create socket.
    struct sockaddr_in servaddr = {0};
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1)
    {
        perror("Failed to create socket.\n");
        exit(EXIT_FAILURE);
    }

    // Set up socket.
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket to port.
    int rc = bind(sockfd, (const struct sockaddr *)&servaddr,
                  sizeof(servaddr));
    if (rc == -1)
    {
        perror("Failed to bind socket.\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    socklen_t len = 0;

    printf("Server listening on port %d.\n", PORT);

    // Loop to receive messages and echo them to client.
    while (1)
    {
        clilen = sizeof(cliaddr);
        // Receive packet, store in buffer.
        int n = recvfrom(sockfd, (struct packet *)buffer, MAXPACKET, MSG_WAITALL,
                         (struct sockaddr *)&cliaddr, &clilen);
        if (n == -1)
        {
            perror("Failed to receive packet.\n");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        else
        {
            printf("================================\n");

            // Disassemble packet struct.
            disassemblePacket((struct packet *)buffer, messageReceived);
            printf("Received message: %s", messageReceived);

            // Send packet struct back.
            int n = sendto(sockfd, (struct packet *)buffer, MAXPACKET, MSG_CONFIRM,
                           (const struct sockaddr *)&cliaddr, clilen);
            printf("Echo sent.\n");
            printf("================================\nWaiting for new messages...\n");
        }
    }
}

// Function to build packet w/ message length (2 bytes), sequence number (4 bytes),
// transmitted timestamp (8 bytes), and string (up to 1,024 bytes).
void assemblePacket(struct packet *packetStruct, uint32_t seqNum, char *message)
{
	// Assign sequence number to packet struct.
	packetStruct->seqNum = seqNum;

	// Assign transmitted timestamp to packet struct.
	struct timeval transTime;
	gettimeofday(&transTime, NULL);
	packetStruct->transTime = (long long int)transTime.tv_sec * 1000000 + (long long int)transTime.tv_usec;

	// Assign message to packet struct.
	strcpy(packetStruct->message, message);

	// Assign total message length to packet struct.
	packetStruct->msgLen = sizeof(packetStruct->seqNum) + sizeof(packetStruct->transTime) + strlen(packetStruct->message);
}

// Function to disassemble packet struct.
void disassemblePacket(struct packet *packetStruct, char *messageReceived)
{
	// Get sequence number from packet struct.
	uint32_t seqNum = packetStruct->seqNum;

	// Get transmitted timestamp from packet struct.
	uint64_t transTime = packetStruct->transTime;

	// Get message from packet struct.
	// printf("Message received: %s", packetStruct->message);
	strcpy(messageReceived, packetStruct->message);

	// Get message length from packet struct.
	uint16_t msgLen = packetStruct->msgLen;
}
