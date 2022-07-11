#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>

/* portul folosit */

#define PORT 2024

extern int errno; /* eroarea returnata de unele apeluri */

int verificare = 0;
int v[200];
int jucatorul_unu;
int jucatorul_doi;
int sd;
fd_set actfds; /* multimea descriptorilor activi */

//Utilitati
//--------------------------------------------------------//

void copiere_tabla(char A[][11], char B[][11])
{
    for (int i = 1; i <= 9; i++)
        for (int j = 0; j <= 8; j++)
            A[i][j] = B[i][j];
}

void codificare_tabla_B(char s[], char Table[11][11])
{
    int k = 0;
    for (int i = 0; i < 1000; i++)
        s[i] = '\0';

    for (int i = 1; i <= 9; i++)
        for (int j = 0; j <= 8; j++)
            s[k++] = Table[i][j];
}

void codificare_tabla_A(char s[], char Table[11][11])
{
    int k = 0;
    for (int i = 0; i < 1000; i++)
        s[i] = '\0';

    for (int i = 8; i >= 1; i--)
        for (int j = 0; j <= 8; j++)
            s[k++] = Table[i][j];

    for (int j = 0; j <= 8; j++)
        s[k++] = Table[9][j];
}

int LiteraMare(char letter)
{
    if (letter >= 'A' && letter <= 'Z')
        return 1;
    return 0;
}

int LiteraMica(char letter)
{
    if (letter >= 'a' && letter <= 'z')
        return 1;
    return 0;
}

int cautare_jucator_1(int v[], int n)
{

    for (int i = 0; i <= n; i++)
        if (v[i] == 1)
            return i;

    return -1;
}

int cautare_jucator_2(int v[], int n)
{
    int k = 0;
    for (int i = 0; i <= n; i++)
    {
        if (k == 1 && v[i] == 1)
            return i;
        if (v[i] == 1)
            k++;
    }
    return -1;
}

//--------------------------------------------------------//

void Create_Table(char Table[11][11])
{
    for (int i = 1; i <= 8; i++)
        for (int j = 1; j <= 8; j++)
            Table[i][j] = '-';

    for (int i = 1; i <= 8; i++)
        Table[i][0] = '1' + i - 1; //bordare pe linie cu cifre

    for (int i = 1; i <= 8; i++)
        Table[9][i] = 'a' + i - 1; // bordare pe coloana cu litere

    //litere mari - alb
    //litere mici - negru

    //pioni
    for (int i = 1; i <= 8; i++)
        Table[2][i] = 'P';

    for (int i = 1; i <= 8; i++)
        Table[7][i] = 'p';

    //cai
    Table[1][2] = Table[1][7] = 'C';
    Table[8][2] = Table[8][7] = 'c';

    //ture
    Table[1][1] = Table[1][8] = 'T';
    Table[8][1] = Table[8][8] = 't';

    //nebuni
    Table[1][3] = Table[1][6] = 'N';
    Table[8][3] = Table[8][6] = 'n';

    //rege
    Table[1][5] = 'K';
    Table[8][5] = 'k';

    //regina
    Table[1][4] = 'Q';
    Table[8][4] = 'q';
}

int D(int IC, int IL, int FC, int FL, char Table[11][11]) //parcurgerea pe diagonale
{
    if (FC < IC && FL > IL) //parcurgere NE (dreapta-sus)
    {
        int i = IC - 1;
        int j = IL + 1;
        while (Table[i][j] == '-' && i > FC && j < FL)
        {
            i--;
            j++;
        }
        if (i == FC && j == FL)
            return 1;
        else
            return 0;
    }

    else if (FC < IC && FL < IL) //parcurgere NV (stanga-sus)
    {
        int i = IC - 1;
        int j = IL - 1;
        while (Table[i][j] == '-' && i > FC && j > FL)
        {
            i--;
            j--;
        }
        if (i == FC && j == FL)
            return 1;
        else
            return 0;
    }

    else if (FC > IC && FL > IL) //parcurgere SE (dreapta-jos)
    {
        int i = IC + 1;
        int j = IL + 1;
        while (Table[i][j] == '-' && i < FC && j < FL)
        {
            i++;
            j++;
        }
        if (i == FC && j == FL)
            return 1;
        else
            return 0;
    }

    else if (FC > IC && FL < IL) //parcurgere SV (stanga-jos)
    {
        int i = IC + 1;
        int j = IL - 1;
        while (Table[i][j] == '-' && i < FC && j > FL)
        {
            i++;
            j--;
        }
        if (i == FC && j == FL)
            return 1;
        else
            return 0;
    }

    else
        return 0;
}

