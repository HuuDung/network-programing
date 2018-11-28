#include <stdio.h> /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

#define BACKLOG 2 /* Number of allowed connections */
#define BUFF_SIZE 1024

#define ONLINE 1
#define OFFLINE 0

#define ACTIVE 1
#define BLOCKED 0

#define LOG_IN 1
#define UNAUTHENTICATE 2
#define AUTHENTICATED 3

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
struct account
{
    char username[30];
    char password[30];
    int status;
    int account;
    int countSignIn;
    struct account *next;
};

struct account *head = NULL;
struct account *current = NULL;

struct account *findUserNameAccount(char *username)
{
    struct account *ptr = NULL;
    for (ptr = head; ptr != NULL; ptr = ptr->next)
    {
        if (strcmp(ptr->username, username) == 0)
            return ptr;
    }
    return NULL;
}

struct account *newAccount(char *username, char *password, int account)
{
    struct account *new = (struct account *)malloc(sizeof(struct account));
    strcpy(new->username, username);
    strcpy(new->password, password);
    new->account = account;
    new->status = OFFLINE;
    new->countSignIn = 0;
    new->next = NULL;
    return new;
}
void addAccount(struct account *new)
{
    if (head == NULL)
    {
        head = new;
        current = new;
    }
    else
    {
        current->next = new;
        current = new;
    }
}

void readFileAndMakeAccount()
{
    FILE *fin;
    head = NULL;
    current = NULL;
    char username[30], password[20];
    int account;
    char c;
    fin = fopen("account.txt", "r");
    struct account *new = NULL;
    while (!feof(fin))
    {
        fscanf(fin, "%s%c%s%c%d%c", username, &c, password, &c, &account, &c);
        if (feof(fin))
            break;
        new = newAccount(username, password, account);
        addAccount(new);
    }
    fclose(fin);
}

