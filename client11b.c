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
	uint16_t msgLen;
	uint32_t seqNum;
	uint64_t sendTime64;
	char message[MAXMSGLEN];
};

void assemblePacket(struct packet *packetStruct, uint32_t seqNum, uint64_t sendTime64, char *message);
void disassemblePacket(struct packet *packetStruct, uint64_t *sendTime64, char *message);

int main(int argc, char *argv[])
{
	// Check if server address is provided.
	if (argc != 2)
	{
		printf("Usage: ./client <server hostname>\n");
		printf("Please try again.\n");
		exit(EXIT_FAILURE);
	}
	else if (argc == 2)
	{
		// Check if hostname is correct (just localhost for now).
		if (strcmp(argv[1], "localhost") != 0)
		{
			printf("Usage: ./client <server hostname>\n");
			printf("(Try 'localhost' instead -- we're only doing local loop here.)\n");
			exit(EXIT_FAILURE);
		}
	}

	// Initialize send and receive timestamp variables.
	struct timeval sendTime, recvTime;
	uint64_t sendTime64 = 0;

	// Initialize variables for message length (2 bytes), sequence number (4 bytes), and echoed message (up to 1,024 bytes).
	int msgLen = 0;
	long int seqNum = 1;
	char messageReceived[MAXMSGLEN];

	// Initialize variable for assembled packet.
	struct packet sendPacket;

	// Initialize input string variable.
	char message[MAXMSGLEN];

	// Create socket.
	struct sockaddr_in servaddr = {0};
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1)
	{
		perror("Failed to create socket. Exiting...\n");
		exit(EXIT_FAILURE);
	}

	// Set up socket.
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	// Loop to send messages and print messages echoed by server.
	while (1)
	{
		/* SEND MESSAGES TO SERVER */

		// Get input string.
		printf("Enter a string: ");
		fgets(message, MAXMSGLEN, stdin);

		// Get sendTime.
		gettimeofday(&sendTime, NULL);
		// Convert sendTime to uint64_t.
		sendTime64 = (uint64_t)sendTime.tv_sec * 1000000 + (uint64_t)sendTime.tv_usec;

		// Assemble packet from string using assemblePacket function.
		assemblePacket(&sendPacket, seqNum, sendTime64, message);

		// Send packet struct to server.
		int len = sendto(sockfd, (const struct packet *)&sendPacket, sizeof(sendPacket), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
		if (len == -1)
		{
			perror("Failed to send message. Try again...\n");
		}
		else
		{
			printf("= CLIENT11B.C ================================\n");
			// Print message sent.
			printf("Message sent: %s", message);
			// Print timestamp for message sent.
			printf("Timestamp for message sent:  %ld.%06ld\n", sendTime.tv_sec, sendTime.tv_usec);
			// Set and print new seqNum.
			seqNum = seqNum + len;
			printf("Updated sequence number: %ld\n", seqNum);
		}

		/* RECEIVE ECHOES FROM SERVER */

		struct packet buffer[MAXPACKET] = {0};
		// Receive echoed packet struct.
		int n = recvfrom(sockfd, (struct packet *)buffer, MAXPACKET, 0, (struct sockaddr *)&servaddr, (socklen_t *)&len);
		if (n == -1)
		{
			printf("----------------------------------------------\n");
			perror("Failed to receive echo.           Try again...\n");
			printf("==============================================\n");
		}
		else
		{
			printf("----------------------------------------------\n");
			// Disassemble received packet struct in buffer.
			disassemblePacket((struct packet *)buffer, &sendTime64, messageReceived);
			// Echo message received.
			printf("Echo received: %s", messageReceived);
			// Get timestamp for echo received.
			gettimeofday(&recvTime, NULL);
			uint64_t recvTime64 = (uint64_t)recvTime.tv_sec * 1000000 + (uint64_t)recvTime.tv_usec;
			// Print timestamp for message received.
			printf("Timestamp for echo received: %ld.%06ld\n", recvTime.tv_sec, recvTime.tv_usec);
			// Calculate round trip time.
			long int rtt = recvTime64 - sendTime64;
			// Print round trip time.
			printf("Round trip time: %ld microseconds\n", rtt);
			printf("==============================================\n");
		}
	}
}

// Function to build packet w/ message length (2 bytes), sequence number (4 bytes),
// transmitted timestamp (8 bytes), and string (up to 1,024 bytes).
void assemblePacket(struct packet *packetStruct, uint32_t seqNum, uint64_t sendTime64, char *message)
{
	// Assign sequence number to packet struct.
	packetStruct->seqNum = seqNum;

	// Assign transmitted timestamp to packet struct.
	packetStruct->sendTime64 = sendTime64;

	// Assign message to packet struct.
	strcpy(packetStruct->message, message);

	// Assign total message length to packet struct.
	packetStruct->msgLen = sizeof(packetStruct->seqNum) + sizeof(packetStruct->sendTime64) + strlen(packetStruct->message);
}

// Function to disassemble packet struct.
void disassemblePacket(struct packet *packetStruct, uint64_t *sendTime64, char *messageReceived)
{
	// Get transmitted timestamp from packet struct.
	*sendTime64 = packetStruct->sendTime64;

	// Get message from packet struct.
	// printf("Message received: %s", packetStruct->message);
	strcpy(messageReceived, packetStruct->message);
}
