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
#include <stdbool.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/select.h>

#define MAX_SIZE 1024

#define PORT 2909

#define WAITING 10
#define RESPONSE_TIME 10


extern int errno;

static void *treat(void *); 
typedef struct thData
{
  int idThread; // id-ul thread-ului tinut in evidenta de acest program
  int cl;       // descriptorul intors de accept
  bool isFinished;
} thData;
typedef struct
{
  pthread_t idThread; // id-ul thread-ului
  int thCount;        // nr de conexiuni servite

} Thread;

Thread *threadsPool; // un array de structuri Thread

int sd, ok = 0; // descriptorul de socket de ascultare
int connected = 0, finished = 0, totalClients = 0;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t countMutex;

          
pthread_mutex_t mlock = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t clients = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t clientsTotal = PTHREAD_MUTEX_INITIALIZER;
sqlite3 *db; 

int selectFunction(int sd);
int login(int sd);
int get_user_id(const char *username);
int update_score(int id_user, int new_score);
int get_score(int id_user);
int count_users(sqlite3 *db);
char *getRankingString(sqlite3 *db);
void threadCreate(int i);
void deleteAndReplace(sqlite3 *db);
void waitingClients();





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
  int user_count = count_users(db);
  fprintf(stderr, "%d", user_count);
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
for(;;)
  {
    thData *td;
    int client;
    int length = sizeof(from);
    printf("[server]Asteptam la portul %d...\n", PORT);
    fflush(stdout);
    if ((client = accept(sd, (struct sockaddr *)&from, &length)) < 0)
    {
      perror("[server]Eroare la accept().\n");
      continue;
    }

    pthread_mutex_lock(&clientsTotal);
    totalClients++;
    pthread_mutex_unlock(&clientsTotal);

    td = (struct thData *)malloc(sizeof(struct thData));
    td->cl = client;
    td->idThread = i++;

    td->isFinished = false;
    pthread_create(&th[i], NULL, &treat, td);
  }

  sqlite3_close(db);
};






int selectFunction(int sd)
{
  struct timeval tv;
  fd_set read;

  tv.tv_sec = 20;
  tv.tv_usec = 0;

  FD_ZERO(&read);
  FD_SET(sd, &read);

  int selectResult = select(sd + 1, &read, NULL, NULL, &tv);

  return selectResult;
}


int login(int sd)
{
  int id_user = -1, loginStatus = 0;
  char username[MAX_SIZE];
  bzero(username, MAX_SIZE);

  if (recv(sd, username, sizeof(username) - 1, 0) < 0)
  {
    perror("Eroare la login!\n");
    return -1;
  }
  username[strcspn(username,"\n")]='\0';
  fprintf(stderr, "%s\n", username);

  fprintf(stderr, "Am primit mesaj de logare\n");
  pthread_mutex_lock(&db_mutex);
  const char *query = "INSERT INTO users_1 (username) VALUES (?);";
  sqlite3_stmt *stmtLogin, *stmtLogin2;
  if (sqlite3_prepare_v2(db, query, -1, &stmtLogin, NULL) != SQLITE_OK)
  {
    perror("Eroare la pregatirea interogarii\n");
    sqlite3_close(db);
  }
  sqlite3_bind_text(stmtLogin, 1, username, -1, SQLITE_STATIC);
  if (sqlite3_step(stmtLogin) != SQLITE_DONE)
  {
    fprintf(stderr, "Eroare la inserarea datelor: %s\n", sqlite3_errmsg(db));
    return -1;
  }

  fprintf(stderr, "[server]: Inregistrare reusita\n");
  sqlite3_finalize(stmtLogin);
  pthread_mutex_unlock(&db_mutex);

  pthread_mutex_lock(&db_mutex);
  const char *select_query = "SELECT id_user FROM users_1 WHERE username = ?;";
  if (sqlite3_prepare_v2(db, select_query, -1, &stmtLogin2, NULL) != SQLITE_OK)
  {
    fprintf(stderr, "Eroare la pregătirea interogării: %s\n", sqlite3_errmsg(db));
    return -1;
  }

  sqlite3_bind_text(stmtLogin2, 1, username, -1, SQLITE_STATIC);
  if (sqlite3_step(stmtLogin2) == SQLITE_ROW)
    id_user = sqlite3_column_int(stmtLogin2, 0);
  sqlite3_finalize(stmtLogin2);
  pthread_mutex_unlock(&db_mutex);
  return id_user;
}

