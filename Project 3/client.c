#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
  {
    printf("Error in socket creation");
    return 1;
  }

  char *ip;
  short port;

  if (argc == 3)
  {
    ip = argv[1];
    port = atoi(argv[2]);
  }
  else
  {
    printf("usage: ./client  <ip> <port>\n");
    return 1;
  }

  struct sockaddr_in serveraddr;
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_port = htons(port);
  serveraddr.sin_addr.s_addr = inet_addr(ip);

  int n = connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
  if (n < 0)
  {
    printf("There was a problem connecting\n");
    return 1;
  }
  while (1)
  {
    printf("Enter a username: ");
    char username[5000];
    fgets(username, 5000, stdin);
    username[strcspn(username, "\n")] = 0;
    if (strlen(username) > 16)
    {
      printf("username is too big try a shorter one\n");
    }
    else
    {
      send(sockfd, username, 17, 0);
      break;
    }
  }
  fd_set sockets;
  FD_ZERO(&sockets);
  FD_SET(sockfd, &sockets);
  FD_SET(fileno(stdin), &sockets);

  // listen(sockfd, 10);
  while (1)
  {
    char command[5000];
    char temp[5000];
    char isAdmin[2];

    fd_set tmpset = sockets;

    select(FD_SETSIZE, &tmpset, NULL, NULL, NULL);
    if (FD_ISSET(fileno(stdin), &tmpset)) // got input from user
    {
      fgets(command, 5000, stdin);
      // printf("fgets is: %d\n", prnt);
      command[strcspn(command, "\n")] = 0;
      if (!strcmp(command, "/help"))
      {
        printf("************************************************\n");
        printf("Commands:\n");
        printf("Get list of other users: /list\n");
        printf("Send message to user: /msg <username> <message>\n");
        printf("Send message to all users: /all <message>\n");
        printf("Check for new messages received: /r");
        printf("Disconnect from server: /quit\n");
        printf("\n");
        printf("Admin only:\n");
        printf("Become an admin: /admin\n");
        printf("Kick off user: /kick <username>\n");
        printf("Rename user: /rename <username> <new username>\n");
        printf("************************************************\n");
      }
      else if (!strcmp(command, "/quit"))
      {
        send(sockfd, command, strlen(command) + 1, 0);
        printf("Disconnected.\n");
        close(sockfd);
        return 0;
      }
      else if (command[0] == '/')
      {
        send(sockfd, command, strlen(command) + 1, 0);
      }
      else
      {
        printf("Invalid command, try /help to get a list of commands.\n");
      }
    }
    if (FD_ISSET(sockfd, &tmpset)) // got data from server
    {
      recv(sockfd, temp, 5000, 0);
      temp[strcspn(temp, "\n")] = 0;

      if (!strcmp(temp, "/quit"))
      {
        send(sockfd, temp, strlen(temp) + 1, 0);
        printf("Disconnected.\n");
        close(sockfd);
        return 0;
      }
      else if (!strncmp(temp, "Users", 5)) // list command
      {
        printf("%s\n", temp);
      }
      else if (!strncmp(temp, "Got a message from ", 19)) // list command
      {
        printf("%s\n", temp);
      }
      else if (!strncmp(temp, "admin: ", 7)) // recv(sockfd, isAdmin, 3, 0);
      {
        isAdmin[0] = temp[7];
        isAdmin[1] = temp[8];
        if (!strncmp(isAdmin, "ye", 2)) // user is admin
        {
          printf("You are an admin.\n");
        }
        else if (!strncmp(isAdmin, "no", 2)) // user is not admin
        {
          printf("Enter password: ");
          struct termios term;
          tcgetattr(fileno(stdin), &term);

          term.c_lflag &= ~ECHO;
          tcsetattr(fileno(stdin), 0, &term);
          char password[5000];
          fgets(password, 5000, stdin);

          term.c_lflag |= ECHO;
          tcsetattr(fileno(stdin), 0, &term);
          password[strcspn(password, "\n")] = 0;
          if (!strcmp(password, "1234567890"))
          {
            printf("password correct\n");
            char tempcommand[] = "/makeadmin";
            send(sockfd, tempcommand, strlen(tempcommand) + 1, 0);
          }
          else
          {
            printf("password incorrect\n");
          }
        }
      }
      else
      {
        printf("%s\n", temp);
      }
    }
  }
  close(sockfd);
  return 0;
}
