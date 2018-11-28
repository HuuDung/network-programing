#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
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
main(int argc, char **argv[])
{
    int i;
    struct in_addr ipv4addr;
    struct hostent *host;
    struct in_addr **addr_list;
    if (argc < 2)
        printf("Bạn cần nhập tham số đầu vào \n");
    else
    {

        if (checkIP(argv[1])) //Địa chỉ nhập vào IP
        {
            inet_pton(AF_INET, argv[1], &ipv4addr);                     //convert sang network address
            host = gethostbyaddr(&ipv4addr, sizeof(ipv4addr), AF_INET); //get host
            if (host != NULL)
            {
                printf("Official name: %s\n", host->h_name);
                if (host->h_aliases[0] != NULL) //Kiểm tra xem có tên địa chỉ thay thế không
                {
                    printf("Alias name : \n");
                    for (i = 0; host->h_aliases[i] != NULL; i++)
                    {
                        printf("%s\n", host->h_aliases[i]);
                    }
                }
            }
            else
            {
                printf("Not found information\n");
            }
        }
        //domain address
        else
        {
            host = gethostbyname(argv[1]); //get host
            if (host != NULL)
            {
                addr_list = (struct in_addr **)host->h_addr_list;      //get list IP address
                printf("Official IP: %s\n", inet_ntoa(*addr_list[0])); //địa chỉ IP đầu sẽ là địa chỉ offical
                if (addr_list[1] != NULL)
                {
                    printf("Alias IP: \n");
                    for (i = 1; addr_list[i] != NULL; i++)
                    {
                        printf("%s \n", inet_ntoa(*addr_list[i]));
                    }
                }
            }
            else
            {
                printf("Not found information\n");
            }
        }
    }
}
