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
#include <bits/stdc++.h>
#include <mysql/mysql.h>

using namespace std;

// Structuri de date pentru interactiunea cu mySQL db
MYSQL *conn;
MYSQL_RES *res;
MYSQL_ROW row;

/* Parametri de conectare la baza de date */
const char *server = "localhost";
const char *user = "Nicu";
const char *password = "Nicu.123";
const char *database = "users";

/* portul folosit */
#define PORT 2016
vector <int> clienti;
vector <bool> connected;

bool lacat = false;

typedef struct thData{
	int idThread; //id-ul thread-ului tinut in evidenta de acest program
	int cl; //descriptorul intors de accept
    bool connected;
}thData;

static void *search(void *); 
static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);

void insertAccidentInDB(string street)
{
    int pos = street.find('-');

    string point1 = street.substr(0, pos), point2 = street.substr(pos + 1, street.length() - pos);

    string query = "insert into accidents values (" + point1 + ", " + point2 + ", NOW())";

    if (mysql_query(conn, query. c_str())) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }
    
}

bool searchLoginInDataBase(string login, string & currentPassword)
{

    bool found = false;

    /* send SQL query */
    if (mysql_query(conn, "select * from connection")) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }
    // Rezultatul unei interogari
    res = mysql_use_result(conn);

    while ((row = mysql_fetch_row(res)) != NULL) {

        string dbLogin = row[1];

        if(dbLogin == login) {
            currentPassword = row[2];
            found = true;
            break;
        }
    }

    mysql_free_result(res);

    return found;
}

void connectToDataBase()
{
    conn = mysql_init(NULL);

    /* Connect to database */
    if (!mysql_real_connect(conn, server,
            user, password, database, 0, NULL, 0)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }
}

void closeDataBase()
{
    /* close connection */
    
    mysql_close(conn);
}

void deleteRow(string time)
{
    // Structuri de date pentru interactiunea cu mySQL db
    MYSQL *conn2;
    MYSQL_RES *res2;
    MYSQL_ROW row2;

    /* Parametri de conectare la baza de date */
    char *server2 = "localhost";
    char *user2 = "Nicu";
    char *password2 = "Nicu.123";
    char *database2 = "users";

     //printf("ok\n");

    conn2 = mysql_init(NULL);
    
    /* Connect to database */
    if (!mysql_real_connect(conn2, server2,
            user2, password2, database2, 0, NULL, 0)) {
        fprintf(stderr, "%s\n", mysql_error(conn2));
        exit(1);
    }

    string query = "delete from accidents where time = '" + time + "'"; 

    if (mysql_query(conn2, query.c_str())) {
        fprintf(stderr, "%s\n", mysql_error(conn2));
        exit(1);
    }

    mysql_close(conn2);
}

void getInfoFromDB(string &infos)
{

    string query = "select * from info where day=date_format(now(), '%W')"; 

    if (mysql_query(conn, query.c_str())) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }
    // Rezultatul unei interogari
    res = mysql_use_result(conn);

    while ((row = mysql_fetch_row(res)) != NULL) {
        infos = infos + row[1] + ":" + row[2] + ":" + row[3];
    }

    mysql_free_result(res);
}

void registerUser(string login, string password)
{
    string query = "insert into connection(login, password) values ('" + login + "','" + password + "')";

    if (mysql_query(conn, query.c_str())) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }
}

string getSpeedFromDB(string p1, string p2)
{
    string regularSpeed = "";
    string query = "select speed from speed where point1=" + p1 + " and point2=" + p2; 

    if (mysql_query(conn, query.c_str())) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }
    // Rezultatul unei interogari
    res = mysql_use_result(conn);

    while ((row = mysql_fetch_row(res)) != NULL) {
        regularSpeed = row[0];
    }

    mysql_free_result(res);

    return regularSpeed;
}

int main ()
{
    struct sockaddr_in server;	// structura folosita de server
    struct sockaddr_in from;	
    int sd;		//descriptorul de socket 
    int pid;
    pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
    int i=0;

    connectToDataBase();

    /* crearea unui socket */
    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror ("[server]Eroare la socket().\n");
        exit(1);
    }

    /* pregatirea structurilor de date */
    bzero (&server, sizeof (server));
    bzero (&from, sizeof (from));

    /* umplem structura folosita de server */
    /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;	
    /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
    /* utilizam un port utilizator */
    server.sin_port = htons (PORT);

    /* atasam socketul */
    if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
        perror ("[server]Eroare la bind().\n");
        exit(1);
    }

    /* punem serverul sa asculte daca vin clienti sa se conecteze */
    if (listen (sd, 2) == -1)
    {
        perror ("[server]Eroare la listen().\n");
        exit(1);
    }

    /* Thread care cauta update-uri in baza de date */
    thData * td1; 
    td1 = (struct thData*) malloc(sizeof(struct thData));
    td1->idThread=777;
    pthread_create(&th[i], NULL, &search, td1);

    /* servim in mod concurent clientii...folosind thread-uri */
    while (1)
    {
        int client;
        thData * td; //parametru functia executata de thread     
        unsigned int length = sizeof (from);

        /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
        if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0)
        {
            perror ("[server]Eroare la accept().\n");
            continue;
        }

            /* s-a realizat conexiunea, se astepta mesajul */

        fflush(stdout);
        printf("Connected!\n");

        td=(struct thData*)malloc(sizeof(struct thData));	
        td->idThread=i++;
        td->cl=client;
        td->connected=false;

        clienti.push_back(client);
        connected.push_back(false);

        pthread_create(&th[i], NULL, &treat, td);	      
                
    }//while   

    closeDataBase(); 
}

