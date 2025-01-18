#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT 3490
#define MAX_FILENAME_LEN 256

int main(int argc, char *argv[])
{
    int sockfd;
    char buffer[1024];
    char filename[MAX_FILENAME_LEN];
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

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(server_addr.sin_zero), 0, 8);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("connect");
        close(sockfd);
        exit(1);
    }

    printf("Connecté au serveur. Entrez le nom du fichier à recevoir : ");
    if (fgets(filename, sizeof(filename), stdin) != NULL)
    {
        filename[strcspn(filename, "\n")] = 0;
    }

    // Envoi du nom du fichier au serveur
    if (send(sockfd, filename, strlen(filename), 0) == -1)
    {
        perror("send");
        close(sockfd);
        exit(1);
    }

    printf("Envoi du nom du fichier '%s' au serveur...\n", filename);

    // Réception du fichier
    FILE *file = fopen(filename, "wb");
    if (!file)
    {
        perror("fopen");
        close(sockfd);
        exit(1);
    }

    int bytes_received;
    while ((bytes_received = recv(sockfd, buffer, sizeof(buffer), 0)) > 0)
    {
        fwrite(buffer, 1, bytes_received, file);
    }

    if (bytes_received < 0)
    {
        perror("recv");
    }

    printf("Fichier '%s' reçu et sauvegardé.\n", filename);

    fclose(file);
    close(sockfd);
    return 0;
}
