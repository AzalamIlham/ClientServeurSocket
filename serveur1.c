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

#define PORT 3490            // Port d'écoute
#define BACKLOG 10           // Nombre maximum de connexions en file
#define MAX_FILENAME_LEN 256 // Taille maximale du nom du fichier

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
    char filename[MAX_FILENAME_LEN];

    // Création du socket
    printf("Étape 1 : Création du socket serveur...\n");
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    printf("Étape 2 : Configuration de l'adresse du serveur.\n");
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(server_addr.sin_zero), 0, 8);

    printf("Étape 3 : Liaison du socket au port %d...\n", PORT);
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("bind");
        close(sockfd);
        exit(1);
    }

    printf("Étape 4 : Serveur en écoute sur le port %d...\n", PORT);
    if (listen(sockfd, BACKLOG) == -1)
    {
        perror("listen");
        close(sockfd);
        exit(1);
    }

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

        printf("Nouvelle connexion de %s\n", inet_ntoa(client_addr.sin_addr));

        // Recevoir le nom du fichier du client
        int bytes_received = recv(new_fd, filename, sizeof(filename) - 1, 0);
        if (bytes_received <= 0)
        {
            perror("recv");
            close(new_fd);
            continue;
        }
        filename[bytes_received] = '\0';

        printf("Demande du fichier : %s\n", filename);

        // Ouvrir le fichier demandé
        FILE *file = fopen(filename, "rb");
        if (!file)
        {
            perror("fopen");
            const char *error_msg = "Fichier non trouvé\n";
            send(new_fd, error_msg, strlen(error_msg), 0);
            close(new_fd);
            continue;
        }

        // Envoi du fichier au client
        printf("Envoi du fichier '%s' au client...\n", filename);
        char buffer[1024];
        int bytes_read;
        while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0)
        {
            if (send(new_fd, buffer, bytes_read, 0) == -1)
            {
                perror("send");
                break;
            }
        }

        fclose(file);
        printf("Fichier envoyé avec succès.\n");
        close(new_fd);
    }

    close(sockfd);
    return 0;
}
