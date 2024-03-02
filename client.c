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
#include <stdbool.h>
#include <sys/select.h>

#define MAX_SIZE 1024
extern int errno;

int port;
 char user_response[MAX_SIZE];
 bool connected=true;

int main(int argc, char *argv[])
{

  int sd;
  struct sockaddr_in server;
  char response;
  char buf[10];
  int registration = 0;

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
  printf("-----------------------------  ^_^ QUIZZ GAME ^_^  ----------------------------------------------\n");
  printf("--------------------BINE ATI VENIT LA ACEASTA SESIUNE DE JOC!------------------------------------\n");
  char username[MAX_SIZE];
  bzero(username, MAX_SIZE);
  printf("Introduceti un nume de utilizator pentru a putea participa la aceasta sesiune de joc!\n> %s", username);
  fgets(username, sizeof(username), stdin);
  send(sd, username, sizeof(username), 0);
 
  char login_message[MAX_SIZE];
  bzero(login_message, MAX_SIZE);
  recv(sd, login_message, sizeof(login_message), 0);
  printf("%s", login_message);

   printf("Asteptam conectarea mai multor jucatori pentru a incepe sesiunea de joc! Este necesara conectarea a minimum 3 utilizatori pentru a putea incepe jocul \n");
    char readyMsg[] = "READY";
    if (send(sd, readyMsg, strlen(readyMsg), 0) < 0) {
        perror("Eroare la trimiterea mesajului READY la server");
        close(sd);
        exit(EXIT_FAILURE);
    }
  

 printf("--------------------------------------------------------------------------------------\n");

  while (connected)
  { 
    
     printf("Daca doriti sa va deconectati inaintea terminarii sesiunii de joc, tastati quit dupa ce va vor fi furnizate intrebarile!\n");
    
      if(strcmp(user_response,"quit")==0){
      int scoreMsgLen;
      recv(sd, &scoreMsgLen, sizeof(scoreMsgLen),0);
      char *scoreMsg= malloc(scoreMsgLen+1);
      recv(sd, scoreMsg, scoreMsgLen,0);
      printf("%s",scoreMsg);
      free(scoreMsg);
      close(sd);
      connected=false;
      exit(0);
      }
    
    int questionLen;
    recv(sd, &questionLen, sizeof(questionLen), 0);

    char *question = malloc(questionLen + 1);
    memset(question,0, questionLen+1);
    if (recv(sd, question, questionLen, 0) < 0)
    {
      perror("[client]Eroare la recv() de la server.\n");
      return errno;
    }
    question[questionLen] = '\0';
    printf("%s", question);
    free(question);
 
    printf("Raspunsul clientului: ");
    fgets(user_response, sizeof(user_response), stdin);

    int user_responseLen = strlen(user_response);

    if (send(sd, &user_responseLen, sizeof(user_responseLen), 0) < 0)
    {
      perror("[client]Eroare la send() spre server.\n");
      return errno;
    }

    if (send(sd, user_response, user_responseLen, 0) < 0)
    {
      perror("[client]Eroare la send() spre server.\n");
      return errno;
    }


    int correct_responseLen;
    if (recv(sd, &correct_responseLen, sizeof(correct_responseLen), 0) < 0)
    {
      perror("[client]Eroare la recv() de la server.\n");
      return errno;
    }
   
    char *correct_response=malloc(correct_responseLen+1);
    memset(correct_response, 0 , correct_responseLen+1);
      if (recv(sd, correct_response, correct_responseLen, 0) < 0)
    {
      perror("[client]Eroare la recv() de la server.\n");
      return errno;
    }
    correct_response[correct_responseLen]='\0';
    printf("[client] Am citit raspunsul de la server. %s\n", correct_response);
    
    if(strstr(correct_response,"Clasament Top 3")!=NULL){
      printf("Jocul s-a terminat!\n");
       close(sd);
     connected=false;
    }
    free(correct_response);
     printf("--------------------------------------------------------------------------------------\n");
  }
  close(sd);
}
