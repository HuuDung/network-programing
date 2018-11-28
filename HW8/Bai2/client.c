/*UDP Echo Client*/
#include <stdio.h> /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFF_SIZE 1024
/*check value between two dots*/
/*
Function: check value [0;255]
Input: int value
Output: true-1
        false-0
*/

int checkValue(int value)
{
	if (value >= 0 && value <= 255)
		return 1;
	else
		return 0;
}
/*check character is digit?*/
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
// check number dot in string
/*
Input: string
*/
int checkDotInString(char *string)
{
	int i, count = 0;
	if (string[0] == '.' || string[strlen(string) - 1] == '.') //last character is dot?
		return 0;
	for (i = 0; i < strlen(string) - 1; i++)
	{
		if (string[i] == '.' && string[i + 1] == '.')
			return 0;
		if (string[i] == '.') //check count dot
			count++;
	}
	if (count != 3) //if count != 3 =>false
		return 0;
	return 1;
}
/*
Input: array
Output: 0- IP but value not in [0;255]
        1- IP valid
        2- domain 
*/
int checkIP(char *string)
{
	int i, value = 0;
	if (checkDotInString(string))
	{
		for (i = 0; i < strlen(string); i++) //Đọc từng kí tự của string
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
					return 2;
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
	{
		return 2;
	}
}
int checkInput(char *ipaddr, char *port)
{
	int i, checkPort = 1;
	if (checkIP(ipaddr) != 1)
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
	char buff[BUFF_SIZE], numberString[BUFF_SIZE], characterString[BUFF_SIZE];
	char result_received[BUFF_SIZE], revc_status[BUFF_SIZE];
	struct sockaddr_in server_addr;
	int bytes_sent, bytes_received;
	socklen_t sin_size;
	if (argc != 3)
	{
		printf("\nNhập sai cú pháp\n");
	}
	else
	{
		if (checkInput(argv[1], argv[2]) != 0)
		{
			servPort = atoi(argv[2]);

			if ((client_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
			{ /* calls socket() */
				perror("\nError: ");
				exit(0);
			}

			//Define the address of the server
			bzero(&server_addr, sizeof(server_addr));
			server_addr.sin_family = AF_INET;
			server_addr.sin_port = htons(servPort);
			server_addr.sin_addr.s_addr = inet_addr(argv[1]);
			for (;;)
			{
				//Communicate with server
				printf("\nInsert string to send:");
				memset(buff, '\0', (strlen(buff) + 1));
				fgets(buff, BUFF_SIZE, stdin);
				if (strlen(buff) == 1)
				{
					printf("\nGood bye\n");
					shutdown(client_sock, SHUT_RDWR);
					break;
				}
				sin_size = sizeof(struct sockaddr);
				buff[strlen(buff) - 1] = '\0';
				//data sent
				bytes_sent = sendto(client_sock, buff, strlen(buff), 0, (struct sockaddr *)&server_addr, sin_size);
				if (bytes_sent < 0)
				{
					perror("Error: ");
					close(client_sock);
					return 0;
				}

				memset(result_received, '\0', (strlen(result_received) + 1));
				memset(revc_status, '\0', (strlen(revc_status) + 1));

				//data recveiced
				bytes_received = recvfrom(client_sock, revc_status, BUFF_SIZE, 0, (struct sockaddr *)&server_addr, &sin_size);
				if (bytes_received < 0)
				{
					perror("Error: ");
					close(client_sock);
					return 0;
				}
				else
				{
					revc_status[bytes_received] = '\0';
					// result's status
					if (strcmp(revc_status, "true") == 0) 
					{
						//offical information
						bytes_received = recvfrom(client_sock, result_received, BUFF_SIZE, 0, (struct sockaddr *)&server_addr, &sin_size);
						if (bytes_received < 0)
						{
							perror("Error: ");
							close(client_sock);
							return 0;
						}
						else
						{
							result_received[bytes_received] = '\0';
							printf("%s", result_received);
						}
						//alias information
						bytes_received = recvfrom(client_sock, result_received, BUFF_SIZE, 0, (struct sockaddr *)&server_addr, &sin_size);
						if (bytes_received < 0)
						{
							perror("Error: ");
							close(client_sock);
							return 0;
						}
						else
						{
							result_received[bytes_received] = '\0';
							printf("%s", result_received);
						}
					}
					else	//not found information
					{
						bytes_received = recvfrom(client_sock, result_received, BUFF_SIZE, 0, (struct sockaddr *)&server_addr, &sin_size);
						if (bytes_received < 0)
						{
							perror("Error: ");
							close(client_sock);
							return 0;
						}
						else
						{
							result_received[bytes_received] = '\0';
							printf("%s", result_received);
						}
					}
				}
			}
		}
	}
	close(client_sock);
	return 0;
}