int L_C(int IC, int IL, int FC, int FL, char Table[11][11]) //parcurgere pe linie si coloana
{

    if (FL > IL && FC == IC) //de la stanga la dreapta
    {
        int i = IC;
        int j = IL + 1;
        while (Table[i][j] == '-' && j < FL)
            j++;
        if (i == FC && j == FL)
            return 1;
        else
            return 0;
    }

    else if (FL < IL && FC == IC) //de la dreapta la stanga
    {
        int i = IC;
        int j = IL - 1;
        while (Table[i][j] == '-' && j > FL)
            j--;
        if (i == FC && j == FL)
            return 1;
        else
            return 0;
    }

    else if (FC > IC && FL == IL) // de sus in jos
    {
        int i = IC + 1;
        int j = IL;
        while (Table[i][j] == '-' && i < FC)
            i++;
        if (i == FC && j == FL)
            return 1;
        else
            return 0;
    }

    else if (FC < IC && FL == IL) // de jos in sus
    {
        int i = IC - 1;
        int j = IL;
        while (Table[i][j] == '-' && i > FC)
            i--;
        if (i == FC && j == FL)
            return 1;
        else
            return 0;
    }

    else
        return 0;
}

int Pawn(char Init, int IC, int IL, int FC, int FL, char Table[11][11])
{
    if (LiteraMare(Init))
    {
        if (LiteraMare(Table[FC][FL]))
            return 0; //se afla o piesa de a ta

        else if (FC > 8 || FC < 1 || FL > 8 || FL < 1)
            return 0; //in afara matricii

        else if (IC >= FC)
            return 0; // nu are voie sa fie dat inapoi

        else if (FC - IC == 2 && IC == 2 && FL == IL && Table[FC][FL] == '-')
            return 1; //muta 2 patratele la inceput

        else if (FC - IC == 1 && FL == IL && Table[FC][FL] == '-')
            return 1; //muta o patratica

        else if (FC - IC == 1 && abs(FL - IL) == 1 && LiteraMica(Table[FC][FL]))
            return 1; //mananca o piesa
        else
            return 0;
    }
    else if (LiteraMica(Init))
    {
        if (LiteraMica(Table[FC][FL]))
            return 0; //se afla o piesa de a ta

        else if (FC > 8 || FC < 1 || FL > 8 || FL < 1)
            return 0; //in afara matricii

        else if (FC >= IC)
            return 0; // nu are voie  sa fie dat inapoi

        else if (IC - FC == 2 && IC == 7 && IL == FL && Table[FC][FL] == '-')
            return 1; //muta 2 patratele la inceput

        else if (IC - FC == 1 && FL == IL && Table[FC][FL] == '-')
            return 1; //muta o patratica

        else if (IC - FC == 1 && abs(FL - IL) == 1 && LiteraMare(Table[FC][FL]))
            return 1; //mananca o piesa
        else
            return 0;
    }
}

int Knight(char Init, int IC, int IL, int FC, int FL, char Table[11][11])
{
    if (LiteraMare(Init))
    {
        if (LiteraMare(Table[FC][FL]))
            return 0; //se afla o piesa de a ta
    }
    else if (LiteraMica(Init))
    {
        if (LiteraMica(Table[FC][FL]))
            return 0;
    }

    if (FC > 8 || FC < 1 || FL > 8 || FL < 1)
        return 0; //in afara matricii

    else if ((abs(FC - IC) == 1 || abs(FC - IC) == 2) && (abs(FL - IL) == 1 || abs(FL - IL) == 2))
        return 1; //mutari posibile
    else
        return 0;
}

