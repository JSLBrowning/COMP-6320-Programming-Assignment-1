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
		printf("Usage: ./client11b <server hostname>\n");
		printf("Please try again.\n");
		exit(EXIT_FAILURE);
	}

	// Initialize send and receive timestamp variables.
	struct timeval sendTime, recvTime, startTime, currentTime;
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

	// Fork process.
	if (fork() == 0)
	{
		// Initialize variable for assembled packet.
		struct packet sendPacket;

		// Send all integers from 1 to 10,000 to the server.
		for (int i = 1; i <= 10000; i++)
		{
			// Convert i (int) to message (char) using sprintf.
			sprintf(message, "%d", i);

			// Get sendTime.
			gettimeofday(&sendTime, NULL);
			// Convert sendTime to uint64_t.
			sendTime64 = (uint64_t)sendTime.tv_sec * 1000000 + (uint64_t)sendTime.tv_usec;

			// Assemble packet from string using assemblePacket function.
			assemblePacket(&sendPacket, seqNum, sendTime64, message);

			// Send message to server.
			int len = sendto(sockfd, (const struct packet *)&sendPacket, sizeof(sendPacket), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
		}
	}
	else
	{
		// Set startTime
		gettimeofday(&startTime, NULL);

		// Declare variable to count number of messages received.
		int numMsgs = 0;

		// Declare array for up to 10,000 recvTimes (long long ints).
		long long int recvTimes[10000];
		// Declare array for up to 10,000 transTimes (long long ints).
		long long int sendTimes[10000];
		// Declare array for up to 10,000 time differences.
		long long int timeDiffs[10000];

		// Receive echos from server, process round trip times.
		while (1)
		{
			// If more than a second has passed since the start of the program, break.
			gettimeofday(&currentTime, NULL);
			if (currentTime.tv_sec - startTime.tv_sec > 1)
			{
				printf("No response for a while. Quitting...\n");
				break;
			}

			char buffer[MAXMSGLEN] = {0};
			int len;
			int n = recvfrom(sockfd, (char *)buffer, MAXMSGLEN, MSG_WAITALL,
							 0, &len);
			if (n == -1)
			{
				perror("Failed to receive message. Try again...\n");
			}
			else
			{
				// Disassemble received packet.
				disassemblePacket((struct packet *)buffer, &sendTime64, messageReceived);
				// Put sendTime in array.
				sendTimes[numMsgs] = sendTime64;
				// Get timestamp for message received.
				gettimeofday(&recvTime, NULL);
				// Convert recvTime to long long int.
				long long int recvTimeLL = (recvTime.tv_sec * 1000000) + recvTime.tv_usec;
				// Put recvTime in array.
				recvTimes[numMsgs] = recvTimeLL;
				// Calculate time difference.
				timeDiffs[numMsgs] = recvTimes[numMsgs] - sendTimes[numMsgs];
				// Increment number of messages received.
				numMsgs++;
				// Update startTime using gettimeofday.
				gettimeofday(&startTime, NULL);

				// Calculate average round trip time.
				double avgRTT = 0;
				for (int i = 0; i < numMsgs; i++)
				{
					avgRTT += timeDiffs[i];
				}
				avgRTT = avgRTT / numMsgs;
				printf("Number of messages so far: %d\n", numMsgs);
				// Print average round trip time.
				printf("Average round trip time so far: %f microseconds\n", avgRTT);
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
