/*
** server12.c -- TCP Server
*/

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define PORT 10030
#define MAX_CLIENTS 10
#define SEND_BUFFER_SIZE 160
#define RECV_BUFFER_SIZE 100

struct recv_data
{
  uint32_t a;
  uint32_t b;
  char operation;
} __attribute__((packed));

struct send_data
{
  uint32_t a;
  uint32_t b;
  uint32_t c;
  char operation;
  char is_valid;
} __attribute__((packed));

void *ClientHandler(void *);

int main(int argc, char **argv)
{
  int sock, client_sock, c;
  struct sockaddr_in server, client;

  // Create the socket
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1)
  {
    perror("Failed to create socket. Exiting...\n");
		exit(EXIT_FAILURE);
  }

  // Prepare the socket
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  server.sin_port = htons(PORT);

  // Bind the socket
  if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
  {
    perror("Failed to bind socket. Exiting...\n");
		exit(EXIT_FAILURE);
  }

  // Listen
  listen(sock, MAX_CLIENTS);
  printf("Server is running on port %d.\n", PORT);

  // Accept incoming connections
  for (;;)
  {
    pthread_t thread_id;

    c = sizeof(struct sockaddr_in);
    client_sock = accept(sock, (struct sockaddr *)&client, (socklen_t *)&c);
    printf("%s\n", "A new client has connected!\n");

    // Assign client to their own thread
    if (pthread_create(&thread_id, NULL, ClientHandler, (void *)&client_sock) < 0)
    {
      printf("Error: Could not create thread...\n");
    }
  }

  if (client_sock < 0)
  {
    printf("Error: Failed to accept client.\n");
  }

  close(sock);
  return 0;
}

/*
** Handles the connection for each client that connects
*/
void *ClientHandler(void *sock_desc)
{
  // Get the socket descriptor
  int sock = *(int *)sock_desc, read_size, bytes_sent;
  char *message, *eval;
  struct recv_data rd;
  struct send_data sd;

  // Send client usage message
  while (read_size = recv(sock, &rd, sizeof(struct recv_data), 0) > 0)
  {
    printf("Bytes received: %ld\n", sizeof(rd));
    // Default message validity to true
    sd.is_valid = '1';
    // Do work with message
    sd.a = rd.a;
    sd.operation = rd.operation;
    sd.b = rd.b;

    switch (sd.operation)
    {
    case '+':
      sd.c = sd.a + sd.b;
      break;
    case '-':
      sd.c = sd.a - sd.b;
      break;
    case '*':
      sd.c = sd.a * sd.b;
      break;
    case '/':
      if (sd.b == 0)
      {
        sd.c = 0;
        sd.is_valid = '2';
      }
      else
      {
        sd.c = sd.a / sd.b;
      }
      break;
    default:
      sd.is_valid = '2';
      break;
    }

    // Send client a message back
    bytes_sent = send(sock, &sd, sizeof(sd), 0);
    printf("Bytes sent: %d\n", bytes_sent);
  }

  // Client disconnected
  if (read_size == 0)
  {
    printf("A client has disconnected...\n");
  }
  else if (read_size == -1)
  {
    printf("Error: recv failed...\n");
  }

  return 0;
}