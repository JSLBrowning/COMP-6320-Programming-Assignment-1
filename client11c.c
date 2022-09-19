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

int main(int argc, char *argv[])
{
	// Initialize send and receive timestamp variables.
	struct timeval sendTime, recvTime;


	// Initialize variables for message length (2 bytes), sequence number (4 bytes), and transmitted timestamp (8 bytes).
	int msgLen = 0;
	long int seqNum = 1;
	long long int transTime;


	// Initialize variable for assembled packet.
	char packet[MAXPACKET];


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


	// Set server address.
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	// Copy hostname argument to servaddr.
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);


	// Fork process.
	if (fork() == 0)
	{
		// Initialize variable for assembled packet.
		struct packet sendPacket;

		// Send all integers from 1 to 10,000 to the server.
		for (int i = 1; i <= 5000; i++)
		{
			// Convert i (int) to message (char) using sprintf.
			sprintf(message, "%d", i);

			// Assemble packet from string using assemblePacket function.
			assemblePacket(&sendPacket, seqNum, message);

			// Send message to server.
			int len = sendto(sockfd, packet, MAXPACKET, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
		}
	}
	else
	{
		// Declare variable to count number of messages received.
		int numMsgs = 0;

		// Declare array for up to 10,000 recvTimes (long long ints).
		long long int recvTimes[10000];
		// Declare array for up to 10,000 transTimes (long long ints).
		long long int transTimes[10000];
		// Declare array for up to 10,000 time differences.
		long long int timeDiffs[10000];

		// Receive echos from server, process round trip times.
		while(1)
		{
			char buffer[MAXMSGLEN] = {0};
			int len;
			int n = recvfrom(sockfd, (char *)buffer, MAXMSGLEN, MSG_WAITALL,
				0, &len);
			if(n == -1)
			{
				perror("Failed to receive message. Try again...\n");
			}
			else
			{
				// Disassemble received packet.
				disassemblePacket(buffer, &msgLen, &seqNum, &transTime, message);
				// Put transTime in array.
				transTimes[numMsgs] = transTime;
				// Get timestamp for message received.
				gettimeofday(&recvTime, NULL);
				// Convert recvTime to long long int.
				long long int recvTimeLL = (recvTime.tv_sec * 1000000) + recvTime.tv_usec;
				// Put recvTime in array.
				recvTimes[numMsgs] = recvTimeLL;
				// Calculate time difference.
				timeDiffs[numMsgs] = recvTimes[numMsgs] - transTimes[numMsgs];
				// Increment number of messages received.
				numMsgs++;
			}

			// Break out of while loop if more than one second passes without receiving a message.
			if (numMsgs > 100)
			{
				if (recvTime.tv_sec - sendTime.tv_sec > 1)
				{
					break;
				}
			}
		}

		// Calculate average round trip time.
		double avgRTT = 0;
		for (int i = 0; i < numMsgs; i++)
		{
			avgRTT += timeDiffs[i];
		}
		avgRTT = avgRTT / numMsgs;
		printf("Number of messages: %d\n", numMsgs);
		// Print average round trip time.
		printf("Average round trip time: %f microseconds\n", avgRTT);
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