static void *treat(void * arg)
{		
    struct thData tdL; 
    tdL= *((struct thData*)arg);		 

    pthread_detach(pthread_self());		
    raspunde((struct thData*)arg);

    /* am terminat cu acest client, inchidem conexiunea */
    close ((intptr_t)arg);
    return(NULL);	
  		
}

static void *search(void * arg)
{
    struct thData tdL; 
	tdL= *((struct thData*)arg);

    pthread_detach(pthread_self());

    // Structuri de date pentru interactiunea cu mySQL db
    MYSQL *conn1;
    MYSQL_RES *res1;
    MYSQL_ROW row1;

    /* Parametri de conectare la baza de date */
    char *server1 = "localhost";
    char *user1 = "Nicu";
    char *password1 = "Nicu.123";
    char *database1 = "users";

    conn1 = mysql_init(NULL);

    /* Connect to database */
    if (!mysql_real_connect(conn1, server1,
            user1, password1, database1, 0, NULL, 0)) {
        fprintf(stderr, "%s\n", mysql_error(conn1));
        exit(1);
    }

    bool was = false;
    while(1) {
        //printf("ok\n");
        sleep(1);

        vector<string> toDelete;

        /* send SQL query */
        if (mysql_query(conn1, "select point1, point2, now()-time, time from accidents")) {
            fprintf(stderr, "%s\n", mysql_error(conn1));
            exit(1);
        }
        // Rezultatul unei interogari
        res1 = mysql_use_result(conn1);


        while ((row1 = mysql_fetch_row(res1)) != NULL) {
            string timeStr = row1[2];
            long long time = stoi(timeStr);

            int n = clienti.size();
            string str1 = row1[0], str2 = row1[1];
            string message = str1 + "-" + str2;

            if(time > 60) {
                message += 'G';

                deleteRow(row1[3]);

            }

            for(int i = 0; i < n; ++i)
                if(connected[i]) {
                    while(lacat);
                    lacat = true;
                    write(clienti[i], message.c_str(), message.length());
                    lacat = false;
                }
            
            sleep(1);
        }

        mysql_free_result(res1);

        for(int i = 0; i < toDelete.size(); ++i)
            deleteRow(toDelete[i]);
    }

    close ((intptr_t)arg);
    mysql_close(conn1);
	return(NULL);	
}

void raspunde(void *arg)
{
    int nr, i=0;
	struct thData tdL; 
	tdL= *((struct thData*)arg);

    char message[100];	

    while(1) {
        bzero(message, 100);
        int len = read(tdL.cl, message, 100);

        if(!len) {
            fflush(stdout);
            printf("Disconnected\n");
            break;
        }

        string aux = message;

        int pos = aux.find(':');

        string protocol = aux.substr(0, pos);
        
        if(protocol == "login") {
            string login = aux.substr(pos + 1, aux.length() - pos);
            
            string correctPassword = "";

            fflush(stdout);
            printf("|%s|\n", login.c_str());

            if(searchLoginInDataBase(login, correctPassword))
                write(tdL.cl, "OK", 2);
            else {
                write(tdL.cl, "NO", 2);
                continue;
            }
            //waiting for password
            char passwordC[200];
            bzero(passwordC, 200);
            read(tdL.cl, passwordC, 200);

            string passUser = passwordC;

            if(passUser == correctPassword) {
                connected[tdL.idThread] = true;
                write(tdL.cl, "OK", 2);
            } else 
                write(tdL.cl, "NO", 2);
        } else 
        if(protocol == "accident") {
            string street = aux.substr(pos + 1, aux.length() - pos);
            insertAccidentInDB(street);
        } else 
        if(protocol == "speed") {
            string speed = aux.substr(pos + 1, aux.length() - pos);

            printf("%s\n", speed.c_str());
        } else 
        if(protocol == "info") {
            string infos = "";

            getInfoFromDB(infos);

            infos = to_string(infos.length()) + ":" + infos;
            fflush(stdout);
            printf("%s\n", infos.c_str());

            while(lacat);
            lacat = true;
            write(tdL.cl, infos.c_str(), infos.length());
            lacat = false;

        } else 
        if(protocol == "register") {

            int pos1 = pos + 1;
            while(aux[pos1] != ':') ++pos1;

            string login = aux.substr(pos + 1, pos1 - pos - 1);
            string passwd = aux.substr(pos1 + 1, aux.length() - pos1);

            registerUser(login, passwd);

        } else 
        if(protocol == "reqSpeed") {
            // fflush(stdout);
            // printf("%s\n", message);

            int pos1 = pos + 1;
            while(aux[pos1] != ':') ++pos1;

            string point1 = aux.substr(pos + 1, pos1 - pos - 1);
            string point2 = aux.substr(pos1 + 1, aux.length() - pos1);

            string speed = getSpeedFromDB(point1, point2);

            // fflush(stdout);
            // printf("|%s|\n", speed.c_str());

            while(lacat);
            lacat = true;
            write(tdL.cl, speed.c_str(), speed.length());
            lacat = false;
        }
    }
}