int get_user_id(const char *username)
{
  sqlite3_stmt *stmt;
  int id_user = -1;
  pthread_mutex_lock(&db_mutex);
  const char *query = "SELECT id_user FROM users_1 WHERE username = ?;";
  if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK)
  {
    fprintf(stderr, "Eroare la pregătirea interogării: %s\n", sqlite3_errmsg(db));
    return id_user;
  }

  sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
  if (sqlite3_step(stmt) == SQLITE_ROW)
  {
    id_user = sqlite3_column_int(stmt, 0);
  }

  sqlite3_finalize(stmt);
  pthread_mutex_unlock(&db_mutex);
  return id_user;
}

int update_score(int id_user, int new_score)
{
  sqlite3_stmt *stmtUpdate;
  int result;
  pthread_mutex_lock(&db_mutex);
  const char *firstQuery = "UPDATE users_1 SET score=0 WHERE id_user=?;";
  const char *query = "UPDATE users_1 SET score = score + ? WHERE id_user = ?;";
  if (sqlite3_prepare_v2(db, query, -1, &stmtUpdate, NULL) != SQLITE_OK)
  {
    fprintf(stderr, "Eroare la pregătirea interogării: %s\n", sqlite3_errmsg(db));
  }
  else
  {
    sqlite3_bind_int(stmtUpdate, 1, new_score);
    sqlite3_bind_int(stmtUpdate, 2, id_user);
    if (sqlite3_step(stmtUpdate) != SQLITE_DONE)
    {
      fprintf(stderr, "Eroare la actualizarea scorului: %s\n", sqlite3_errmsg(db));
      result = -1;
    }
    else
    {
      fprintf(stderr, "[server]: Scor actualizat cu succes pentru id_user %d\n", id_user);
      result = 1;
    }
    sqlite3_finalize(stmtUpdate);
    pthread_mutex_unlock(&db_mutex);
  }
  return result;
}

int get_score(int id_user)
{
  sqlite3_stmt *stmtScore;
  int score = 0;
  pthread_mutex_lock(&db_mutex);
  const char *query = " SELECT score FROM users_1 WHERE id_user= ?;";
  if (sqlite3_prepare_v2(db, query, -1, &stmtScore, NULL) != SQLITE_OK)
  {
    fprintf(stderr, "Eroare la pregătirea interogării: %s\n", sqlite3_errmsg(db));
  }
  sqlite3_bind_int(stmtScore, 1, id_user);
  if (sqlite3_step(stmtScore) == SQLITE_ROW)
  {
    score = sqlite3_column_int(stmtScore, 0);
  }
  sqlite3_finalize(stmtScore);
  pthread_mutex_unlock(&db_mutex);
  return score;
}

int count_users(sqlite3 *db)
{
  sqlite3_stmt *stmt;
  int count = 0;
  pthread_mutex_lock(&db_mutex);
  const char *query = "SELECT COUNT(*) FROM users_1;";

  if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK)
  {
    fprintf(stderr, "Eroare la pregătirea interogării: %s\n", sqlite3_errmsg(db));
    return -1;
  }

  if (sqlite3_step(stmt) == SQLITE_ROW)
  {
    count = sqlite3_column_int(stmt, 0);
  }

  sqlite3_finalize(stmt);
  pthread_mutex_unlock(&db_mutex);
  return count;
}