int Bishop(char Init, int IC, int IL, int FC, int FL, char Table[11][11])
{
    if (LiteraMare(Init))
    {
        if (LiteraMare(Table[FC][FL])) //se afla o piesa de ata
            return 0;
        else if (FC > 8 || FC < 1 || FL > 8 || FL < 1) //in afara matricii
            return 0;
        else if (D(IC, IL, FC, FL, Table)) //verificare pe diagonale
            return 1;
        else
            return 0;
    }
    else if (LiteraMica(Init))
    {
        if (LiteraMica(Table[FC][FL])) //se afla o piesa de a ta
            return 0;
        else if (FC > 8 || FC < 1 || FL > 8 || FL < 1) //in afara matricii
            return 0;
        else if (D(IC, IL, FC, FL, Table)) //verificare pe diagonale
            return 1;
        else
            return 0;
    }
}

int Rook(char Init, int IC, int IL, int FC, int FL, char Table[11][11])
{
    if (LiteraMare(Init))
    {
        if (LiteraMare(Table[FC][FL])) //se afla o piesa de ata
            return 0;
        else if (FC > 8 || FC < 1 || FL > 8 || FL < 1) //in afara matricii
            return 0;
        else if ((abs(FC - IC) == 1 || abs(FC - IC) == 0) && (abs(FL - IL) == 0 || abs(FL - IL) == 1)) //verificare mutari posibile
            return 1;
        else
            return 0;
    }
    else if (LiteraMica(Init))
    {
        if (LiteraMica(Table[FC][FL])) //se afla o piesa de a ta
            return 0;
        else if (FC > 8 || FC < 1 || FL > 8 || FL < 1) //in afara matricii
            return 0;
        else if ((abs(FC - IC) == 1 || abs(FC - IC) == 0) && (abs(FL - IL) == 0 || abs(FL - IL) == 1)) //verificare mutari posibile
            return 1;
        else
            return 0;
    }
}

int King(char Init, int IC, int IL, int FC, int FL, char Table[11][11])
{
    if (LiteraMare(Init))
    {
        if (LiteraMare(Table[FC][FL])) //se afla o piesa de a ta
            return 0;
        else if (FC > 8 || FC < 1 || FL > 8 || FL < 1) //in afara matricii
            return 0;
        else if ((abs(FC - IC) == 1 || abs(FC - IC) == 0) && (abs(FL - IL) == 1 || abs(FL - IL) == 0))
            return 1;
        else
            return 0;
    }
    else if (LiteraMica(Init))
    {
        if (LiteraMica(Table[FC][FL])) //se afla o piesa de a ta
            return 0;
        else if (FC > 8 || FC < 1 || FL > 8 || FL < 1) //in afara matricii
            return 0;
        else if ((abs(FC - IC) == 1 || abs(FC - IC) == 0) && (abs(FL - IL) == 1 || abs(FL - IL) == 0))
            return 1;
        else
            return 0;
    }
}

int Queen(char Init, int IC, int IL, int FC, int FL, char Table[11][11])
{
    if (LiteraMare(Init))
    {
        if (LiteraMare(Table[FC][FL])) //se afla o piesa de ata
            return 0;
        else if (FC > 8 || FC < 1 || FL > 8 || FL < 1) //in afara matricii
            return 0;
        else if (L_C(IC, IL, FC, FL, Table) || D(IC, IL, FC, FL, Table)) //verificare pe linii
            return 1;
        else
            return 0;
    }
    else if (LiteraMica(Init))
    {
        if (LiteraMica(Table[FC][FL])) //se afla o piesa de a ta
            return 0;
        else if (FC > 8 || FC < 1 || FL > 8 || FL < 1) //in afara matricii
            return 0;
        else if (L_C(IC, IL, FC, FL, Table) || D(IC, IL, FC, FL, Table)) //verificare pe linii
            return 1;
        else
            return 0;
    }
}

int Verifica_mutare(char Init, int IC, int IL, int FC, int FL, char Table[11][11]) // verificare daca o piesa poate fi mutata intr-o anumita pozitie
{
    if (Init == 'P' || Init == 'p')
    {
        if (Pawn(Init, IC, IL, FC, FL, Table))
            return 1;
        return 0;
    }
    else if (Init == 'T' || Init == 't')
    {
        if (Rook(Init, IC, IL, FC, FL, Table))
            return 1;
        return 0;
    }
    else if (Init == 'C' || Init == 'c')
    {
        if (Knight(Init, IC, IL, FC, FL, Table))
            return 1;
        return 0;
    }
    else if (Init == 'N' || Init == 'n')
    {
        if (Bishop(Init, IC, IL, FC, FL, Table))
            return 1;
        return 0;
    }
    else if (Init == 'Q' || Init == 'q')
    {
        if (Queen(Init, IC, IL, FC, FL, Table))
            return 1;
        return 0;
    }
    else if (Init == 'K' || Init == 'k')
    {
        if (King(Init, IC, IL, FC, FL, Table))
            return 1;
        return 0;
    }
}

