/*
** client12.c -- TCP Client
*/

#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#define PORT 10030
#define SEND_BUFFER_SIZE 23 // Client can enter up to 23 characters ((+/-)10 digits, op, (+/-)10 digits)

struct recv_data
{
  uint32_t a;
  uint32_t b;
  uint32_t c;
  char operation;
  char is_valid;
} __attribute__((packed));

struct send_data
{
  uint32_t a;
  uint32_t b;
  char operation;
} __attribute__((packed));

int main(int argc, char *argv[])
{
  int sock, send_bytes;
  struct sockaddr_in server_addr = {0};
  socklen_t addr_size;
  struct recv_data rd;
  struct send_data sd;
  char send_buff[SEND_BUFFER_SIZE], *eval;

  // Verify that the server hostname was provided
  if (argc != 2)
  {
    printf("Error: Missing server hostname!\n");
    exit(-1);
  }

  // Attempt to create the socket
  sock = socket(AF_INET, SOCK_STREAM, 0);

  // Configure TCP socket
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = inet_addr(argv[1]);
  memset(server_addr.sin_zero, '\0', sizeof(server_addr.sin_zero));

  // Connect the socket to the server
  addr_size = sizeof(server_addr);
  if (connect(sock, (struct sockaddr *)&server_addr, addr_size) < 0)
  {
    printf("Error: Could not connect to the server...\n");
    exit(-1);
  }

  // Send and receive input to the server
  printf("Connected to server. Usage: <a><+, -, *, /><b>\n");
  printf("Enter a query: ");
  while (fgets(send_buff, SEND_BUFFER_SIZE, stdin) != NULL)
  {
    // Extract data from user input
    sd.a = strtol(send_buff, &eval, 10);
    while (isdigit(*eval))
      eval++;
    sd.operation = *eval++;
    sd.b = strtol(eval, &eval, 10);

    printf("= CLIENT12 ===================================\n");
    printf("Sending data to server.      A: %d, B: %d, Op: %c\n", sd.a, sd.b, sd.operation);
    send_bytes = send(sock, &sd, sizeof(sd), 0);

    // Wait to receive response from the server
    if (recv(sock, &rd, sizeof(struct recv_data), 0) == 0)
    {
      printf("----------------------------------------------\n");
      printf("Error:      The server terminated prematurely.\n");
      printf("==============================================\n\n");
      exit(-1);
    }

    // Print response from server.
    printf("----------------------------------------------\n");
    printf("Response: %d%c%d=%d\nIs valid? %c\n", rd.a, rd.operation, rd.b, rd.c, rd.is_valid);
    printf("==============================================\n\n");
    printf("Enter another query: ");

    // clear input buffer
    bzero(send_buff, SEND_BUFFER_SIZE);
  }

  exit(0);
}