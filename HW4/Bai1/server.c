#include <stdio.h> /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define BACKLOG 2 /* Number of allowed connections */
#define BUFF_SIZE 1024
//Kiểm tra kí tự có phải kí tự đặc biệt hay không
//Input: một mảng
//Output: tồn tại kí tự đặc biệt => 0
//          không tồn tại kí tự đặc biệt = > 1
int checkCharacter(char *input)
{
    int i, checkCharacter = 0;
    for (i = 0; i < strlen(input); i++)
    {
        if ((input[i] >= '0' && input[i] <= '9') || (input[i] >= 'a' && input[i] <= 'z') || (input[i] >= 'A' && input[i] <= 'Z'))
            checkCharacter = 0;
        else
        {
            checkCharacter = 1;
            break;
        }
    }
    if (checkCharacter == 1)
        return 0;
    else
        return 1;
}
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
/*
Input: Nhận vào địa chỉ mảng input, numberString, characterString
Process: Mảng input không chứa kí tự đặc biệt thì tách thành hai mảng: numberString- chứa số & characterString -chứa kí tự
Output: Trả lại trạng thaí khi kiểm tra có kí tự đặc biệt không:
    0 - chứa kí tự đặc biêt
    1 - không chứa kí tự đặc biệt
*/
int convertString(char *input, char *numberString, char *characterString)
{
    int i, lengthNumber = 0, lengthChar = 0;
    if (checkCharacter(input) == 1)
    {
        for (i = 0; i < strlen(input); i++)
        {

            if (input[i] >= '0' && input[i] <= '9')
            {
                numberString[lengthNumber++] = input[i];
            }
            else
            {
                characterString[lengthChar++] = input[i];
            }
        }

        numberString[lengthNumber] = '\0';
        characterString[lengthChar] = '\0';
        return 1;
    }
    else
        return 0;
}

int main(int argc, char const *argv[])
{

    int listen_sock, conn_sock, servPort; /* file descriptors */
    char recv_data[BUFF_SIZE], number[BUFF_SIZE], charater[BUFF_SIZE], result[BUFF_SIZE];
    int bytes_sent, bytes_received;
    struct sockaddr_in server; /* server's address information */
    struct sockaddr_in client; /* client's address information */
    int sin_size, idx, ret, nLeft;

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

    //Step 4: Communicate with client
    while (1)
    {
        //accept request
        sin_size = sizeof(struct sockaddr_in);
        if ((conn_sock = accept(listen_sock, (struct sockaddr *)&client, &sin_size)) == -1)
            perror("\nError: ");

        printf("You got a connection from %s\n", inet_ntoa(client.sin_addr)); /* prints client's IP */

        //start conversation
        while (1)
        {
            //receives message from client
            bytes_received = recv(conn_sock, recv_data, BUFF_SIZE - 1, 0); //blocking
            if (bytes_received <= 0)
            {
                printf("\nConnection closed\n");
                break;
            }
            else
            {
                recv_data[bytes_received] = '\0';
                printf("\nReceive: %s \n", recv_data);
            }

            //echo to client
            if (convertString(recv_data, number, charater) == 1)
            {
                strcpy(result, "Result: \n");
                strcat(result, number);
                strcat(result, "\n");
                strcat(result, charater);
            }
            else
            {
                strcpy(result, "Error");
            }

            nLeft = strlen(result);
            idx = 0;
            while (nLeft > 0)
            {
                // Assume s is a valid, connected stream socket
                ret = send(conn_sock, &result[idx], nLeft, 0);
                if (ret == -1)
                {
                    // Error handler
                }
                nLeft -= ret;
                idx += ret;
            }
        } //end conversation
        close(conn_sock);
    }
    close(listen_sock);
    return 0;
}