int find_king_FC(char Init, char Table[11][11]) //cautarea regelui (returneaza linia pe care se afla)
{

    for (int i = 1; i <= 8; i++)
        for (int j = 1; j <= 8; j++)
            if (Table[i][j] == Init)
                return i;
}

int find_king_FL(char Init, char Table[11][11]) //cautarea regelui (returneaza coloana pe care se afla)
{

    for (int i = 1; i <= 8; i++)
        for (int j = 1; j <= 8; j++)
            if (Table[i][j] == Init)
                return j;
}

int Attacked(char Init, int FC, int FL, char Table[11][11]) //verificare daca o piesa este atacata (pentru rege)
{

    if (FC < 1 || FC > 8 || FL < 1 || FL > 8)
        return 0;

    else if (LiteraMare(Init))
    {
        for (int i = 1; i <= 8; i++)
            for (int j = 1; j <= 8; j++)
                if (LiteraMica(Table[i][j]))
                    if (Verifica_mutare(Table[i][j], i, j, FC, FL, Table))
                        return 1;

        return 0;
    }

    else if (LiteraMica(Init))
    {
        for (int i = 1; i <= 8; i++)
            for (int j = 1; j <= 8; j++)
                if (LiteraMare(Table[i][j]))
                    if (Verifica_mutare(Table[i][j], i, j, FC, FL, Table))
                        return 1;

        return 0;
    }
}

int Sah(char Init, char Table[11][11]) // daca regele se afla sau nu in sah
{
    int FC = 0;
    int FL = 0;
    FC = find_king_FC(Init, Table);
    FL = find_king_FL(Init, Table);
    if (Attacked(Init, FC, FL, Table))
        return 1;
    return 0;
}

int Sah_Mat(char Init, char Table[11][11]) // daca regele se afla sau nu in sahmat
{

    if (Sah(Init, Table))
    {
        char A[11][11];
        char B[11][11];
        int FC = find_king_FC(Init, Table);
        int FL = find_king_FL(Init, Table);

        copiere_tabla(A, Table);
        copiere_tabla(B, Table);

        //o sa adaugam in jurul regelui k-uri cu semnificatia ca acolo se poate deplasa pentru a scapa de sah
        if (LiteraMare(Init))
        {
            if (B[FC - 1][FL] == '-')
                Table[FC - 1][FL] = 'K';
            if (B[FC + 1][FL] == '-')
                Table[FC + 1][FL] = 'K';
            if (B[FC + 1][FL - 1] == '-')
                Table[FC + 1][FL - 1] = 'K';
            if (B[FC][FL - 1] == '-')
                Table[FC][FL - 1] = 'K';
            if (B[FC - 1][FL - 1] == '-')
                Table[FC - 1][FL - 1] = 'K';
            if (B[FC - 1][FL + 1] == '-')
                Table[FC - 1][FL + 1] = 'K';
            if (B[FC][FL + 1] == '-')
                Table[FC][FL + 1] = 'K';
            if (B[FC + 1][FL + 1] == '-')
                Table[FC + 1][FL + 1] = 'K';
        }
        else if (LiteraMica(Init))
        {
            if (B[FC - 1][FL] == '-')
                Table[FC - 1][FL] = 'k';
            if (B[FC + 1][FL] == '-')
                Table[FC + 1][FL] = 'k';
            if (B[FC - 1][FL - 1] == '-')
                Table[FC - 1][FL - 1] = 'k';
            if (B[FC - 1][FL + 1] == '-')
                Table[FC - 1][FL + 1] = 'k';
            if (B[FC + 1][FL + 1] == '-')
                Table[FC + 1][FL + 1] = 'k';
            if (B[FC + 1][FL - 1] == '-')
                Table[FC + 1][FL - 1] = 'k';
            if (B[FC][FL - 1] == '-')
                Table[FC][FL - 1] = 'k';
            if (B[FC][FL + 1] == '-')
                Table[FC][FL + 1] = 'k';
        }

        //verificam daca regele poate scapa de sah mat
        if (Attacked(Init, FC - 1, FL, Table) && A[FC - 1][FL] == '-')
            A[FC - 1][FL] = 'x';
        if (Attacked(Init, FC + 1, FL, Table) && A[FC + 1][FL] == '-')
            A[FC + 1][FL] = 'x';
        if (Attacked(Init, FC + 1, FL + 1, Table) && A[FC + 1][FL + 1] == '-')
            A[FC + 1][FL + 1] = 'x';
        if (Attacked(Init, FC + 1, FL - 1, Table) && A[FC + 1][FL - 1] == '-')
            A[FC + 1][FL - 1] = 'x';
        if (Attacked(Init, FC - 1, FL - 1, Table) && A[FC - 1][FL - 1] == '-')
            A[FC - 1][FL - 1] = 'x';
        if (Attacked(Init, FC, FL + 1, Table) && A[FC][FL + 1] == '-')
            A[FC][FL + 1] = 'x';
        if (Attacked(Init, FC - 1, FL + 1, Table) && A[FC - 1][FL + 1] == '-')
            A[FC - 1][FL + 1] = 'x';
        if (Attacked(Init, FC, FL - 1, Table) && A[FC][FL - 1] == '-')
            A[FC][FL - 1] = 'x';

        copiere_tabla(Table, B);

        //da, are cale de scapare
        if (A[FC][FL + 1] == '-' || A[FC][FL - 1] == '-' || A[FC - 1][FL + 1] == '-' || A[FC + 1][FL - 1] == '-' || A[FC + 1][FL + 1] == '-' || A[FC - 1][FL - 1] == '-' || A[FC + 1][FL] == '-' || A[FC - 1][FL] == '-')
            return 0;
        //nu are
        return 1;
    }
    return 0;
}

