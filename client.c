/*///////////////////////////////////////////////////////////
*
* FILE:     client.c
* AUTHOR:   Fernando Mello
* PROJECT:  CS 3251 Project 2 - Professor Jun Xu 
* DESCRIPTION:  Hangman Player
*
*////////////////////////////////////////////////////////////

/* Included libraries */
#include <stdio.h>          /* printf() and fprintf() */
#include <sys/socket.h>     /* socket(), connect(), send(), and recv() */
#include <arpa/inet.h>      /* sockaddr_in and inet_addr() */
#include <stdlib.h>         /* all sorts of functionality */
#include <unistd.h>         /* close() */
#include <string.h>         /* any string ops */
#include <ctype.h>          /* tolower() */

#define RCVBUFSIZE 22   /* Receive buffer size */
#define SNDBUFSIZE 2    /* Send buffer size */

/* The main function */
int main(int argc, char* argv[]) {
    int clientSock;                 /* Socket descriptor */
    struct sockaddr_in servAddr;    /* Server address structure */
    
    char* servIP;                   /* Server IP address */
    unsigned short servPort;        /* Server port number */
    
    char sndBuf[SNDBUFSIZE];        /* Send buffer */
    char rcvBuf[RCVBUFSIZE];        /* Receive buffer */

    char yn;                        /* Player ready input */
    int guess;
    char guessbuf[256];                /* Player guess input */
    int guessValid;                 /* Validity check of player guess */

    int wordLen;                    /* Length of game word */
    int numIncorrect;               /* Number of incorrect guesses */

    int gameOver = 0;               /* Game state */
    char* result = "";

    int i;                          /* Loop counter */

    if (argc != 3) {
        printf("Incorrect number of arguments. The correct format is:\n\t"
               "./client serverIP serverPort\n");
        exit(1);
    }

    /* Get arguments from the command line */
    servIP = argv[1];
    servPort = atoi(argv[2]);

    /* Reserve space for buffers */
    memset(&rcvBuf, 0, RCVBUFSIZE);
    memset(&sndBuf, 0, SNDBUFSIZE);

    /* Create a new TCP socket */
    if ((clientSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        printf("socket() failed\n"); 
        exit(1);
    }

    /* Construct the server address structure */
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(servIP);
    servAddr.sin_port = htons(servPort);

    /* Establish connecction to the server */
    if (connect(clientSock, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0) {
        printf("connect() failed\n");
        exit(1);
    }

    /* Check if player is ready */
    do {
        printf("Ready to start game? (y/n): ");
        yn = getchar();
        getchar();
    } while (yn != 'y');

    printf("\n");

    memset(sndBuf, 0x00, 1);


    /* Send game start packet */
    if (send(clientSock, sndBuf, SNDBUFSIZE, 0) != SNDBUFSIZE) {
        printf("send() failed\n");
        exit(1);
    }

    while (1) {
        /* Receive incoming packet */
        if (recv(clientSock, rcvBuf, RCVBUFSIZE, 0) < 0) {
            printf("recv() failed\n");
            exit(1);
        }

        if (rcvBuf[0] == 0x00) {    /* Game control packet */
            wordLen = rcvBuf[1];
            numIncorrect = rcvBuf[2];

            gameOver = 1;

            for (i = 3; i < wordLen + 3; i++) {
                if (rcvBuf[i] == '_') {
                    gameOver = 0;
                    break;
                }
            }

            if (numIncorrect == 6) {
                gameOver = 1;
            }

            if (!gameOver) {

                for (i = 3; i < wordLen + numIncorrect + 3; i++) {
                    if (i < wordLen + 2) {
                        printf("%c ", rcvBuf[i]);
                    } else if (i == wordLen + 2) {
                        printf("%c", rcvBuf[i]);
                        printf("\nIncorrect Guesses: ");
                    } else if (i < wordLen + numIncorrect + 2) {
                        printf("%c, ", rcvBuf[i]);
                    } else {
                        printf("%c", rcvBuf[i]);
                    }
                    if(i == wordLen + numIncorrect + 2){
                        printf("\n");
                    }
                }
                /* Get player's guess */
                // printf("\nLetter to guess: ");
                // bzero(guessbuf, 256);
                // fgets(guessbuf, 255, stdin);
                // guess = tolower(guessbuf[0]);

                /* Check guess validity */
                guessValid = 0;

                while (!guessValid) {
                    printf("\nLetter to guess: ");
                    bzero(guessbuf, 256);
                    fgets(guessbuf, 255, stdin);
                    guess = tolower(guessbuf[0]);

                    while(strlen(guessbuf) != 2 || guess < 'a' || guess > 'z') {
                        printf("\nError! Please guess one letter.\nLetter to guess: ");
                        bzero(guessbuf, 256);
                        fgets(guessbuf, 255, stdin);
                        guess = tolower(guessbuf[0]);
                    }

                    if (guess >= 'a' && guess <= 'z') {
                        guessValid = 1;
                        for (i = wordLen + 3; i < numIncorrect + wordLen + 3; i++) {
                            if (guess == rcvBuf[i]) {
                                guessValid = 0;
                                printf("\nError! Letter %c has been guessed before, please guess "
                                       "another letter.\nLetter to guess: ", guess);
                            }
                        }
                    }
                }

                printf("\n");

                /* Send player guess */
                memset(sndBuf, 0x01, 1);
                memset(sndBuf + 1, guess, 1);
            }

            if (send(clientSock, sndBuf, SNDBUFSIZE, 0) != SNDBUFSIZE) {
                printf("send() failed\n");
                exit(1);
            }
        } else {    /* Message packet */
            /* Print out message */
            if (rcvBuf[0] == (21 & 0xFF) || rcvBuf[0] == (19 & 0xFF)) {
                printf("The word was ");
                for (i = 0; i < strlen(result); i++) {
                    printf(rcvBuf[i]+" ");
                }
                printf("\n");
                printf("\t%.*s\n\t%.*s", rcvBuf[0] - 10, rcvBuf + 1,
                                                    10, rcvBuf + rcvBuf[0] - 9); /* Win or loss */
            } else {
                printf("\t%.*s", rcvBuf[0], rcvBuf + 1); /* Overload or wrong mode */
            }

            close(clientSock);
            break;
        }        
    }

    return 0;
}