void saveFile()
{
    FILE *fout;
    struct account *ptr = NULL;
    fout = fopen("account.txt", "w");
    for (ptr = head; ptr != NULL; ptr = ptr->next)
    {
        fprintf(fout, "%s%c%s%c%d%c", ptr->username, ' ', ptr->password, ' ', ptr->account, '\n');
    }
    fclose(fout);
}
//Check port number
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
void printListAccount()
{
    struct account *ptr = NULL;
    for (ptr = head; ptr != NULL; ptr = ptr->next)
    {
        printf("Username: %s\n", ptr->username);
        printf("Account: %s\n", ptr->account == ACTIVE ? "Active" : "Blocked");
        printf("Status : %s\n", ptr->status == ONLINE ? "Online" : "Offline");
        printf("Pass: %s\n", ptr->password);
        printf("Count wrong password: %d\n", ptr->countSignIn);
        printf("\n");
    }
}
/*
Check sapce in string from client
*/
int checkSpace(char *input)
{
    int i, result = 1, count = 0;
    if (input[0] == ' ' || input[strlen(input) - 1] == ' ')
        return 0;
    else
    {
        for (i = 0; i < strlen(input); i++)
        {
            if (input[i] == ' ')
                count++;
        }
        if (count == 0)
            return 0;
        else
            return 1;
    }
}
/*
input: string from client
output: string code and string data
*/
void splitMessage(char *input, char *code, char *data)
{
    int i, codeLength = 0, dataLength = 0;
    for (i = 0; input[i] != ' '; i++)
    {
        code[codeLength++] = input[i];
    }
    code[codeLength] = '\0';
    i++;
    for (i; i < strlen(input); i++)
    {
        data[dataLength++] = input[i];
    }
    data[dataLength] = '\0';
}
/*
input: code of message
output: print message 
*/
void messageError(char *input)
{
    if (strcmp(input, CANNOT_FIND_ACCOUNT) == 0)
        printf("%s\tCannot find account\n", input);
    else if (strcmp(input, SIGN_IN_ONLY_ONE_ACCOUNT) == 0)
        printf("%s\tLogin only one account\n", input);
    else if (strcmp(input, ACCOUNT_IS_BLOCKED) == 0)
        printf("%s\tAccount is blocked\n", input);
    else if (strcmp(input, PASSWORD_IS_INCORRECT) == 0)
        printf("%s\tPassword is incorrect\n", input);
    else if (strcmp(input, SYNTAX_ERROR) == 0)
        printf("%s\tSyntax error\n", input);
    else if (strcmp(input, INVALID_OPERATION) == 0)
        printf("%s\tInvalid operation\n", input);
    else if (strcmp(input, PASSWORD_IS_INCORRECT_OVER_THREE_TIMES) == 0)
        printf("%s\tPassword is incorrect. Account is blocked\n", input);
    else if (strcmp(input, ACCOUNT_IS_NOT_SIGN_IN) == 0)
        printf("%s\tAccount is not sign in\n", input);
}
void messageSuccess(char *input, struct account *acc)
{
    if (strcmp(input, SIGN_IN_SUCCESSFULL) == 0)
        printf("%s\tLogin successfull %s\n", input, acc->username);
    else if (strcmp(input, LOGOUT_SUCCESSFULL) == 0)
        printf("%s\tGoodbye %s\n", input, acc->username);
    else if (strcmp(input, FINDED_ACCOUNT) == 0)
        printf("%s\tUSER %s\n", input, acc->username);
}
int main(int argc, char const *argv[])
{

    int listen_sock, conn_sock, servPort; /* file descriptors */
    char recv_data[BUFF_SIZE], result[BUFF_SIZE];
    int bytes_sent, bytes_received;
    struct sockaddr_in server; /* server's address information */
    struct sockaddr_in client; /* client's address information */
    int sin_size, status, messageStatus;
    char code[BUFF_SIZE], data[BUFF_SIZE];
    pid_t pid;

    if (argc != 2)
    {
        printf("\nError params");
    }
    else
    {
        if (checkPort(argv[2]) == 1)
        {

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
                {
                    if (errno == EINTR)
                        continue;
                    else
                    {
                        perror("\nError: ");
                        return 0;
                    }
                }
                pid = fork();
                if (pid == 0)
                {
                    close(listen_sock);
                    printf("You got a connection from %s\n", inet_ntoa(client.sin_addr)); /* prints client's IP */
                    //start conversation
                    struct account *acc;
                    status = LOG_IN;
                    readFileAndMakeAccount(); // Readfile and make account in linklist
                    while (1)
                    {
                        bytes_received = recv(conn_sock, recv_data, BUFF_SIZE - 1, 0); //blocking
                        if (bytes_received <= 0)
                        {
                            printf("\nConnection %s closed\n", inet_ntoa(client.sin_addr));
                            break;
                        }
                        recv_data[bytes_received] = '\0';
                        if (checkSpace(recv_data))
                        {
                            splitMessage(recv_data, code, data);
                            if (strcmp(code, "USER") == 0) // code = "USER"
                            {
                                switch (status)
                                {
                                case LOG_IN:
                                    if (findUserNameAccount(data) != NULL)
                                    {
                                        acc = findUserNameAccount(data);
                                        if (acc->account == BLOCKED)
                                        {
                                            strcpy(result, ACCOUNT_IS_BLOCKED);
                                        }
                                        else
                                        {
                                            status = UNAUTHENTICATE;
                                            strcpy(result, FINDED_ACCOUNT);
                                        }
                                    }
                                    else
                                    {
                                        strcpy(result, CANNOT_FIND_ACCOUNT);
                                    }
                                    break;
                                case UNAUTHENTICATE:
                                    if (findUserNameAccount(data) != NULL)
                                    {
                                        acc = findUserNameAccount(data);
                                        if (acc->account == BLOCKED)
                                        {
                                            status = LOG_IN;
                                            strcpy(result, ACCOUNT_IS_BLOCKED);
                                        }
                                        else
                                        {
                                            strcpy(result, FINDED_ACCOUNT);
                                        }
                                    }
                                    else
                                    {
                                        status = LOG_IN;
                                        strcpy(result, CANNOT_FIND_ACCOUNT);
                                    }
                                    break;
                                case AUTHENTICATED:
                                    strcpy(result, SIGN_IN_ONLY_ONE_ACCOUNT);
                                    break;
                                }
                            }
                            else if (strcmp(code, "PASS") == 0) //CODE = PASS
                            {
                                switch (status)
                                {
                                case UNAUTHENTICATE:
                                    if (strcmp(data, acc->password) != 0)
                                    {
                                        acc->countSignIn++;
                                        if (acc->countSignIn >= 3)
                                        {
                                            status = LOG_IN;
                                            acc->account = BLOCKED;
                                            strcpy(result, PASSWORD_IS_INCORRECT_OVER_THREE_TIMES);
                                        }
                                        else
                                        {
                                            strcpy(result, PASSWORD_IS_INCORRECT);
                                        }
                                    }
                                    else
                                    {
                                        status = AUTHENTICATED;
                                        acc->status = ONLINE;
                                        acc->countSignIn = 0;
                                        strcpy(result, SIGN_IN_SUCCESSFULL);
                                    }
                                    break;
                                default:
                                    strcpy(result, INVALID_OPERATION);
                                    break;
                                }
                            }
                            else if (strcmp(code, "LOGOUT") == 0) // CODE = LOGOUT
                            {
                                switch (status)
                                {
                                case AUTHENTICATED:
                                    if (strcmp(data, acc->username) == 0)
                                    {
                                        status = LOG_IN;
                                        acc->status = OFFLINE;
                                        strcpy(result, LOGOUT_SUCCESSFULL);
                                    }
                                    else
                                    {
                                        strcpy(result, ACCOUNT_IS_NOT_SIGN_IN);
                                    }
                                    break;
                                default:
                                    strcpy(result, ACCOUNT_IS_NOT_SIGN_IN);
                                    break;
                                }
                            }
                            else
                            {
                                strcpy(result, SYNTAX_ERROR);
                            }
                            bytes_sent = send(conn_sock, result, strlen(result), 0);
                            if (bytes_sent <= 0)
                            {
                                printf("Error\n");
                                break;
                            }
                            saveFile();
                        }
                        else
                        {
                            strcpy(result, SYNTAX_ERROR);
                            bytes_sent = send(conn_sock, result, strlen(result), 0);
                            if (bytes_sent <= 0)
                            {
                                printf("Error\n");
                                break;
                            }
                        }
                        printf("[%s:%d]\t", inet_ntoa(client.sin_addr), client.sin_port);
                        messageError(result);
                        messageSuccess(result, acc);
                    }
                    exit(0);
                } //end conversation
                close(conn_sock);
            }
            close(listen_sock);
            return 0;
        }
    }
}
