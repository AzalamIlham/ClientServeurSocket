#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT 3492 // Port d'écoute
#define BACKLOG 10 // Nombre maximum de connexions en file

// Nettoyage des processus fils
void sigchld_handler(int s)
{
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
}

int main()
{
    int sockfd, new_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t sin_size;
    struct sigaction sa;

    // Création du socket
    printf("Etape 1 :Création du socket serveur...\n");
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    printf("Etape 2 :Configuration de l'adresse du serveur.\n");
    // Configuration de l'adresse du serveur
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(server_addr.sin_zero), 0, 8);

    // Liaison du socket

    printf("Etape 3: Identification de socket et Liaison du socket au port %d...\n", PORT);
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("bind");
        close(sockfd);
        exit(1);
    }

    // Écoute des connexions entrantes
    printf("Etape 4 :Serveur en écoute sur le port %d...\n", PORT);
    if (listen(sockfd, BACKLOG) == -1)
    {
        perror("listen");
        close(sockfd);
        exit(1);
    }

    // Gestion des processus fils
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        close(sockfd);
        exit(1);
    }

    while (1)
    {
        sin_size = sizeof(struct sockaddr_in);
        new_fd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size);
        if (new_fd == -1)
        {
            perror("accept");
            continue;
        }

        printf("Etape7:Nouvelle connexion de %s\n", inet_ntoa(client_addr.sin_addr));

        if (!fork())
        {
            close(sockfd);
            printf("Etape 10 :Connexion avec le client terminée.\n");
            const char *message = "Hello, World!\n";
            if (send(new_fd, message, strlen(message), 0) == -1)
            {
                perror("send");
            }
            close(new_fd);
            exit(0);
        }

        close(new_fd);
    }

    close(sockfd);
    return 0;
}
