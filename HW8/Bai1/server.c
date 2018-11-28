#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <unistd.h>

#define TRUE 1
#define FALSE 0
#define MAX 3
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
      for (i = 0; i < strlen(input); i++)
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

int main(int argc, char *argv[])
{
   int i, len, rc, on = 1;
   int listen_sd, max_sd, new_sd;
   int desc_ready, end_server = FALSE;
   int close_conn;
   char buffer[BUFF_SIZE], result[BUFF_SIZE], number[BUFF_SIZE], character[BUFF_SIZE];
   struct sockaddr_in addr;
   struct timeval timeout;
   fd_set master_set, working_set;
   struct iovec iov[MAX];
   if (argc != 2)
   {
      printf("Nhập thiếu tham số\n");
   }
   else
   {
      if (checkPort(argv[1]) != 0)
      {
         listen_sd = socket(AF_INET, SOCK_STREAM, 0);
         if (listen_sd < 0)
         {
            perror("socket() failed");
            exit(0);
         }

         /*************************************************************/
         /* Set socket to be nonblocking. All of the sockets for    */
         /* the incoming connections will also be nonblocking since  */
         /* they will inherit that state from the listening socket.   */
         /*************************************************************/
         rc = ioctl(listen_sd, FIONBIO, (char *)&on);
         if (rc < 0)
         {
            perror("ioctl() failed");
            close(listen_sd);
            exit(0);
         }

         memset(&addr, 0, sizeof(addr));
         addr.sin_family = AF_INET;
         addr.sin_addr.s_addr = htonl(INADDR_ANY);
         addr.sin_port = htons(atoi(argv[1]));
         rc = bind(listen_sd, (struct sockaddr *)&addr, sizeof(addr));
         if (rc < 0)
         {
            perror("bind() failed");
            close(listen_sd);
            exit(0);
         }

         rc = listen(listen_sd, 32);
         if (rc < 0)
         {
            perror("listen() failed");
            close(listen_sd);
            exit(0);
         }

         /*************************************************************/
         /* Initialize the master fd_set                              */
         /*************************************************************/
         FD_ZERO(&master_set);
         max_sd = listen_sd;
         FD_SET(listen_sd, &master_set);

         /*************************************************************/
         /* Initialize the timeval struct to 3 minutes.  If no        */
         /* activity after 3 minutes this program will end.           */
         /*************************************************************/
         timeout.tv_sec = 3 * 60;
         timeout.tv_usec = 0;

         /*************************************************************/
         /* Loop waiting for incoming connects or for incoming data   */
         /* on any of the connected sockets.                          */
         /*************************************************************/
         do
         {
            memcpy(&working_set, &master_set, sizeof(master_set));

            /**********************************************************/
            /* Call select() and wait 3 minutes for it to complete.   */
            /**********************************************************/
            printf("Waiting on select()...\n");
            rc = select(max_sd + 1, &working_set, NULL, NULL, &timeout);

            if (rc < 0)
            {
               perror("  select() failed");
               break;
            }

            /**********************************************************/
            /* Check to see if the 3 minute time out expired.         */
            /**********************************************************/
            if (rc == 0)
            {
               printf("  select() timed out.  End program.\n");
               break;
            }

            desc_ready = rc;
            for (i = 0; i <= max_sd && desc_ready > 0; ++i)
            {
               if (FD_ISSET(i, &working_set))
               {
                  desc_ready -= 1;

                  /****************************************************/
                  /* Check to see if this is the listening socket     */
                  /****************************************************/
                  if (i == listen_sd)
                  {
                     printf("Listening socket is readable\n");
                     do
                     {
                        /**********************************************/
                        /* Accept each incoming connection.  If       */
                        /* accept fails with EWOULDBLOCK, then we     */
                        /* have accepted all of them.  Any other      */
                        /* failure on accept will cause us to end the */
                        /* server.                                    */
                        /**********************************************/
                        new_sd = accept(listen_sd, NULL, NULL);
                        if (new_sd < 0)
                        {
                           if (errno != EWOULDBLOCK)
                           {
                              perror("  accept() failed");
                              end_server = TRUE;
                           }
                           break;
                        }

                        printf("  New incoming connection - %d\n", new_sd);
                        FD_SET(new_sd, &master_set);
                        if (new_sd > max_sd)
                           max_sd = new_sd;

                     } while (new_sd != -1);
                  }

                  /****************************************************/
                  /* This is not the listening socket, therefore an   */
                  /* existing connection must be readable             */
                  /****************************************************/
                  else
                  {
                     printf("  Descriptor %d is readable\n", i);
                     close_conn = FALSE;
                     /*************************************************/
                     /* Receive all incoming data on this socket      */
                     /* before we loop back and call select again.    */
                     /*************************************************/

                     /**********************************************/
                     /* Receive data on this connection until the  */
                     /* recv fails with EWOULDBLOCK.  If any other */
                     /* failure occurs, we will close the          */
                     /* connection.                                */
                     /**********************************************/
                     rc = recv(i, buffer, sizeof(buffer), 0);
                     if (rc < 0)
                     {
                        if (errno != EWOULDBLOCK)
                        {
                           perror("  recv() failed");
                           close_conn = TRUE;
                        }
                     }

                     else if (rc == 0)
                     {
                        printf("  Connection closed\n");
                        close_conn = TRUE;
                     }
                     else
                     {
                        /**********************************************/
                        /* Data was received                          */
                        /**********************************************/
                        buffer[rc] = '\0';
                        if (convertString(buffer, number, character) == 1)
                        {
                           //Ghép xâu
                           strcpy(result, "Result: \n");
                           strcat(result, "\tDigit: ");
                           strcat(result, number);
                           strcat(result, "\n");
                           strcat(result, "\tCharacter: ");
                           strcat(result, character);
                        }
                        else
                        {
                           strcpy(result, "Error \n");
                        }
                        len = strlen(result);
                        /**********************************************/
                        /* Echo the data back to the client           */
                        /**********************************************/
                        iov[0].iov_base = result;
                        iov[0].iov_len = len;
                        rc = writev(i, iov, 1);

                        if (rc < 0)
                        {
                           perror("  send() failed");
                           close_conn = TRUE;
                        }
                     }

                     if (close_conn)
                     {
                        close(i);
                        FD_CLR(i, &master_set);
                        if (i == max_sd)
                        {
                           while (FD_ISSET(max_sd, &master_set) == FALSE)
                              max_sd -= 1;
                        }
                     }
                  } /* End of existing connection is readable */
               }    /* End of if (FD_ISSET(i, &working_set)) */
            }       /* End of loop through selectable descriptors */

         } while (end_server == FALSE);

         /*************************************************************/
         /* Clean up all of the sockets that are open                  */
         /*************************************************************/
         for (i = 0; i <= max_sd; ++i)
         {
            if (FD_ISSET(i, &master_set))
               close(i);
         }
         return 0;
      }
   }
}