int Mutarea_facuta(int fd, int fd_opposed, char move, char s[], char Table[11][11])
{

    
    char msg[1000];
    for (int i = 0; i < 1000; i++)
        msg[i] = '\0';

    int bytes;

    if ((bytes = read(fd, msg, sizeof(msg))) < 0)
    {
        perror("Eroare la read() de la client.\n");
        return 0;
    }

   

    int IC, IL, FC, FL;
    char Init, Final, transform;
    int nr = 0;
    // Preluare coordonate.
    for (int i = 0; msg[i]; i++)
    {
        if (msg[i] != ' ')
        {
            if (nr == 0)
                IL = (int)msg[i] - 96;

            if (nr == 1)
                IC = (int)msg[i] - 48;

            if (nr == 2)

                FL = (int)msg[i] - 96;

            if (nr == 3)
                FC = (int)msg[i] - 48;

            if (nr == 4)
                transform = msg[i];

            nr++;
        }
    }

    Init = Table[IC][IL];
    Final = Table[FC][FL];

    if (strstr(msg, "surrender"))
    {

        write(fd_opposed, "Ai castigat", 1000);

        if (move == 'a')
        {
            codificare_tabla_B(s, Table);
            write(fd_opposed, s, 1000);
        }
        else
        {
            codificare_tabla_A(s, Table);
            write(fd_opposed, s, 1000);
        }

        return -1;
    }

    else if (move == 'a' && LiteraMica(Init))
    {
        strcpy(msg, "Mutare invalida");

        write(fd, msg, 1000);

        return -2;
    }
    else if (move == 'b' && LiteraMare(Init))
    {
        strcpy(msg, "Mutare invalida");

        write(fd, msg, 1000);

        return -2;
    }
    else if (Table[IC][IL] == '-')
    {
        strcpy(msg, "Mutare invalida");

        write(fd, msg, 1000);

        return -2;
    }
    else if (!Verifica_mutare(Init, IC, IL, FC, FL, Table))
    {
        strcpy(msg, "Mutare invalida");

        write(fd, msg, 1000);

        return -2;
    }
    else if (Verifica_mutare(Init, IC, IL, FC, FL, Table))
    {
        Table[IC][IL] = '-';
        Table[FC][FL] = Init;

        if (move == 'a' && Sah('K', Table))
        {
            strcpy(msg, "Mutare invalida! Sah!");

            write(fd, msg, 1000);

            Table[IC][IL] = Init;
            Table[FC][FL] = Final;
            return -2;
        }
        else if (move == 'b' && Sah('k', Table))
        {
            strcpy(msg, "Mutare invalida! Sah!");

            write(fd, msg, 1000);

            Table[IC][IL] = Init;
            Table[FC][FL] = Final;
            return -2;
        }

        //transformare pion
        if (Init == 'p' && FC == 1 && LiteraMica(transform) && (transform == 'n' || transform == 'c' || transform == 'q' || transform == 't'))
        {
            Table[IC][IL] = '-';
            Table[FC][FL] = transform;
        }
        else if (Init == 'P' && FC == 8 && LiteraMare(transform) && (transform == 'N' || transform == 'C' || transform == 'Q' || transform == 'T'))
        {
            Table[IC][IL] = '-';
            Table[FC][FL] = transform;
        }
        else if ((transform != 'n' || transform != 'c' || transform != 'q' || transform != 't' || transform != 'N' || transform != 'C' || transform != 'Q' || transform != 'T') && (Init == 'p' || Init == 'P'))
        {
            if (FC == 1 || FC == 8)
            {
                strcpy(msg, "Transformare invalida!");
                write(fd_opposed, msg, 1000);
                return -2;
            }
        }


          if (move == 'a' && Sah_Mat('k', Table))
    {

        write(fd, "Ai castigat", 1000);
        write(fd_opposed, "Ai pierdut", 1000);

        if (move == 'a')
        {
            codificare_tabla_A(s, Table);
            write(fd, s, 1000);
            codificare_tabla_B(s, Table);
            write(fd_opposed, s, 1000);
        }
        else
        {
            codificare_tabla_B(s, Table);
            write(fd, s, 1000);
            codificare_tabla_A(s, Table);
            write(fd_opposed, s, 1000);
        }

        return -1;
    }
    else if (move == 'b' && Sah_Mat('K', Table))
    {

        write(fd, "Ai castigat", 1000);
        write(fd_opposed, "Ai pierdut", 1000);

        if (move == 'a')
        {
            codificare_tabla_A(s, Table);
            write(fd, s, 1000);

            codificare_tabla_B(s, Table);
            write(fd_opposed, s, 1000);
        }
        else
        {
            codificare_tabla_B(s, Table);
            write(fd, s, 1000);

            codificare_tabla_A(s, Table);
            write(fd_opposed, s, 1000);
        }

        return -1;
    }


        else if (move == 'a' && Sah('k', Table))
        {
            strcpy(msg, "Mutare efecuata! Regele inamicului este in sah!");
            codificare_tabla_B(s, Table);
            bytes = strlen(s) + 1;
            if (bytes && write(fd_opposed, s, 1000) < 0)
            {
                perror("[server] Eroare la write() catre client.\n");
                return 0;
            }

            if (strlen(msg) && write(fd, msg, 1000) < 0)
            {
                perror("[server] Eroare la write() catre client.\n");
                return 0;
            }

            verificare = 1;
            return bytes;
        }
        else if (move == 'b' && Sah('K', Table))
        {
            strcpy(msg, "Mutare efecuata! Regele inamicului este in sah!");
            codificare_tabla_A(s, Table);
            bytes = strlen(s) + 1;
            if (bytes && write(fd_opposed, s, 1000) < 0)
            {
                perror("[server] Eroare la write() catre client.\n");
                return 0;
            }

            if (strlen(msg) && write(fd, msg, 1000) < 0)
            {
                perror("[server] Eroare la write() catre client.\n");
                return 0;
            }

            verificare = 1;
            return bytes;
        }
        else if (move == 'a')
        {
            strcpy(msg, "Mutare efecuata!");
            codificare_tabla_B(s, Table);
            bytes = strlen(s) + 1;
            if (bytes && write(fd_opposed, s, 1000) < 0)
            {
                perror("[server] Eroare la write() catre client.\n");
                return 0;
            }

            if (strlen(msg) && write(fd, msg, 1000) < 0)
            {
                perror("[server] Eroare la write() catre client.\n");
                return 0;
            }

            verificare = 1;
            return bytes;
        }
        else if (move == 'b')
        {
            strcpy(msg, "Mutare efecuata!");
            codificare_tabla_A(s, Table);
            bytes = strlen(s) + 1;
            if (bytes && write(fd_opposed, s, 1000) < 0)
            {
                perror("[server] Eroare la write() catre client.\n");
                return 0;
            }

            if (strlen(msg) && write(fd, msg, 1000) < 0)
            {
                perror("[server] Eroare la write() catre client.\n");
                return 0;
            }

            verificare = 1;
            return bytes;
        }
    }
}

