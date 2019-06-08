#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

void *doprocessing(void *);
int numOfChild = 0;
pthread_t threads[3];

int main( int argc, char **argv )
{
  int portNum=0;
  if(argc != 2){
    printf("Enter port number in command line.\n");
    return 1;
  }
  portNum = atoi(argv[1]);

  int fd;
  if ( (fd = socket(AF_INET, SOCK_STREAM, 0)) == 0 ) {
    perror( "socket failed" );
    exit(1);
  }

  int opt = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
    perror("setsockopt");
    exit(1);
  }

  struct sockaddr_in serveraddr;
  memset( &serveraddr, 0, sizeof(serveraddr) );
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_port = htons( portNum );
  serveraddr.sin_addr.s_addr = htonl( INADDR_ANY );

  if ( bind(fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0 ) {
    perror( "bind failed" );
    exit(1);
  }

  if(listen(fd,3)<0){
    perror("listen");
    exit(1);
  }

  struct sockaddr_in clientaddr;
  socklen_t clientlen = sizeof(clientaddr);
  int new_socket;
  
  while(1){
    if ((new_socket = accept(fd, (struct sockaddr *)&clientaddr, &clientlen))<0){
      perror("accept failed");
      exit(1);
    }
    
    // printf("%d\n",numOfChild);

    if(numOfChild < 3){
      if (0 != pthread_create(&threads[numOfChild], NULL, doprocessing , (void*) &new_socket)){
	perror("thread creation failed\n");
      }
      char output2[128] = "6accept";
      send(new_socket, output2, sizeof(output2),0);
    }else{
      char output2[128];
      int number17 = 17;
      output2[0] = number17+'0';
      char message[17] = "server-overloaded";
      for( int i = 1; i < 18; i++){
	output2[i] = message[i-1];
      }
      send(new_socket, output2, sizeof(output2),0);
    }
  }
}

void *doprocessing(void *socket_desc){
  numOfChild++;
  int new_socket = *(int*)socket_desc;
  char buffer[128];
  
  // char* words[15] = {"cat", "dog", "hangman", "book", "potato", "tomato", "ham", "food", "chicken", "beef", "pork", "cake", "water", "melon", "drink"};
  
  // char* words[15];
  // FILE *fp;    
  // fp = fopen("hangman_words.txt", "r");

  // int n_lines=0;
  // char in[100];
  // int size=sizeof(in);
  // while(fgets(in, size, fp)!=NULL){ 
  //     if(n_lines == 15){
  //         break;
  //     }       
  //     words[n_lines] = in;                
  //     n_lines++;
  // }
  char words[15][8];
  char buf[8];
  FILE *fp = fopen("hangman_words.txt","r");
  for(int i = 0; i< 14; i++){
    fscanf(fp, "%s", buf);
    words[i] = buf;

  }


  for(int x = 0; x<15; x++){
    printf("%s\n",words[x] );
  }

  srand(time(NULL));
  char word[8];
  int randomNum = rand() % 15;
  //printf("random is %d", randomNum);
  for(int i=0;i<strlen(words[randomNum]);i++){
    word[i] = words[randomNum][i];
    //printf("%c", word[i]);
  }
  int wordlength = strlen(words[randomNum]);
  int errNum = 0;
    
  int length = read( new_socket, buffer, sizeof(buffer) );

  char output[128];
  char output2[128];
  if(buffer[0] == '0'){
    output[0] = '0';
    output[1] = wordlength + '0';
    output[2] = '0';
    for (int i = 3; i< 3 + wordlength; i++){
      output[i] = '_';
    }
    send(new_socket, output, sizeof(output),0);
  }

  while(1){
    int length = read( new_socket, buffer, sizeof(buffer) );
    if(buffer[0] != (1 + '0') || !isalpha(buffer[1])) {
      int number30 = 30;
      output2[0] = number30+'0';
      char message[30] = "Error!Please guess one letter.";
      for( int i = 1; i < 31; i++){
	output2[i] = message[i-1];
      }
      send(new_socket, output2, sizeof(output2),0);
    }else{
      char guessChar = buffer[1];
      int isErr = 1;
      for( int j = 3; j < 3 + wordlength; j++){
	if(word[j-3] == guessChar){
	  output[j] = guessChar;
	  isErr = 0;
	}
      }
	
      if(isErr){
	output[3+wordlength+errNum] = guessChar;
	errNum++;
	output[2] = errNum+'0';
      }

      int isWin = 1;
      for(int i =3; i<wordlength+3; i++){
	if(output[i] == '_') isWin = 0;	
      }

      if(isWin || errNum >= 6){
	int number = 13 + wordlength*2;
	output2[0] = number + '0';
	char message4[13] = "The word was ";
	for( int i = 1; i < 14; i++){
	  output2[i] = message4[i-1];
	}
	for( int i = 14; i < (14 + wordlength*2) ; i++){
	  output2[i] = word[(i-14)/2];
	  output2[i+1] =' ';
	  i++;
	}
	send(new_socket, output2, sizeof(output2) ,0);

	if(errNum >=6){
	  int number10 = 10;
	  output2[0] = number10 + '0';
	  char message5[10] = "You Lose.";
	  for(int i = 1; i<11;i++){
	    output2[i] = message5[i-1];
	  }
	}
	  
	if(isWin){
	  char message2[9]  = "8You Win!";
	  for(int i = 0; i<9;i++){
	    output2[i] = message2[i];
	  }
	}
	send(new_socket, output2, sizeof(output2) ,0);
	int number10 = 10;
	output2[0] = number10 + '0';
	char message3[10] = "Game Over!";
	for( int i = 1; i < 11; i++){
	  output2[i] = message3[i-1];
	}
	send(new_socket, output2, sizeof(output2) ,0);
	break;
      }else{
	send(new_socket, output, sizeof(output),0);
      }
    }
  }
  numOfChild--;
}
