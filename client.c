#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#define MAX_SIZE 1024
extern int errno;

int port;

int main(int argc, char *argv[])
{
  int sd;
  struct sockaddr_in server;
  char response;
  char buf[10];

  if (argc != 3)
  {
    printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
    return -1;
  }

  port = atoi(argv[2]);
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("Eroare la socket().\n");
    return errno;
  }

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(argv[1]);
  server.sin_port = htons(port);

  if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    perror("[client]Eroare la connect().\n");
    return errno;
  }
  while (1)
  {
    char question[MAX_SIZE];
    question[0]='\0';
    recv(sd, question, sizeof(question), 0);
    printf("%s", question);

    
    char user_response[MAX_SIZE];
    user_response[0]='\0';
    printf("Raspunsul clientului: ");
    fgets(user_response, sizeof(user_response), stdin);

    if (send(sd, user_response, sizeof(user_response), 0) < 0)
    {
      perror("[client]Eroare la send() spre server.\n");
      return errno;
    }

    char correct_response[MAX_SIZE];
     correct_response[0]='\0';
    if (recv(sd, correct_response, sizeof(correct_response), 0) < 0)
    {
      perror("[client]Eroare la recv() de la server.\n");
      return errno;
    }
    printf("[client] Am citit raspunsul de la server. %s\n", correct_response);
  }
  close(sd);
}