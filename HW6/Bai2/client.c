#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>

#define BUFF_SIZE 1024

#define SYNTAX_ERROR "00"
#define INVALID_OPERATION "01"

#define CANNOT_FIND_ACCOUNT "10"
#define FINDED_ACCOUNT "11"
#define ACCOUNT_IS_BLOCKED "12"
#define SIGN_IN_ONLY_ONE_ACCOUNT "13"

#define PASSWORD_IS_INCORRECT "20"
#define SIGN_IN_SUCCESSFULL "21"
#define PASSWORD_IS_INCORRECT_OVER_THREE_TIMES "22"

#define ACCOUNT_IS_NOT_SIGN_IN "30"
#define LOGOUT_SUCCESSFULL "31"
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
                    value = value * 10 + string[i] - '0'; //Lấy giá trị giữa các dấu chấm và kiểm tra xem có nằm trong [0;255] không
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
void messageError(char *input)
{
    if (strcmp(input, CANNOT_FIND_ACCOUNT) == 0)
        printf("Cannot find account\n");
    else if (strcmp(input, SIGN_IN_ONLY_ONE_ACCOUNT) == 0)
        printf("Login only one account\n");
    else if (strcmp(input, ACCOUNT_IS_BLOCKED) == 0)
        printf("Account is blocked\n");
    else if (strcmp(input, PASSWORD_IS_INCORRECT) == 0)
        printf("Password is incorrect\n");
    else if (strcmp(input, SYNTAX_ERROR) == 0)
        printf("Syntax error\n");
    else if (strcmp(input, INVALID_OPERATION) == 0)
        printf("Invalid operation\n");
    else if (strcmp(input, PASSWORD_IS_INCORRECT_OVER_THREE_TIMES) == 0)
        printf("Password is incorrect. Account is blocked\n");
    else if (strcmp(input, ACCOUNT_IS_NOT_SIGN_IN) == 0)
        printf("Account is not sign in\n");
}
void messageSuccess(char *input)
{
    if (strcmp(input, SIGN_IN_SUCCESSFULL) == 0)
        printf("Login Successfull\n");
    else if (strcmp(input, LOGOUT_SUCCESSFULL) == 0)
        printf("Goodbye \n");
}
int main(int argc, char const *argv[])
{
    int client_sock, servPort;
    char buff[BUFF_SIZE], username[BUFF_SIZE];
    struct sockaddr_in server_addr; /* server's address information */
    int msg_len, bytes_sent, bytes_received;
    char code[BUFF_SIZE];
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
            while (1)
            {
                struct msg_code *sendData;
                printf("--------------------------------------\n");
                printf("\nTutorial: ");
                printf("\n\tSignin syntax: USER username");
                printf("\n\tPassword syntax: PASS password");
                printf("\n\tLogout syntax: LOGOUT username");
                printf("\n\tQuit syntax: QUIT ");
                printf("\n\t\tInput to syntax: \n");
                memset(code, '\0', (strlen(buff) + 1));
                fgets(code, BUFF_SIZE, stdin);
                code[strlen(code) - 1] = '\0';
                if (strcmp(code, "QUIT") == 0)
                    break;
                bytes_sent = send(client_sock, code, strlen(code), 0);
                if (bytes_sent <= 0)
                {
                    printf("Error\n");
                    break;
                }

                bytes_received = recv(client_sock, buff, BUFF_SIZE - 1, 0);
                if (bytes_received <= 0)
                {
                    printf("Error\n");
                    break;
                }
                buff[bytes_received] = '\0';
                messageError(buff);
                messageSuccess(buff);
            }
            //Step 5: Close socket
            close(client_sock);
            return 0;
        }
    }
}