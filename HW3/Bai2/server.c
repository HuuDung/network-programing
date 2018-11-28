/*UDP Echo Server*/
#include <stdio.h> /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#define BUFF_SIZE 1024
/*Kiểm tra giá trị nằm giữa hai dấu chấm của địa chỉ IP có chấm*/
/*
Chức năng hàm: Kiểm tra giá trị có nằm trong khoảng [0;255]
Input: int value
Output: đúng-1
        sai-0
*/
int checkValue(int value)
{
    if (value >= 0 && value <= 255)
        return 1;
    else
        return 0;
}
/*Kiểm tra kí tự có phải là kí tự số hay không*/
/*
Input: char a
Output: đúng-1
        sai-0
*/
int checkDigit(char a)
{
    if (a >= '0' && a <= '9')
        return 1;
    else
        return 0;
}
// Kiểm tra số lượng chấm có trong địa chỉ
/*
Input: string
Process:
    - IP kí tự đầu và cuối không phải là dấu '.'
    - IP không có hai dấu chấm liên tiếp
    - IP chỉ có 3 dấu chấm
    - IP không có kí tự chữ
    - Giá trị nằm giữa các dấu chấm nằm trong khoảng [0;255]
*/
int checkDotInString(char *string)
{
    int i, count = 0;
    if (string[0] == '.' || string[strlen(string) - 1] == '.') //Kiểm tra dấu chấm có ở đầu hay cuối địa chỉ không
        return 0;
    for (i = 0; i < strlen(string) - 1; i++)
    {
        if (string[i] == '.' && string[i + 1] == '.') //Kiểm tra dấu chấm có phải là liên tiếp hay không
            return 0;
        if (string[i] == '.') //Kiểm tra số lượng dấu chấm
            count++;
    }
    if (count != 3) //Số lượng dấu chấm khác 3 sẽ fail
        return 0;
    return 1;
}
/*
Input: mảng nhập vào
Output: 0- string dạng IP nhưng value bị vượt quá [0;255]
        1- IP valid
        2- String chứa kí tự ->domain 
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
main(int argc, char const *argv[])
{

    int server_sock, servPort; /* file descriptors */
    char buff[BUFF_SIZE], result_alias[BUFF_SIZE], result_official[BUFF_SIZE], result_status[BUFF_SIZE];
    char enter[] = "\n";
    int bytes_sent, bytes_received;
    struct sockaddr_in server; /* server's address information */
    struct sockaddr_in client; /* client's address information */
    int sin_size;
    struct sockaddr_in cliaddr;
    struct in_addr ipv4addr;
    struct hostent *host;
    struct in_addr **addr_list;
    int i;

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

            //Communicate with clients
            while (1)
            {
                sin_size = sizeof(struct sockaddr_in);

                bytes_received = recvfrom(server_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *)&client, &sin_size);
                if (bytes_received < 0)
                    perror("\nError: ");
                else
                {
                    buff[bytes_received] = '\0';
                    printf("[%s:%d]: %s \n", inet_ntoa(client.sin_addr), client.sin_port, buff);
                }
                memset(result_alias, '\0', (strlen(result_alias) + 1));
                memset(result_official, '\0', (strlen(result_official) + 1));
                memset(result_status, '\0', (strlen(result_status) + 1));

                if (checkIP(buff) == 1) // IP
                {
                    inet_pton(AF_INET, buff, &ipv4addr);
                    host = gethostbyaddr(&ipv4addr, sizeof(ipv4addr), AF_INET);
                    if (host != NULL)
                    {
                        //Gửi về client trạng thái
                        strcpy(result_status, "true");
                        bytes_sent = sendto(server_sock, result_status, strlen(result_status), 0, (struct sockaddr *)&client, sin_size);
                        //Gửi về client Official name
                        strcpy(result_official, "\nOfficial name: ");
                        strcat(result_official, host->h_name);
                        strcat(result_official, enter);
                        bytes_sent = sendto(server_sock, result_official, strlen(result_official), 0, (struct sockaddr *)&client, sin_size);
                        if (bytes_sent < 0)
                            perror("\nError: ");
                        if (result_alias[1] != NULL) //Kiểm tra có alias name k
                            strcpy(result_alias, "Alias name: \n");
                        else
                            //Gửi về client alias name
                            strcpy(result_alias, enter);
                        for (i = 0; host->h_aliases[i] != NULL; i++)
                        {
                            strcat(result_alias, host->h_aliases[i]);
                            strcat(result_alias, enter);
                        }
                        bytes_sent = sendto(server_sock, result_alias, strlen(result_alias), 0, (struct sockaddr *)&client, sin_size);
                        //
                        if (bytes_sent < 0)
                            perror("\nError: ");
                    }
                    else
                    {
                        //Gửi về client trạng thái
                        strcpy(result_status, "false");
                        bytes_sent = sendto(server_sock, result_status, strlen(result_status), 0, (struct sockaddr *)&client, sin_size);
                        //Gửi về thông báo lỗi
                        strcpy(result_official, "Not found information");
                        strcat(result_official, enter);
                        bytes_sent = sendto(server_sock, result_official, strlen(result_official), 0, (struct sockaddr *)&client, sin_size);
                    }
                }
                else if (checkIP(buff) == 2)
                {
                    host = gethostbyname(buff);
                    if (host != NULL)
                    {
                        //Gửi về client trạng thái
                        strcpy(result_status, "true");
                        bytes_sent = sendto(server_sock, result_status, strlen(result_status), 0, (struct sockaddr *)&client, sin_size);

                        addr_list = (struct in_addr **)host->h_addr_list;
                        //Ghép xâu gửi về cho client xâu chứa official IP
                        strcpy(result_official, "\nOfficial IP: ");
                        strcat(result_official, inet_ntoa(*addr_list[0]));
                        strcat(result_official, enter);
                        bytes_sent = sendto(server_sock, result_official, strlen(result_official), 0, (struct sockaddr *)&client, sin_size);
                        if (bytes_sent < 0)
                            perror("\nError: ");
                        //Kiểm tra có IP phụ không
                        if (addr_list[1] != NULL)
                            strcpy(result_alias, "Alias IP: \n");
                        else
                        {
                            strcpy(result_alias, enter);
                        }
                        for (i = 1; addr_list[i] != NULL; i++)
                        {
                            strcat(result_alias, inet_ntoa(*addr_list[i]));
                            strcat(result_alias, enter);
                        }
                        //Ghép xâu gửi về cho client xâu chứa alias IP
                        bytes_sent = sendto(server_sock, result_alias, strlen(result_alias), 0, (struct sockaddr *)&client, sin_size);
                    }
                    else
                    {
                        //Gửi về client trạng thái
                        strcpy(result_status, "false");
                        bytes_sent = sendto(server_sock, result_status, strlen(result_status), 0, (struct sockaddr *)&client, sin_size);
                        //Gửi về thông báo lỗi
                        strcpy(result_official, "Not found information");
                        strcat(result_official, enter);
                        bytes_sent = sendto(server_sock, result_official, strlen(result_official), 0, (struct sockaddr *)&client, sin_size);
                    }
                }
                else
                {
                    //Gửi về client trạng thái
                    strcpy(result_status, "false");
                    bytes_sent = sendto(server_sock, result_status, strlen(result_status), 0, (struct sockaddr *)&client, sin_size);
                    //Gửi về thông báo lỗi
                    strcpy(result_official, "IP Address is invalid");
                    strcat(result_official, enter);
                    bytes_sent = sendto(server_sock, result_official, strlen(result_official), 0, (struct sockaddr *)&client, sin_size);
                }
            }
        }
    }
}