void displayUsers(sqlite3 *db)
{ 

  sqlite3_stmt *stmt;
  pthread_mutex_lock(&db_mutex);
  const char *sql = "SELECT *  FROM users_1 ORDER BY score DESC LIMIT 3;";

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
  {
    fprintf(stderr, "Eroare la pregătirea interogării: %s\n", sqlite3_errmsg(db));
    return;
  }

  fprintf(stderr, "Lista utilizatorilor:\n");
  while (sqlite3_step(stmt) == SQLITE_ROW)
  {
    int id = sqlite3_column_int(stmt, 0);
    const unsigned char *username = sqlite3_column_text(stmt, 1);
    fprintf(stderr, "ID: %d, Username: %s\n", id, username);
  }

  sqlite3_finalize(stmt);
  pthread_mutex_unlock(&db_mutex);
}

char *getRankingString(sqlite3 *db)
{
  sqlite3_stmt *stmt;
  pthread_mutex_lock(&db_mutex);
  const char *sql = "SELECT id_user, username, score FROM users_1 ORDER BY score DESC LIMIT 3;";

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
  {
    fprintf(stderr, "Eroare la pregătirea interogării: %s\n", sqlite3_errmsg(db));
    pthread_mutex_unlock(&db_mutex);
    perror("Eroare la obținerea clasamentului.");
  }

  char *clasament = malloc(1024);
  if (!clasament)
  {
    sqlite3_finalize(stmt);
    pthread_mutex_unlock(&db_mutex);
  }
  clasament[0] = '\0';

  size_t folosit = 0;         
  size_t alocat = 1024; 

  int rowCount = 0;
  folosit += snprintf(clasament + folosit, alocat - folosit, "Clasament Top 3 Utilizatori:\n");

  while (sqlite3_step(stmt) == SQLITE_ROW && folosit< alocat)
  {
    int id = sqlite3_column_int(stmt, 0);
    const char *username = (const char *)sqlite3_column_text(stmt, 1);
    int score = sqlite3_column_int(stmt, 2);

    int needed = snprintf(NULL, 0, " ID user: %d. Username: %s Scor: %d",id,  username, score);

    if (folosit + needed >= alocat)
    {
      alocat *= 2;
      char *clasaamentNou = realloc(clasament, alocat);
      if (!clasaamentNou)
      {
        free(clasament);
        sqlite3_finalize(stmt);
        pthread_mutex_unlock(&db_mutex);
        return NULL;
      }
      clasament = clasaamentNou;
    }

    folosit += snprintf(clasament + folosit, alocat - folosit, "ID user: %d. Username: %s, Scor: %d\n", id, username, score);
    rowCount++;
  }

  if (rowCount == 0)
    folosit += snprintf(clasament + folosit, alocat - folosit, "Nu există utilizatori în clasament.\n");

  sqlite3_finalize(stmt);
  pthread_mutex_unlock(&db_mutex);

  return clasament;
}

void threadCreate(int i)
{
  void *treat(void *);
  pthread_create(&threadsPool[i].idThread, NULL, &treat, (void *)i);
  return;
}


void deleteAndReplace(sqlite3 *db)
{
  sqlite3_stmt *stmtDel, *stmtRepl;
  char *errMsg = NULL;
  pthread_mutex_lock(&db_mutex);
  const char *delete = "DROP TABLE users_1";
  if (sqlite3_prepare_v2(db, delete, -1, &stmtDel, NULL) != SQLITE_OK)
    fprintf(stderr, "Eroare la pregatirea interogarii!\n");
  else
  {
    if (sqlite3_step(stmtDel) != SQLITE_DONE)
      fprintf(stderr, "Eroare la stergerea datelor!\n");
  }
  sqlite3_finalize(stmtDel);
  pthread_mutex_unlock(&db_mutex);

  pthread_mutex_lock(&db_mutex);
  const char *replace = "CREATE TABLE IF NOT EXISTS users_1 (id_user INTEGER PRIMARY KEY AUTOINCREMENT , username VARCHAR(1024), score INTEGER DEFAULT 0);";
  if (sqlite3_exec(db, replace, 0, 0, &errMsg) != SQLITE_OK)
  {
    fprintf(stderr, "Eroare la creare: %s\n", errMsg);
    sqlite3_free(errMsg);
  }
  else
    fprintf(stderr, "Tabel creat cu succes!\n");
}

