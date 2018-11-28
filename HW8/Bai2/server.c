#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netdb.h>

#define QSIZE 8		   /* size of input queue */
#define MAXDG 4096	 /* max datagram size */
#define BUFF_SIZE 1024 /*max string*/

typedef struct
{
	void *dg_data;			/* ptr to actual datagram */
	size_t dg_len;			/* length of datagram */
	struct sockaddr *dg_sa; /* ptr to sockaddr{} w/client's address */
	socklen_t dg_salen;		/* length of sockaddr{} */
} DG;
static DG dg[QSIZE]; /* queue of datagrams to process */

static int iget;		 /* next one for main loop to process */
static int iput;		 /* next one for signal handler to read into */
static int nqueue;		 /* # on queue for main loop to process */
static socklen_t clilen; /* max length of sockaddr{} */
static int sockfd;

static void sig_io(int);
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
int main(int argc, char const *argv[])
{
	int i;
	int bytes_sent;
	char result_status[BUFF_SIZE], result_alias[BUFF_SIZE], result_official[BUFF_SIZE];
	const int on = 1;
	sigset_t zeromask, newmask, oldmask;

	struct sockaddr_in servaddr, cliaddr;

	struct in_addr ipv4addr;
	struct hostent *host;
	struct in_addr **addr_list;

	if (argc != 2)
	{
		printf("Nhập thiếu tham số\n");
	}
	else
	{
		if (checkPort(argv[1]) != 0)
		{
			//
			if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
			{ /* calls socket() */
				perror("socket() error\n");
				return 0;
			}

			bzero(&servaddr, sizeof(servaddr));
			servaddr.sin_family = AF_INET;
			servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
			servaddr.sin_port = htons(atoi(argv[1]));

			if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
			{ /* calls bind() */
				perror("bind() error\n");
				return 0;
			}

			clilen = sizeof(cliaddr);

			for (i = 0; i < QSIZE; i++)
			{ /* init queue of buffers */
				dg[i].dg_data = malloc(MAXDG);
				dg[i].dg_sa = malloc(clilen);
				dg[i].dg_salen = clilen;
			}
			iget = iput = nqueue = 0;

			/* Signal handlers are established for SIGIO. The socket owner is
	 * set using fcntl and the signal-driven and non-blocking I/O flags are set using ioctl
	 */
			signal(SIGIO, sig_io);
			fcntl(sockfd, F_SETOWN, getpid());
			ioctl(sockfd, FIOASYNC, &on);
			ioctl(sockfd, FIONBIO, &on);

			/* Three signal sets are initialized: zeromask (which never changes),
	 * oldmask (which contains the old signal mask when we block SIGIO), and newmask.
	 */
			sigemptyset(&zeromask);
			sigemptyset(&oldmask);
			sigemptyset(&newmask);
			sigaddset(&newmask, SIGIO); /* signal we want to block */

			/* Stores the current signal mask of the process in oldmask and then
	 * logically ORs newmask into the current signal mask. This blocks SIGIO
	 * and returns the current signal mask. We need SIGIO blocked when we test
	 * nqueue at the top of the loop
	 */
			sigprocmask(SIG_BLOCK, &newmask, &oldmask);

			for (;;)
			{
				while (nqueue == 0)
					sigsuspend(&zeromask); /* wait for datagram to process */

				/* unblock SIGIO by calling sigprocmask to set the signal mask of
		 * the process to the value that was saved earlier (oldmask).
		 * The reply is then sent by sendto.
		 */
				sigprocmask(SIG_SETMASK, &oldmask, NULL);

				if (checkIP(dg[iget].dg_data) == 1) // IP
				{
					inet_pton(AF_INET, dg[iget].dg_data, &ipv4addr);
					host = gethostbyaddr(&ipv4addr, sizeof(ipv4addr), AF_INET);
					if (host != NULL)
					{
						//send to client status
						strcpy(result_status, "true");
						bytes_sent = sendto(sockfd, result_status, strlen(result_status), 0, dg[iget].dg_sa, dg[iget].dg_salen);
						//send to client official name
						strcpy(result_official, "\nOfficial name: ");
						strcat(result_official, host->h_name);
						strcat(result_official, "\n");

						bytes_sent = sendto(sockfd, result_official, strlen(result_official), 0, dg[iget].dg_sa, dg[iget].dg_salen);
						if (bytes_sent < 0)
							perror("\nError: ");
						if (host->h_aliases[0] != NULL) //check alias name exist?
							strcpy(result_alias, "Alias name: \n");
						else
							//send to client alias name
							strcpy(result_alias, "\n");

						for (i = 0; host->h_aliases[i] != NULL; i++)
						{
							strcat(result_alias, host->h_aliases[i]);
							strcat(result_alias, "\n");
						}
						bytes_sent = sendto(sockfd, result_alias, strlen(result_alias), 0, dg[iget].dg_sa, dg[iget].dg_salen);
						//
						if (bytes_sent < 0)
							perror("\nError: ");
					}
					else
					{
						//send to client status
						strcpy(result_status, "false");
						bytes_sent = sendto(sockfd, result_status, strlen(result_status), 0, dg[iget].dg_sa, dg[iget].dg_salen);
						//send to cilent message error
						strcpy(result_official, "Not found information");
						strcat(result_official, "\n");
						bytes_sent = sendto(sockfd, result_official, strlen(result_official), 0, dg[iget].dg_sa, dg[iget].dg_salen);
					}
				}
				else if (checkIP(dg[iget].dg_data) == 2)
				{
					host = gethostbyname(dg[iget].dg_data);
					if (host != NULL)
					{
						//send to client status
						strcpy(result_status, "true");
						bytes_sent = sendto(sockfd, result_status, strlen(result_status), 0, dg[iget].dg_sa, dg[iget].dg_salen);

						addr_list = (struct in_addr **)host->h_addr_list;
						//combine official IP
						strcpy(result_official, "\nOfficial IP: ");
						strcat(result_official, inet_ntoa(*addr_list[0]));
						strcat(result_official, "\n");
						bytes_sent = sendto(sockfd, result_official, strlen(result_official), 0, dg[iget].dg_sa, dg[iget].dg_salen);
						if (bytes_sent < 0)
							perror("\nError: ");
						//check alias ip
						if (addr_list[1] != NULL)
							strcpy(result_alias, "Alias IP: \n");
						else
						{
							strcpy(result_alias, "\n");
						}
						for (i = 1; addr_list[i] != NULL; i++)
						{
							strcat(result_alias, inet_ntoa(*addr_list[i]));
							strcat(result_alias, "\n");
						}
						//send alias ip to client
						bytes_sent = sendto(sockfd, result_alias, strlen(result_alias), 0, dg[iget].dg_sa, dg[iget].dg_salen);
					}
					else
					{
						//send to client status
						strcpy(result_status, "false");
						bytes_sent = sendto(sockfd, result_status, strlen(result_status), 0, dg[iget].dg_sa, dg[iget].dg_salen);
						//send to cilent message error
						strcpy(result_official, "Not found information");
						strcat(result_official, "\n");
						bytes_sent = sendto(sockfd, result_official, strlen(result_official), 0, dg[iget].dg_sa, dg[iget].dg_salen);
					}
				}
				else
				{
					//send to client status
					strcpy(result_status, "false");
					bytes_sent = sendto(sockfd, result_status, strlen(result_status), 0, dg[iget].dg_sa, dg[iget].dg_salen);
					//send to cilent message error
					strcpy(result_official, "IP Address is invalid");
					strcat(result_official, "\n");
					bytes_sent = sendto(sockfd, result_official, strlen(result_official), 0, dg[iget].dg_sa, dg[iget].dg_salen);
				}

				if (++iget >= QSIZE)
					iget = 0;

				/* SIGIO is blocked and the value of nqueue is decremented.
		 * We must block the signal while modifying this variable since
		 * it is shared between the main loop and the signal handler.
		 */
				sigprocmask(SIG_BLOCK, &newmask, &oldmask);
				nqueue--;
			}
		}
	}
}

static void sig_io(int signo)
{
	ssize_t len;
	DG *ptr;

	for (;;)
	{
		if (nqueue >= QSIZE)
		{
			perror("receive overflow");
			break;
		}

		ptr = &dg[iput];
		ptr->dg_salen = clilen;
		len = recvfrom(sockfd, ptr->dg_data, MAXDG, 0,
					   ptr->dg_sa, &ptr->dg_salen);
		if (len < 0)
		{
			if (errno == EWOULDBLOCK)
				break; /* all done; no more queued to read */
			else
			{
				perror("recvfrom error");
				break;
			}
		}
		ptr->dg_len = len;

		nqueue++;
		if (++iput >= QSIZE)
			iput = 0;
	}
}
