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
Input: Nhận một mảng 
Process: Kiểm tra các kí tự của mảng để  xem địa chỉ truyền vào thuộc loại nào
Output: 0 - mảng chứa character != '.'
        1 - mảng có dạng địa chỉ có chấm
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
Input: Nhận một mảng để kiểm tra IP, một mảng để kiểm tra số hiệu cổng
Process: Kiểm tra các kí tự của mảng để  xem địa chỉ truyền vào thuộc loại nào
Output: 0 - nếu sai dạng địa chỉ IP hoặc sai số hiệu cổng
        1 - Địa chỉ IP và số hiệu cổng hợp lệ
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
main(int argc, char const *argv[])
{
    int client_sock, servPort;
    char buff[BUFF_SIZE], numberString[BUFF_SIZE], characterString[BUFF_SIZE];
    struct sockaddr_in server_addr;
    int bytes_sent, bytes_received, sin_size;
    if (argc != 3)
    {
        printf("\nNhập sai cú pháp\n");
    }
    else
    {
        //Check input : IP address & Port 
        if (checkInput(argv[1], argv[2]) != 0)
        {
            servPort = atoi(argv[2]);

            if ((client_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
            { /* calls socket() */
                perror("\nError: ");
                exit(0);
            }

            // Define the address of the server
            bzero(&server_addr, sizeof(server_addr));
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(servPort);
            server_addr.sin_addr.s_addr = inet_addr(argv[1]);
            for (;;)
            {

                // Communicate with server
                printf("\nInsert string to send:");
                memset(buff, '\0', (strlen(buff) + 1));
                fgets(buff, BUFF_SIZE, stdin);
                // Nếu enter thì mảng sẽ có độ dài là 1, nếu enter sẽ đóng socket ở client
                if (strlen(buff) == 1)
                {
                    printf("\nGood bye\n");
                    shutdown(client_sock, SHUT_RDWR);
                    break;
                }
                sin_size = sizeof(struct sockaddr);
                //Dữ liệu được gửi
                bytes_sent = sendto(client_sock, buff, strlen(buff), 0, (struct sockaddr *)&server_addr, sin_size);
                if (bytes_sent < 0)
                {
                    perror("Error: ");
                    close(client_sock);
                    return 0;
                }
                //Dữ liệu nhận được bao gồm 1 chuỗi chứa 2 chuỗi con: chuỗi toàn số và chuỗi toàn số
                bytes_received = recvfrom(client_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *)&server_addr, &sin_size);
                if (bytes_received < 0)
                {
                    perror("Error: ");
                    close(client_sock);
                    return 0;
                }
                else
                {
                    buff[bytes_received] = '\0';
                    printf("%s\n", buff);
                }
            }
            close(client_sock);
            return 0;
        }
    }
}