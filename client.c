#include<stdio.h>
#include<netdb.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/time.h>
#include<sys/types.h>

#define SEC  0
#define USEC 0
#define MAXLEN 100
#define MYPORT "8001"
#define PARTNERPORT "8000"

int main()
{
    int rv;
    int result;
    int sockfd;
    int numBytes;
    int terminate;
    fd_set readset;
    char msg[MAXLEN];
    struct timeval tv;
    struct addrinfo hints, *myinfo, *partnerinfo;

    // setting my address
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if((rv = getaddrinfo(NULL, MYPORT, &hints, &myinfo)) != 0){
        fprintf(stderr, "my: getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    if(myinfo == NULL){
        fprintf(stderr, "my: failed to bind socket\n");
        exit(2);
    }

    if((sockfd = socket(myinfo->ai_family, myinfo->ai_socktype, myinfo->ai_protocol)) == -1){
        perror("socket");
        exit(1);
    }

    if(bind(sockfd, myinfo->ai_addr, myinfo->ai_addrlen) == -1){
        perror("bind");
        exit(1);
    }

    freeaddrinfo(myinfo);

    // setting partner address
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if((rv = getaddrinfo(NULL, PARTNERPORT, &hints, &partnerinfo)) != 0){
        fprintf(stderr, "partner: getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    if(partnerinfo == NULL){
        fprintf(stderr, "failed to set partner addr info\n");
        exit(2);
    }

    while(1){
        FD_ZERO(&readset);
        FD_SET(0, &readset);
        FD_SET(sockfd, &readset);

        tv.tv_sec = SEC;
        tv.tv_usec = USEC;

        if((result = select(sockfd+1, &readset, NULL, NULL, &tv)) == -1){
            perror("select1");
            exit(1);
        }
        else if(result){
            if(FD_ISSET(sockfd, &readset)){
                if((numBytes = recvfrom(sockfd, msg, MAXLEN-1, 0, NULL, NULL)) == -1){
                    perror("recvfrom");
                    exit(1);
                }
                else if(numBytes == 0){
                    printf("\nconnection closed by partner! exiting....\n\n");
                    close(sockfd);
                    break;
                }
                msg[numBytes] = '\0';
                printf("p2>>> %s\n", msg);
            }

            terminate = 0;
            if(FD_ISSET(0, &readset)){
                if(fgets(msg, MAXLEN, stdin) == NULL)
                    terminate = 1;

                if(strcmp(msg, "\n") == 0)
                    continue;

                if(terminate)
                    msg[0] = '\0';
                else
                    msg[strlen(msg)-1] = '\0';
                if((numBytes = sendto(sockfd, msg, strlen(msg), 0, partnerinfo->ai_addr, partnerinfo->ai_addrlen)) == -1){
                    perror("sendto");
                    exit(1);
                }
                if(terminate){
                    printf("\nconnection closed! exiting....\n\n");
                    break;
                }
            }
        }
    }

    freeaddrinfo(partnerinfo);
    close(sockfd);
    exit(0);
}
