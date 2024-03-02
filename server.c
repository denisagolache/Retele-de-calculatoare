/* Exemplu de server TCP concurent care deserveste clientii
   prin crearea unui thread pentru fiecare client.
   Autor: Lenuta Alboaie  <adria@info.uaic.ro> (c)


   Modificarea adusa acestui program: adaugarea unei baze de date, pe care serverul o acceseaza pentru a prelua intrebari
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <sqlite3.h>
#include <arpa/inet.h>

#define MAX_SIZE 1024
/* portul folosit */
#define PORT 2909

/* codul de eroare returnat de anumite apeluri */
extern int errno;

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
typedef struct thData
{
  int idThread; // id-ul thread-ului tinut in evidenta de acest program
  int cl;       // descriptorul intors de accept

} thData;
typedef struct
{
  pthread_t idThread; // id-ul thread-ului
  int thCount;        // nr de conexiuni servite
} Thread;

Thread *threadsPool; // un array de structuri Thread

int sd;                                            // descriptorul de socket de ascultare
int nthreads;                                      // numarul de threaduri
pthread_mutex_t mlock = PTHREAD_MUTEX_INITIALIZER; // variabila mutex ce va fi partajata de threaduri
sqlite3 *db;                                       // descriptorul pentru baza de date
void raspunde(int cl, int idThread);

int main(int argc, char *argv[])
{
  struct sockaddr_in server;
  struct sockaddr_in from;
  pthread_t th[100];
  int i = 0;

  if (sqlite3_open("quizzgame_database.db", &db) != SQLITE_OK)
  {
    perror("Eroare la deschiderea bazei de date\n");
    return errno;
  }

  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("[server]Eroare la socket().\n");
    return errno;
  }

  int on = 1;
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

  bzero(&server, sizeof(server));
  bzero(&from, sizeof(from));

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  server.sin_port = htons(PORT);

  if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    perror("[server]Eroare la bind().\n");
    return errno;
  }

  if (listen(sd, 2) == -1)
  {
    perror("[server]Eroare la listen().\n");
    return errno;
  }
  for (;;)
  {
    int client;
    thData *td;
    int length = sizeof(from);
    printf("[server]Asteptam la portul %d...\n", PORT);
    fflush(stdout);
    if ((client = accept(sd, (struct sockaddr *)&from, &length)) < 0)
    {
      perror("[server]Eroare la accept().\n");
      continue;
    }
    td = (struct thData *)malloc(sizeof(struct thData));
    td->idThread = i++;
    td->cl = client;

    pthread_create(&th[i], NULL, &treat, td);
  }
  sqlite3_close(db);
};

void threadCreate(int i)
{
  void *treat(void *);

  pthread_create(&threadsPool[i].idThread, NULL, &treat, (void *)i);
  return;
}
void *treat(void *arg)
{
  struct thData tdL;
  tdL = *((struct thData *)arg);
  int logged = 0;
  printf("[Thread %d] - Asteptam mesajul...\n", tdL.idThread);
  fflush(stdout);
  printf("[Thread %d] - Mesajul a fost receptionat.\n", tdL.idThread);

  pthread_detach(pthread_self());

  char question[MAX_SIZE];
  question[0]='\0';
  while (1)
  {
    const char *interogare = "SELECT intrebare, raspuns1, raspuns2, raspuns3 FROM intrebari ORDER BY RANDOM()";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, interogare, -1, &stmt, NULL) == SQLITE_OK)
    {
      if (sqlite3_step(stmt) == SQLITE_ROW)
        snprintf(question, MAX_SIZE, "%s\nA. %s\nB. %s\nC. %s\n", sqlite3_column_text(stmt, 0), sqlite3_column_text(stmt, 1), sqlite3_column_text(stmt, 2), sqlite3_column_text(stmt, 3));
      else
        snprintf(question, MAX_SIZE, "Eroare la preluarea întrebării din baza de date");
    }
    else
      snprintf(question, MAX_SIZE, "Eroare la pregătirea interogării");

    send(tdL.cl, question, sizeof(question), 0);
    char user_response[MAX_SIZE];
    user_response[0]='\0';
    if (recv(tdL.cl, user_response, sizeof(user_response), 0) < 0)
    {
      perror("Eroare la primirea răspunsului de la client");
      break;
    }

    const char *correct_answer = "C";
    if (strstr(user_response, correct_answer) != NULL)
      send(tdL.cl, "Răspunsul este corect!\n", sizeof("Răspunsul este corect!\n"), 0);
    else
      send(tdL.cl, "Răspunsul este greșit.\n", sizeof("Răspunsul este greșit.\n"), 0);
  }
  close(tdL.cl);
  pthread_mutex_lock(&mlock);
  pthread_mutex_unlock(&mlock);
  pthread_exit(NULL);
}