#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT 3492       // Port du serveur
#define MAXDATASIZE 100 // Taille du tampon de réception

int main(int argc, char *argv[])
{
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct hostent *he;
    struct sockaddr_in server_addr;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <hostname>\n", argv[0]);
        exit(1);
    }

    if ((he = gethostbyname(argv[1])) == NULL)
    {
        herror("gethostbyname");
        exit(1);
    }
    printf("Etape 4 :Création du socket client .\n");
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    // Configuration de l'adresse du client
    printf("Etape 5:Configuration de l'adresse du client  .\n");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(server_addr.sin_zero), 0, 8);

    // Connexion au serveur
    printf("Etape 6: Demande de connexion au serveur .\n");
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("connect");
        close(sockfd);
        exit(1);
    }

    // Réception des données
    printf("Etape 8 :Connecté avec succès au serveur.\n");
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1)
    {
        perror("recv");
        close(sockfd);
        exit(1);
    }

    buf[numbytes] = '\0';
    printf("Reçu: %s\n", buf);

    close(sockfd);
    printf("Etape 9: Connexion fermée.\n");
    return 0;
}
