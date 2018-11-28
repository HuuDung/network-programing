#include <stdio.h> /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define BACKLOG 2 /* Number of allowed connections */
#define BUFF_SIZE 1024
#define ERROR_FILE_IS_EXISTENT "Error: File is existent on server"
#define SUCCESS "success"
#define STORAGE "./data"
//Kiểm tra số hiệu cổng
int checkPort(char *port)
{
    int i, checkPort = 1;
    for (i = 0; i < strlen(port) - 1; i++)
    {
        if (port[i] < '0' || port[i] > '9')
            checkPort = 0;
    }
    if (checkPort == 0)
    {
        printf("\nPort invalid\n");
        return 0;
    }
    return 1;
}
//split string to file name
void getNameFile(char *input, char *output)
{
    int i, length = 0, checkSlap = 0, locate;
    for (i = strlen(input) - 1; i >= 0; i--)
    {
        if (input[i] == '/')
        {
            checkSlap = 1;
            locate = i + 1;
            break;
        }
    }
    if (checkSlap == 1)
    {
        for (i = locate; i < strlen(input); i++)
        {
            output[length++] = input[i];
        }
        output[length] = '\0';
    }
    else
        strcpy(output, input);
}
int main(int argc, char const *argv[])
{

    int listen_sock, conn_sock, servPort; /* file descriptors */
    char recv_data[BUFF_SIZE], name[BUFF_SIZE], status[BUFF_SIZE], locate[BUFF_SIZE], data[BUFF_SIZE];
    int bytes_sent, bytes_received;
    struct sockaddr_in server; /* server's address information */
    struct sockaddr_in client; /* client's address information */
    int sin_size;
    int i = 0;
    long fileSize = 0, currentSize;
    //Step 1: Construct a TCP socket to listen connection request
    if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    { /* calls socket() */
        perror("\nError: ");
        return 0;
    }

    //Step 2: Bind address to socket
    servPort = atoi(argv[1]);

    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(servPort);          /* Remember htons() from "Conversions" section? =) */
    server.sin_addr.s_addr = htonl(INADDR_ANY); /* INADDR_ANY puts your IP address automatically */
    if (bind(listen_sock, (struct sockaddr *)&server, sizeof(server)) == -1)
    { /* calls bind() */
        perror("\nError: ");
        return 0;
    }

    //Step 3: Listen request from client
    if (listen(listen_sock, BACKLOG) == -1)
    { /* calls listen() */
        perror("\nError: ");
        return 0;
    }
    struct stat st = {0};

    if (stat(STORAGE, &st) == -1)
    { //create storage if it not exist

        mkdir(STORAGE, 0755);
    }
    //Step 4: Communicate with client
    while (1)
    {
        //accept request
        sin_size = sizeof(struct sockaddr_in);
        if ((conn_sock = accept(listen_sock, (struct sockaddr *)&client, &sin_size)) == -1)
            perror("\nError: ");

        printf("You got a connection from %s\n", inet_ntoa(client.sin_addr)); /* prints client's IP */
        while (1)
        {
            bytes_received = recv(conn_sock, data, BUFF_SIZE - 1, 0); // file name
            if (bytes_received <= 0)
            {
                printf("\nConnection closed\n");
                break;
            }
            else
            {
                data[bytes_received] = '\0';
                getNameFile(data, name);
                strcpy(locate, "data/");
                strcat(locate, name);
            }

            FILE *fout = fopen(locate, "rb");
            if (fout) //exits file
            {
                strcpy(status, ERROR_FILE_IS_EXISTENT);
                bytes_sent = send(conn_sock, status, strlen(status), 0);
                fclose(fout);
            }
            else
            {
                fout = fopen(locate, "wb"); //create file
                strcpy(status, SUCCESS);
                bytes_sent = send(conn_sock, status, strlen(status), 0); //send status

                bytes_received = recv(conn_sock, &fileSize, sizeof(fileSize), 0); // received size of file
                printf("\n%s\t%ld\n", locate, fileSize);
                currentSize = 0;
                while (currentSize < fileSize)
                {
                    bytes_received = recv(conn_sock, recv_data, 1, 0);
                    currentSize += bytes_received;
                    fwrite(recv_data, sizeof(char), 1, fout);
                }
                fclose(fout);
            }
        }
        // close socket
        close(conn_sock);
    }
    close(listen_sock);
    return 0;
}
