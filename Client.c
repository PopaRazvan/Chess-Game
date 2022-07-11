#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;

char Table[11][11];
char s[1000];

void decodificare_tabla(char s[], char Table[11][11])
{
    int k = 0;
    for (int i = 1; i <= 9; i++)
    {
        for (int j = 0; j <= 8; j++)
            Table[i][j] = s[k++];
    }
}

void Print(char Table[11][11])
{

    printf("%s\n", "============================================");

    for (int i = 1; i <= 8; i++)
    {
        for (int j = 0; j <= 8; j++)
            if (j == 0)
                printf("%c   ", Table[i][j]);
            else
                printf("| %c |", Table[i][j]);

        printf("\n");
    }
    printf("%s\n", "============================================");
    printf("      ");
    for (int j = 1; j <= 8; j++)
        printf("%c    ", Table[9][j]);

    printf("\n\n");
}

int main(int argc, char *argv[])
{
    int sd;                    // descriptorul de socket
    struct sockaddr_in server; // structura folosita pentru conectare
    char msg[1000];            // mesajul trimis

    /* exista toate argumentele in linia de comanda? */
    if (argc != 3)
    {
        printf("[client] Sintaxa: %s <adresa_server> <port>\n", argv[0]);
        return -1;
    }

    /* stabilim portul */
    port = atoi(argv[2]);

    /* cream socketul */
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("[client] Eroare la socket().\n");
        return errno;
    }

    /* umplem structura folosita pentru realizarea conexiunii cu serverul */
    /* familia socket-ului */
    server.sin_family = AF_INET;
    /* adresa IP a serverului */
    server.sin_addr.s_addr = inet_addr(argv[1]);
    /* portul de conectare */
    server.sin_port = htons(port);

    /* ne conectam la server */
    if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("[client]Eroare la connect().\n");
        return errno;
    }

    while (1)
    {
        int ok = 0;

        while (ok == 0)
        {

            printf("\n\n==========================/ CHESS /==========================\n\n");
            printf("Comenzi:\n");
            printf(" 1. play \n");
            printf(" 2. quit \n\n");

            printf("[client]Introduceti comanda: ");

            fflush(stdout);
            bzero(msg, 1000);
            read(0, msg, 1000);

            if (write(sd, msg, 1000) < 0)
            {
                perror("Eroare la write()\n");
                return 0;
            }

            if (strstr(msg, "quit"))
            {
                close(sd);
                return 0;
            }
            else if (strstr(msg, "play"))
            {
                if (read(sd, msg, 1000) < 0)
                {
                    perror("Eroare la read()\n");
                    return 0;
                }

                if (strstr(msg, "Cautare jucator..."))
                {
                    printf("[client]Se asteapta oponent!\n\n");
                    ok = 1;
                }
            }
        }

        int game = 1;
        bzero(msg, 1000);

        if (read(sd, msg, 1000) < 0)
        {
            perror("[client]Eroare la read() de la server.\n");
            return errno;
        }

        if (strstr(msg, "Ai castigat") != NULL)
        {
            printf("%s\n", "[client]Ai castigat!");
            game = 0;
            if (read(sd, s, 1000) < 0)
            {
                perror("[client]Eroare la read() de la server.\n");
                return errno;
            }
            decodificare_tabla(s, Table);
            Print(Table);
            continue;
        }
        else if (strstr(msg, "Ai pierdut") != NULL)
        {
            printf("%s\n", "[client]Ai pierdut!");
            game = 0;
            if (read(sd, s, 1000) < 0)
            {
                perror("[client]Eroare la read() de la server.\n");
                return errno;
            }
            decodificare_tabla(s, Table);
            Print(Table);
            continue;
        }
        else
            printf("[client]A inceput meciul\n\n");

        if (read(sd, s, 1000) < 0)
        {
            perror("[client]Eroare la read() de la server.\n");
            return errno;
        }

        if (strstr(s, "Ai castigat"))
        {
            printf("%s\n", "[client]Ai castigat!");
            game = 0;
            if (read(sd, s, 1000) < 0)
            {
                perror("[client]Eroare la read() de la server.\n");
                return errno;
            }
            decodificare_tabla(s, Table);
            Print(Table);
            continue;
        }

        if (strstr(s, "Ai pierdut") != NULL)
        {
            printf("%s\n", "[client]Ai pierdut!");
            game = 0;
            if (read(sd, s, 1000) < 0)
            {
                perror("[client]Eroare la read() de la server.\n");
                return errno;
            }
            decodificare_tabla(s, Table);
            Print(Table);
            continue;
        }

        decodificare_tabla(s, Table);
        Print(Table);

        while (game == 1)
        {

            bzero(msg, 1000);
            printf("[client]Introduceti mutarea: ");
            fflush(stdout);
            read(0, msg, 1000);

            if (write(sd, msg, 1000) <= 0)
            {
                perror("[client]Eroare la write() spre server.\n");
                return errno;
            }

            if (strstr(msg, "surrender\n"))
            {
                printf("[client]Ai renuntat!\n");
                game = 0;
                continue;
            }

            if (read(sd, msg, 1000) < 0)
            {
                perror("[client]Eroare la read() de la server.\n");
                return errno;
            }

            if (strstr(msg, "Ai castigat") != NULL)
            {
                printf("%s\n", "[client]Ai castigat!");
                game = 0;
                if (read(sd, s, 1000) < 0)
                {
                    perror("[client]Eroare la read() de la server.\n");
                    return errno;
                }
                decodificare_tabla(s, Table);
                Print(Table);
                continue;
            }
            else if (strstr(msg, "Ai pierdut") != NULL)
            {
                printf("%s\n", "[client]Ai pierdut!");
                game = 0;
                if (read(sd, s, 1000) < 0)
                {
                    perror("[client]Eroare la read() de la server.\n");
                    return errno;
                }
                decodificare_tabla(s, Table);
                Print(Table);
                continue;
            }
            else
                printf("[client]%s\n\n", msg);

            if (read(sd, s, 1000) < 0)
            {
                perror("[client]Eroare la read() de la server.\n");
                return errno;
            }

            if (strstr(s, "Ai castigat") != NULL)
            {
                printf("%s\n", "[client]Ai castigat!");
                game = 0;
                if (read(sd, s, 1000) < 0)
                {
                    perror("[client]Eroare la read() de la server.\n");
                    return errno;
                }
                decodificare_tabla(s, Table);
                Print(Table);
                continue;
            }

            if (strstr(s, "Ai pierdut") != NULL)
            {
                printf("%s\n", "[client]Ai pierdut!");
                game = 0;
                if (read(sd, s, 1000) < 0)
                {
                    perror("[client]Eroare la read() de la server.\n");
                    return errno;
                }
                decodificare_tabla(s, Table);
                Print(Table);
                continue;
            }

            decodificare_tabla(s, Table);
            Print(Table);

            if (!strstr(msg, "invalid"))
            {

                if (read(sd, s, 1000) < 0)
                {
                    perror("[client]Eroare la read() de la server.\n");
                    return errno;
                }

                if (strstr(s, "Ai castigat") != NULL)
                {
                    printf("%s\n", "[client]Ai castigat!");
                    game = 0;
                    if (read(sd, s, 1000) < 0)
                    {
                        perror("[client]Eroare la read() de la server.\n");
                        return errno;
                    }
                    decodificare_tabla(s, Table);
                    Print(Table);
                    continue;
                }

                if (strstr(s, "Ai pierdut") != NULL)
                {
                    printf("%s\n", "[client]Ai pierdut!");
                    game = 0;
                    if (read(sd, s, 1000) < 0)
                    {
                        perror("[client]Eroare la read() de la server.\n");
                        return errno;
                    }
                    decodificare_tabla(s, Table);
                    Print(Table);
                    continue;
                }
                decodificare_tabla(s, Table);
                Print(Table);
                printf("\n");
            }
        }
    }
    close(sd);
}