void *Game(void *vargp)
{
    char NewTable[11][11];
    char s[1000];
    bzero(s, 1000);
    Create_Table(NewTable);

    int a_round = 1;
    int b_round = 0;
    int a_fd;
    int b_fd;

    if ((rand() % 2) == 1)
    {
        a_fd = jucatorul_unu;
        b_fd = jucatorul_doi;
    }
    else
    {
        a_fd = jucatorul_doi;
        b_fd = jucatorul_unu;
    }

    if (write(a_fd, "[client]Meciul a inceput!", 1000) < 0)
    {
        perror("[server] Eroare la write() catre client.\n");
        return 0;
    }

    if (write(b_fd, "[client]Meciul a inceput!", 1000) < 0)
    {
        perror("[server] Eroare la write() catre client.\n");
        return 0;
    }

    codificare_tabla_A(s, NewTable);

    if (write(a_fd, s, 1000) < 0)
    {
        perror("[server] Eroare la write() catre client.\n");
        return 0;
    }

    while (1)
    {
        if (a_round == 1)
        {

            verificare = 0;
            if (a_fd != sd)
            {
                if (Mutarea_facuta(a_fd, b_fd, 'a', s, NewTable) == -1) //Jocul a luat sfarsit! Jucatorul B a castigat!
                {

                    v[jucatorul_unu] = 0;
                    v[jucatorul_doi] = 0;
                    return 0;
                }
                else if (verificare == 1) //mutarea este valida
                {
                    codificare_tabla_A(s, NewTable);

                    if (write(a_fd, s, 1000) < 0)
                    {
                        perror("[server] Eroare la write() catre client.\n");
                        return 0;
                    }

                    a_round = 0;
                    b_round = 1;
                    verificare = 0;
                }
                else //mutarea nu este valida si jucatorul trebuie sa introduca una noua
                {
                    codificare_tabla_A(s, NewTable); //informatiile despre tabla vor fi trimise catre client printr-un vector char

                    if (write(a_fd, s, 1000) < 0)
                    {
                        perror("[server] Eroare la write() catre client.\n");
                        return 0;
                    }
                }
            }
        }
        if (b_round == 1)
        {
            verificare = 0;
            if (b_fd != sd)
            {
                if (Mutarea_facuta(b_fd, a_fd, 'b', s, NewTable) == -1) //jocul sa terminat! Jucatorul A a castigat!
                {

                    v[jucatorul_unu] = 0;
                    v[jucatorul_doi] = 0;
                    return 0;
                }
                else if (verificare == 1) //mutarea este valida
                {
                    codificare_tabla_B(s, NewTable);

                    if (write(b_fd, s, 1000) < 0)
                    {
                        perror("[server] Eroare la write() catre client.\n");
                        return 0;
                    }
                    b_round = 0;
                    a_round = 1;
                    verificare = 0;
                }
                else //mutarea nu este valida si jucatorul trebuie sa introduca una noua
                {
                    codificare_tabla_B(s, NewTable); //informatiile despre tabla vor fi trimise catre client printr-un vector char

                    if (write(b_fd, s, 1000) < 0)
                    {
                        perror("[server] Eroare la write() catre client.\n");
                        return 0;
                    }
                }
            }
        }
    }
}

