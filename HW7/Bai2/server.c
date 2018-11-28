#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#define BACKLOG 20 /* Number of allowed connections */
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

/* The processData function copies the input string to output */
void processData(struct account *acc, int *status, char *code, char *data, char *recv_data, char *result);

/* The recv() wrapper function*/
int receiveData(int s, char *buff, int size, int flags);

/* The send() wrapper function*/
int sendData(int s, char *buff, int size, int flags);

int main(int argc, char const *argv[])
{
	int i, maxi, maxfd, listenfd, connfd, sockfd;
	int nready, client[FD_SETSIZE];
	ssize_t ret;
	fd_set readfds, allset;
	char sendBuff[BUFF_SIZE], rcvBuff[BUFF_SIZE];
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;
	char code[BUFF_SIZE], data[BUFF_SIZE], result[BUFF_SIZE];
	struct account *acc[BUFF_SIZE];
	int status[BUFF_SIZE];
	if (argc != 2)
	{
		printf("Params invalid\n");
	}
	else
	{
		if (checkPort(argv[1]) == 1)
		{
			//Step 1: Construct a TCP socket to listen connection request
			if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
			{ /* calls socket() */
				perror("\nError: ");
				return 0;
			}

			//Step 2: Bind address to socket
			bzero(&servaddr, sizeof(servaddr));
			servaddr.sin_family = AF_INET;
			servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
			servaddr.sin_port = htons(atoi(argv[1]));

			if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
			{ /* calls bind() */
				perror("\nError: ");
				return 0;
			}

			//Step 3: Listen request from client
			if (listen(listenfd, BACKLOG) == -1)
			{ /* calls listen() */
				perror("\nError: ");
				return 0;
			}

			maxfd = listenfd; /* initialize */
			maxi = -1;		  /* index into client[] array */
			for (i = 0; i < FD_SETSIZE; i++)
			{
				client[i] = -1; /* -1 indicates available entry */
				status[i] = LOG_IN;
			}

			FD_ZERO(&allset);
			FD_SET(listenfd, &allset);

			//Step 4: Communicate with clients
			while (1)
			{
				readfds = allset; /* structure assignment */
				nready = select(maxfd + 1, &readfds, NULL, NULL, NULL);
				if (nready < 0)
				{
					perror("\nError: ");
					return 0;
				}

				if (FD_ISSET(listenfd, &readfds))
				{ /* new client connection */
					clilen = sizeof(cliaddr);
					if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen)) < 0)
						perror("\nError: ");
					else
					{
						printf("You got a connection from %s\n", inet_ntoa(cliaddr.sin_addr)); /* prints client's IP */
						for (i = 0; i < FD_SETSIZE; i++)
							if (client[i] < 0)
							{
								client[i] = connfd; /* save descriptor */
								break;
							}
						if (i == FD_SETSIZE)
						{
							printf("\nToo many clients");
							close(connfd);
						}

						FD_SET(connfd, &allset); /* add new descriptor to set */
						if (connfd > maxfd)
							maxfd = connfd; /* for select */
						if (i > maxi)
							maxi = i; /* max index in client[] array */

						if (--nready <= 0)
							continue; /* no more readable descriptors */
					}
				}

				for (i = 0; i <= maxi; i++)
				{ /* check all clients for data */
					if ((sockfd = client[i]) < 0)
						continue;
					if (FD_ISSET(sockfd, &readfds))
					{
						ret = receiveData(sockfd, rcvBuff, BUFF_SIZE - 1, 0);
						if (ret <= 0)
						{
							FD_CLR(sockfd, &allset);
							close(sockfd);
							client[i] = -1;
						}
						else
						{
							rcvBuff[ret] = '\0';
							// status_client = status[i];
							// processData(acc, &status_client, code, data, rcvBuff, result);
							// status[i]=status_client;

							readFileAndMakeAccount(); // Readfile and make account in linklist
							if (checkSpace(rcvBuff))
							{
								splitMessage(rcvBuff, code, data);
								if (strcmp(code, "USER") == 0) // code = "USER"
								{
									switch (status[i])
									{
									case LOG_IN:
										if (findUserNameAccount(data) != NULL)
										{
											acc[i] = findUserNameAccount(data);
											if (acc[i]->account == BLOCKED)
											{
												strcpy(result, ACCOUNT_IS_BLOCKED);
											}
											else
											{
												status[i] = UNAUTHENTICATE;
												strcpy(result, FINDED_ACCOUNT);
											}
										}
										else
										{
											printf("\n%s\t%s\n", code, data);
											strcpy(result, CANNOT_FIND_ACCOUNT);
										}
										break;
									case UNAUTHENTICATE:
										if (findUserNameAccount(data) != NULL)
										{
											acc[i] = findUserNameAccount(data);
											if (acc[i]->account == BLOCKED)
											{
												status[i] = LOG_IN;
												strcpy(result, ACCOUNT_IS_BLOCKED);
											}
											else
											{
												strcpy(result, FINDED_ACCOUNT);
											}
										}
										else
										{
											status[i] = LOG_IN;
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
									switch (status[i])
									{
									case UNAUTHENTICATE:
										if (strcmp(data, acc[i]->password) != 0)
										{
											acc[i]->countSignIn++;
											if (acc[i]->countSignIn >= 3)
											{
												status[i] = LOG_IN;
												acc[i]->account = BLOCKED;
												strcpy(result, PASSWORD_IS_INCORRECT_OVER_THREE_TIMES);
											}
											else
											{
												strcpy(result, PASSWORD_IS_INCORRECT);
											}
										}
										else
										{
											status[i] = AUTHENTICATED;
											acc[i]->status = ONLINE;
											acc[i]->countSignIn = 0;
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
									switch (status[i])
									{
									case AUTHENTICATED:
										if (strcmp(data, acc[i]->username) == 0)
										{
											status[i] = LOG_IN;
											acc[i]->status = OFFLINE;
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
								saveFile();
							}
							else
							{
								strcpy(result, SYNTAX_ERROR);
							}

							printf("[%s:%d]\t", inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
							messageError(result);
							messageSuccess(result, acc[i]);
							sendData(sockfd, result, BUFF_SIZE, 0);
						}

						if (--nready <= 0)
							break; /* no more readable descriptors */
					}
				}
			}

			return 0;
		}
	}
}

void processData(struct account *acc, int *status, char *code, char *data, char *recv_data, char *result)
{
}

int receiveData(int s, char *buff, int size, int flags)
{
	int n;
	n = recv(s, buff, size, flags);
	if (n < 0)
		perror("Error: ");
	return n;
}

int sendData(int s, char *buff, int size, int flags)
{
	int n;
	n = send(s, buff, size, flags);
	if (n < 0)
		perror("Error: ");
	return n;
}