void waitingClients()
{

  pthread_mutex_lock(&clients);
  connected++;
  if (connected < 3)
  {
    fprintf(stderr, "[server] - Astept alti clienti...\n!");
    while (connected < 3)
      pthread_cond_wait(&cond, &clients);
  }
  pthread_mutex_unlock(&clients);
  if (connected >= 3)
    pthread_cond_broadcast(&cond);
}

void *treat(void *arg)
{ 
  struct thData tdL;
  tdL = *((thData *)arg);
  pthread_t timer;
  int id = 0, i = 0, count = 0, loginStatus = 0, resultSelect = 0, score = 0;
  char message[MAX_SIZE];

  char login_response[MAX_SIZE];

  bzero(login_response, MAX_SIZE);
  printf("[Thread %d] - Asteptam mesajul...\n", tdL.idThread);
  fflush(stdout);
  int id_user = login(tdL.cl);
  printf("[Thread %d] - Mesajul a fost receptionat...%s\n", tdL.idThread, login_response);

  pthread_detach(pthread_self());

  snprintf(login_response, MAX_SIZE, "[server] Clientul cu ID ul %d s-a inregistrat cu succes\n", id_user);
  send(tdL.cl, login_response, sizeof(login_response), 0);

  fprintf(stderr, "[server] Am trimis raspunsul mai departe\n");

  char readyMsg[6];
  memset(readyMsg, 0, 6);
  int len = recv(tdL.cl, readyMsg, 5, 0);
  fprintf(stderr, "%d\n", len);
  if (recv(tdL.cl, readyMsg, 5, 0) <= 0)
  {
    perror("Eroare la primirea mesajului READY de la client");
    close(tdL.cl);
    free(arg);
    return NULL;
  }
  readyMsg[5] = '\0';
  fprintf(stderr, "%s\n", readyMsg);
  if (strcmp(readyMsg, "READY") != 0)
  {
    fprintf(stderr, "Mesaj incorect de la client: %s\n", readyMsg);
    close(tdL.cl);
    free(arg);
    return NULL;
  }

  waitingClients();
  for (int i = 1; i <= 10; i++)
  {
    score = get_score(id_user);
    char question[MAX_SIZE], raspuns1[MAX_SIZE], raspuns2[MAX_SIZE], raspuns3[MAX_SIZE];
    memset(question, 0, MAX_SIZE);
    pthread_mutex_lock(&db_mutex);
    const char *interogare = "SELECT * FROM intrebari WHERE id NOT IN ( SELECT id_intrebare FROM intrebariFolosite WHERE id_user = ? AND used = 1) LIMIT 10;";
    sqlite3_stmt *stmt, *stmtUsed, *stmt1, *stmt2;
    if (sqlite3_prepare_v2(db, interogare, -1, &stmt, NULL) != SQLITE_OK)
    {
      fprintf(stderr, "Eroare la pregătirea interogării\n");
      sqlite3_finalize(stmt);
      pthread_mutex_unlock(&db_mutex);
      continue;
    }
    sqlite3_bind_int(stmt, 1, id_user);
    if (sqlite3_step(stmt) != SQLITE_ROW)
    {
      fprintf(stderr, "Eroare la preluarea întrebării din baza de date\n");
      sqlite3_finalize(stmt);
      pthread_mutex_unlock(&db_mutex);
      continue;
    }

    id = sqlite3_column_int(stmt, 0);
    snprintf(question, MAX_SIZE, "%s\nA. %s\nB. %s\nC. %s\n",   sqlite3_column_text(stmt, 1),  sqlite3_column_text(stmt, 2),  
                                                                sqlite3_column_text(stmt, 3),   sqlite3_column_text(stmt, 4));
    sqlite3_finalize(stmt);

    const char *update = "INSERT INTO intrebariFolosite ( id_user, id_intrebare, used) VALUES (?,?, 1) ON CONFLICT (id_user,id_intrebare) DO UPDATE SET used = 1;";
    if (sqlite3_prepare_v2(db, update, -1, &stmtUsed, NULL) != SQLITE_OK)
    {
      fprintf(stderr, "Eroare la pregătirea interogării: %s\n", sqlite3_errmsg(db));
    }
    sqlite3_bind_int(stmtUsed, 1, id_user);
    sqlite3_bind_int(stmtUsed, 2, id);
    if (sqlite3_step(stmtUsed) != SQLITE_DONE)
      fprintf(stderr, "Eroare la actualizarea starii %s\n", sqlite3_errmsg(db));
    

    fprintf(stderr, "%s\n", question);
    int questionLen = strlen(question);
    send(tdL.cl, &questionLen, sizeof(questionLen), 0);
    send(tdL.cl, question, questionLen, 0);
    pthread_mutex_unlock(&db_mutex);

    resultSelect = selectFunction(tdL.cl);

    if (resultSelect == -1)
      perror("Eroare la select()\n");
    else if (resultSelect == 0)
    {
      printf("Timpul a expirat.Trimit alta intrebare\n");
      char scoreMsg[MAX_SIZE];
      memset(scoreMsg, 0, MAX_SIZE);
      snprintf(scoreMsg, MAX_SIZE, "Timpul acordat pentru a raspunde la intrebare a expirat!\n ");

      int scoreMsgLen = strlen(scoreMsg);
      send(tdL.cl, &scoreMsgLen, sizeof(scoreMsgLen), 0);
      send(tdL.cl, scoreMsg, scoreMsgLen, 0);
      continue;
    }
    else

    {
      int user_responseLen;
      recv(tdL.cl, &user_responseLen, sizeof(user_responseLen), 0);
      char *user_response = malloc(user_responseLen + 1);

      memset(user_response, 0, user_responseLen + 1);
      if (recv(tdL.cl, user_response, user_responseLen, 0) <= 0)
      {
        perror("Eroare la recv()!\n");
        update_score(id_user, 0);
        continue;
      }
      else
      {
        user_response[user_responseLen] = '\0';
        printf("[server]Am primit raspunsul de la client %s \n", user_response);

        if (strstr(user_response, "quit") != NULL)
        {
          fprintf(stderr, "Jucatorul %d a parasit jocul si a obtinut %d puncte\n", id_user, score);

          score = get_score(id_user);

          char scoreMsg[MAX_SIZE];
          memset(scoreMsg, 0, MAX_SIZE);
          snprintf(scoreMsg, MAX_SIZE, "Ai parasit sesiunea de joc inainte de final. Scorul tau final este: %d\n", score);

          int scoreMsgLen = strlen(scoreMsg);
          send(tdL.cl, &scoreMsgLen, sizeof(scoreMsgLen), 0);
          send(tdL.cl, scoreMsg, scoreMsgLen, 0);
          totalClients--;
          close(tdL.cl);
          return NULL;
        }

        pthread_mutex_lock(&db_mutex);
        const char *correct_answer = "SELECT raspuns_corect FROM intrebari WHERE id = ?";
        char correct[MAX_SIZE];
        if (sqlite3_prepare_v2(db, correct_answer, -1, &stmt1, NULL) == SQLITE_OK)
        {
          sqlite3_bind_int(stmt1, 1, id);
          fprintf(stderr, "%d\n", id);
          if (sqlite3_step(stmt1) == SQLITE_ROW)
            snprintf(correct, MAX_SIZE, "%s\n", sqlite3_column_text(stmt1, 0));
          else
            fprintf(stderr, "Eroare la citirea raspunsului corect din baza de date\n");
        }
        sqlite3_finalize(stmt1);
        pthread_mutex_unlock(&db_mutex);

        pthread_mutex_lock(&db_mutex);
        const char *compare = "SELECT raspuns1, raspuns2, raspuns3 FROM intrebari WHERE id = ?";
        if (sqlite3_prepare_v2(db, compare, -1, &stmt2, NULL) == SQLITE_OK)
        {
          sqlite3_bind_int(stmt2, 1, id);
          fprintf(stderr, "%d\n", id);
          if (sqlite3_step(stmt2) == SQLITE_ROW)
          {
            snprintf(raspuns1, MAX_SIZE, "%s\n", sqlite3_column_text(stmt2, 0));
            snprintf(raspuns2, MAX_SIZE, "%s\n", sqlite3_column_text(stmt2, 1));
            snprintf(raspuns3, MAX_SIZE, "%s\n", sqlite3_column_text(stmt2, 2));
          }
          else
            fprintf(stderr, "Eroare la citirea raspunsurilor din baza de date\n");
        }
        /*fprintf(stderr, "%s \n", raspuns1);
        fprintf(stderr, "%s \n", raspuns2);
        fprintf(stderr, "%s \n", raspuns3);*/

        sqlite3_finalize(stmt2);
        pthread_mutex_unlock(&db_mutex);
        if (strcmp(correct, raspuns1) == 0)
        {
          snprintf(correct, MAX_SIZE, "A");
        }
        else if (strcmp(correct, raspuns2) == 0)
        {
          snprintf(correct, MAX_SIZE, "B");
        }
        else if (strcmp(correct, raspuns3) == 0)
          snprintf(correct, MAX_SIZE, "C");
        fprintf(stderr, "%s \n", correct);

        char message[MAX_SIZE];
        memset(message, 0, MAX_SIZE);
        if (strstr(user_response, correct) != NULL)
        {
          snprintf(message, MAX_SIZE, "Răspunsul este corect!\n");
          update_score(id_user, 100);
          int messageLen = strlen(message);
          send(tdL.cl, &messageLen, sizeof(messageLen), 0);
          send(tdL.cl, message, messageLen, 0);
        }
        else
        {
          snprintf(message, MAX_SIZE, "Răspunsul este gresit!\n");
          int messageLen = strlen(message);
          send(tdL.cl, &messageLen, sizeof(messageLen), 0);
          send(tdL.cl, message, messageLen, 0);
         
        }
        free(user_response);
      }
      sqlite3_finalize(stmtUsed);
      pthread_mutex_unlock(&db_mutex);
    }
  }

  pthread_mutex_lock(&clients);
  tdL.isFinished = true;
  finished++;
  if (finished == totalClients)
    pthread_cond_broadcast(&cond);

  pthread_mutex_unlock(&clients);

  fprintf(stderr, "Clienti care au terminat %d SI CLIENTII TOTALI %d\n", finished, totalClients);

  pthread_mutex_lock(&clients);
  while (finished < totalClients)
  {
    pthread_cond_wait(&cond, &clients);
  }
  pthread_mutex_unlock(&clients);

  char *ranking = getRankingString(db);
  fprintf(stderr, "%s", ranking);
  int rankingLen=strlen(ranking);
  if(send(tdL.cl, &rankingLen, sizeof(rankingLen), 0)<0)
  perror("Eroare la send()");

  if(send(tdL.cl,ranking, rankingLen,0)<0)
      perror("Eroare la send()");

  free(ranking);
  close(tdL.cl);
  free(arg);
    deleteAndReplace(db);
 return NULL;
}
