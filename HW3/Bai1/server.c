/*UDP Echo Server*/
#include <stdio.h> /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define BUFF_SIZE 1024
//Kiểm tra kí tự có phải kí tự đặc biệt hay không
//Input: một mảng
//Output: tồn tại kí tự đặc biệt => 0
//          không tồn tại kí tự đặc biệt = > 1
int checkCharacter(char *input)
{
    int i, checkCharacter = 0;
    for (i = 0; i < strlen(input) - 1; i++)
    {
        if ((input[i] >= '0' && input[i] <= '9') || (input[i] >= 'a' && input[i] <= 'z') || (input[i] >= 'A' && input[i] <= 'Z'))
            checkCharacter = 0;
        else
            checkCharacter = 1;
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
        for (i = 0; i < strlen(input) - 1; i++)
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
main(int argc, char const *argv[])
{

    int server_sock, servPort; /* file descriptors */
    char buff[BUFF_SIZE], number[BUFF_SIZE], charater[BUFF_SIZE];
    char result[BUFF_SIZE];
    int bytes_sent, bytes_received;
    int bytes_number_sent, bytes_character_sent;
    struct sockaddr_in server; /* server's address information */
    struct sockaddr_in client; /* client's address information */
    int sin_size;
    struct sockaddr_in cliaddr;

    if (argc != 2)
    {
        printf("Nhập thiếu tham số\n");
    }
    else
    {
        if (checkPort(argv[1]) != 0)
        {
            servPort = atoi(argv[1]);
            //Construct a UDP socket
            if ((server_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
            { /* calls socket() */
                perror("\nError: ");
                exit(0);
            }

            //Bind address to socket
            server.sin_family = AF_INET;
            server.sin_port = htons(servPort);   /* Remember htons() from "Conversions" section? =) */
            server.sin_addr.s_addr = INADDR_ANY; /* INADDR_ANY puts your IP address automatically */
            bzero(&(server.sin_zero), 8);        /* zero the rest of the structure */

            if (bind(server_sock, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
            { /* calls bind() */
                perror("\nError: ");
                exit(0);
            }

            //Step 3: Communicate with clients
            while (1)
            {
                sin_size = sizeof(struct sockaddr_in);
                //Nhân mảng được truyền lên server
                bytes_received = recvfrom(server_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *)&client, &sin_size);

                if (bytes_received < 0)
                    perror("\nError: ");
                else
                {
                    buff[bytes_received] = '\0';
                    printf("[%s:%d]: %s", inet_ntoa(client.sin_addr), client.sin_port, buff);
                    if (convertString(buff, number, charater) == 1)
                    {
                        //Ghép xâu 
                        strcpy(result, "Result: \n");
                        strcat(result, number);
                        strcat(result, "\n");
                        strcat(result, charater);
                    }
                    else
                    {
                        strcpy(result, "Error \n");
                    }
                }

                bytes_sent = sendto(server_sock, result, strlen(result), 0, (struct sockaddr *)&client, sin_size);
                if (bytes_sent < 0)
                    perror("\nError: ");
            }
        }
    }
}