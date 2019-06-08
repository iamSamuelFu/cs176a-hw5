#include <stdio.h>      
#include <stdlib.h>     
#include <string.h>     
#include <sys/socket.h> 
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <math.h>
#include <ctype.h>

int main( int argc, char **argv )
{
  if(argc != 3){
    printf("wrong input\n");
    return 1;
  }
  int portNum = atoi(argv[2]);
  struct hostent *server = gethostbyname(argv[1]);
  if (server == NULL) {
    fprintf(stderr,"ERROR, no host as %s\n", argv[1]);
    exit(0);
  }

  int fd;
  if ( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
    perror("socket failed");
    return 1;
  }

  struct sockaddr_in serveraddr;
  memset( &serveraddr, 0, sizeof(serveraddr) );
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_port = htons( portNum );              
  bcopy((char *)server->h_addr, 
      (char *)&serveraddr.sin_addr.s_addr, server->h_length);

  if (connect(fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0){
    printf("\nConnection Failed \n");
    return 1;
  }

  char serverStateData[128];
  read(fd, serverStateData, sizeof(serverStateData));
  if(serverStateData[1] == 's'){
    int len = serverStateData[0] -'0';
    for(int i = 1; i < len + 1 ; i++){
      printf("%c", serverStateData[i]);
    }
    printf("\n");
    close(fd);
  }else if (serverStateData[1] == 'a'){
    printf("%s","Ready to start game?(y/n): ");
    char inputs[128];
    int size = 0;
    while(1){
      char chr;
      scanf("%c",&chr);
      if(chr=='\n') break;
      else{
    inputs[size++] = chr;
      }
    }
    if(inputs[0] == 'y'| inputs[0] =='Y') ;
    else exit(0);

    char startMessage[1] = {'0'};
    send( fd, startMessage, sizeof(startMessage), 0);

    while(1){
        char buffer[128];
        read(fd, buffer, sizeof(buffer));
        int isEnd = 0;
        int needInput = 1;
        if (buffer[0] == '0') {
            int len = buffer[1]-'0';
            int incorr = buffer[2] - '0';
            for(int i = 3; i<len+3; i++){
              printf("%c ", buffer[i]);
            }
            printf("\n%s", "Incorrect Guesses: ");
            for(int i = len+3; i<incorr+len+3; i++){
              printf("%c ", buffer[i]);
            }
            printf("\n");
        }else {
            int len = buffer[0] -'0';
            for(int i = 1; i < len + 1 ; i++){
              printf("%c", buffer[i]);
            }
            printf("\n");
            if(buffer[1] == 'G'){
              needInput = 0;
              isEnd = 1;
            }else if(buffer[1] == 'T'){
              needInput = 0;
            }else if(buffer[1] == 'Y'){
              needInput = 0;
            }
        }
        if(needInput){
            printf("\n%s","Letter to guess: ");
            char inputs2[128];
            int size2 = 1;
            while(1){
                char chr2;
                scanf("%c",&chr2);
                if(chr2=='\n') break;
                else{
                    inputs2[size2++] = tolower(chr2);
                }
            }
            inputs2[0] = (--size2)+'0';
            int len = buffer[1]-'0';
            int incorr = buffer[2] - '0';
            for(int i = len+3; i<incorr+len+3; i++){
              if(inputs2[1] == buffer[i]){
                printf("Error! Letter %c has been guessed before, please guess another letter.", inputs2[1]);
                break;
              }
              send(fd, inputs2, sizeof(inputs2), 0);
            }
    
        }
        if(isEnd) break;
    }    

    close( fd );
  }
}