//-------------------------------------------------------------------------------------------//

//functie de convertire a adresei IP a clientului in sir de caractere
char *conv_addr(struct sockaddr_in address)
{
    static char str[25];
    char port[7];

    /* adresa IP a clientului */
    strcpy(str, inet_ntoa(address.sin_addr));
    /* portul utilizat de client */
    bzero(port, 7);
    sprintf(port, ":%d", ntohs(address.sin_port));
    strcat(str, port);
    return (str);
}

int main()
{
    srand(time(NULL));
    struct sockaddr_in server; /* structurile pentru server si clienti */
    struct sockaddr_in from;
    fd_set readfds; /* multimea descriptorilor de citire */

    struct timeval tv; /* structura de timp pentru select() */
    int sd, client;    /* descriptori de socket */
    int optval = 1;    /* optiune folosita pentru setsockopt()*/
    int fd;            /* descriptor folosit pentru 
				   parcurgerea listelor de descriptori */
    int nfds;          /* numarul maxim de descriptori */
    int len;           /* lungimea structurii sockaddr_in */

    /* creare socket */
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("[server] Eroare la socket().\n");
        return errno;
    }

    /*setam pentru socket optiunea SO_REUSEADDR */
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    /* pregatim structurile de date */
    bzero(&server, sizeof(server));

    /* umplem structura folosita de server */
    /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;
    /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    /* utilizam un port utilizator */
    server.sin_port = htons(PORT);

    /* atasam socketul */
    if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("[server] Eroare la bind().\n");
        return errno;
    }

    /* punem serverul sa asculte daca vin clienti sa se conecteze */
    if (listen(sd, 5) == -1)
    {
        perror("[server] Eroare la listen().\n");
        return errno;
    }

    /* completam multimea de descriptori de citire */
    FD_ZERO(&actfds);    /* initial, multimea este vida */
    FD_SET(sd, &actfds); /* includem in multime socketul creat */

    tv.tv_sec = 1; /* se va astepta un timp de 1 sec. */
    tv.tv_usec = 0;

    /* valoarea maxima a descriptorilor folositi */
    nfds = sd;

    printf("[server] Asteptam la portul %d...\n", PORT);
    fflush(stdout);

    /* servim in mod concurent clientii... */
    while (1)
    {
        /* ajustam multimea descriptorilor activi (efectiv utilizati) */
        bcopy((char *)&actfds, (char *)&readfds, sizeof(readfds));

        /* apelul select() */
        if (select(nfds + 1, &readfds, NULL, NULL, &tv) < 0)
        {
            perror("[server] Eroare la select().\n");
            return errno;
        }
        /* vedem daca e pregatit socketul pentru a-i accepta pe clienti */
        if (FD_ISSET(sd, &readfds))
        {
            /* pregatirea structurii client */
            len = sizeof(from);
            bzero(&from, sizeof(from));

            /* a venit un client, acceptam conexiunea */
            client = accept(sd, (struct sockaddr *)&from, &len);

            /* eroare la acceptarea conexiunii de la un client */
            if (client < 0)
            {
                perror("[server] Eroare la accept().\n");
                continue;
            }

            if (nfds < client) /* ajusteaza valoarea maximului */
                nfds = client;

            /* includem in lista de descriptori activi si acest socket */
            FD_SET(client, &actfds);
            printf("[server] S-a conectat clientul cu descriptorul %d, de la adresa %s.\n", client, conv_addr(from));
            fflush(stdout);
        }
        for (fd = 0; fd <= nfds; fd++) /* parcurgem multimea de descriptori */
        {

            if (fd != sd && FD_ISSET(fd, &readfds) && v[fd] == 0) //vedem daca un socket de citire este pregatit
            {
                char mesaj[1000];
                bzero(mesaj, 1000);
                if (read(fd, mesaj, 1000) < 0)
                {
                    perror("[server] Eroare la read().\n");
                    return errno;
                }

                if (strstr(mesaj, "quit"))
                {
                    printf("[server] S-a deconectat clientul cu descriptorul %d.\n", fd);
                    fflush(stdout);
                    close(fd);           /* inchidem conexiunea cu clientul */
                    FD_CLR(fd, &actfds); /* scoatem si din multime */
                }

                if (strstr(mesaj, "play"))
                {
                    v[fd] = 1; //jucatorul asteapta sa intre in meci
                    write(fd, "Cautare jucator...", 1000);
                }
            }
        }

        if (cautare_jucator_1(v, nfds) != -1 && cautare_jucator_2(v, nfds) != -1) //daca doi jucatori asteapta sa intre intr-un meci, atunci acesta va incepe
        {

            jucatorul_unu = cautare_jucator_1(v, nfds);
            jucatorul_doi = cautare_jucator_2(v, nfds);

            v[jucatorul_unu] = 2; //jucatorii se aflam in meci
            v[jucatorul_doi] = 2;

            pthread_t tid;

            pthread_create(&tid, NULL, Game, (void *)&tid); //logica jocului
        }
    }
}
