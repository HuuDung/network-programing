#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>

#define BUFF_SIZE 1024
#define ERROR_FILE_IS_EXISTENT "Error: File is existent on server"
#define ERROR_FILE_NOT_FOUND "Error: File not found"
#define ERROR_TRANFERING "Error: File tranfering is interupted"

/*
check value between 2dots is valid [0,255]
Input: int value
Output: true- 1
        false -0
*/
int checkValue(int value)
{
    if (value >= 0 && value <= 255)
        return 1;
    else
        return 0;
}
/*check isCharacter*/
/*
Input: char a
Output: true-1
        false-0
*/
int checkDigit(char a)
{
    if (a >= '0' && a <= '9')
        return 1;
    else
        return 0;
}
// Count dot in string
int checkDotInString(char *string)
{
    int i, count = 0;
    if (string[0] == '.' || string[strlen(string) - 1] == '.') //at last address has dot?
        return 0;
    for (i = 0; i < strlen(string) - 1; i++)
    {
        if (string[i] == '.' && string[i + 1] == '.')
            return 0;
        if (string[i] == '.') //count dot
            count++;
    }
    if (count != 3) //dot != 3
        return 0;
    return 1;
}
/*
Input: string
Process: type of address
Output: 0 - not IP
        1 - IP
*/
int checkIP(char *string)
{
    int i, value = 0;
    if (checkDotInString(string))
    {
        for (i = 0; i < strlen(string); i++)
        {
            if (string[i] != '.')
            {
                if (checkDigit(string[i])) //
                {
                    value = value * 10 + string[i] - '0'; 
                    if (i == strlen(string) - 1)
                        if (!checkValue(value))
                            return 0;
                }
                else
                    return 0;
            }
            else
            {
                if (!checkValue(value))
                    return 0;
                value = 0;
            }
        }
        return 1;
    }
    else
        return 0;
}
/*
Input: address, port
Output: 0 - false
        1 - true
*/
int checkInput(char *ipaddr, char *port)
{
    int i, checkPort = 1;
    if (checkIP(ipaddr) == 0)
    {
        printf("\nIP address invalid\n");
        return 0;
    }

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
int main(int argc, char const *argv[])
{
    int client_sock, servPort;
    char buff[BUFF_SIZE], data[BUFF_SIZE], status[BUFF_SIZE];
    struct sockaddr_in server_addr; /* server's address information */
    int msg_len, bytes_sent, bytes_received;
    long fileSize;
    FILE *fin;
    if (argc != 3)
    {
        printf("\nNhập sai cú pháp\n");
    }
    else
    {
        //Check input : IP address & Port
        if (checkInput(argv[1], argv[2]) != 0)
        {
            //Step 1: Construct socket
            client_sock = socket(AF_INET, SOCK_STREAM, 0);

            //Step 2: Specify server address
            servPort = atoi(argv[2]);

            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(servPort);
            server_addr.sin_addr.s_addr = inet_addr(argv[1]);
            //Step 3: Request to connect server
            if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
            {
                printf("\nError!Can not connect to sever! Client exit imediately! ");
                return 0;
            }

            //Step 4: Communicate with server
            while (1)
            {
                //send message
                printf("\nInsert to the file_name:");
                memset(buff, '\0', (strlen(buff) + 1));
                fgets(buff, BUFF_SIZE, stdin);
                buff[strlen(buff) - 1] = '\0';
                msg_len = strlen(buff);
                if (msg_len == 0)
                    break;
                fin = fopen(buff, "rb");
                if (fin) //exist file 
                {
                    bytes_sent = send(client_sock, buff, strlen(buff), 0); //send filename 
                    if (bytes_sent <= 0)
                    {
                        printf("\nConnection closed!\n");
                        break;
                    }
                    bytes_received = recv(client_sock, data, BUFF_SIZE - 1, 0); // status of server 
                    data[bytes_received] = '\0';
                    if (strcmp(data, "success") == 0)
                    {
                        fseek(fin, 0, SEEK_END);
                        long fileSize = ftell(fin); // size of file
                        fseek(fin, 0, SEEK_SET);

                        bytes_sent = send(client_sock, &fileSize, sizeof(long), 0);// send sizeof file to server

                        if (bytes_sent <= 0)
                        {
                            printf("\nConnection closed!\n");
                            break;
                        }
                        while (!feof(fin)) //read file
                        {
                            fread(data, sizeof(char), 1, fin);
                            if (feof(fin))
                                break;
                            bytes_sent = send(client_sock, data, 1, 0); //send byte of file to server
                            if(bytes_sent <0)
                            {
                                printf("%s\n", ERROR_TRANFERING);// error during send file
                            }
                        }
                        fclose(fin);
                    }
                    else
                    {
                        printf("%s\n", data);
                    }
                }
                else
                {
                    printf("%s\n", ERROR_FILE_NOT_FOUND);
                }
            }
            close(client_sock);
            //Step 4: Close socket
            return 0;
        }
    }
}