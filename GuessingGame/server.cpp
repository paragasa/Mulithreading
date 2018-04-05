//
//  server.cpp
//  GuessingGame
//
//  Created by Alan Paul Paragas on 5/17/17.
//  Copyright © 2017 Alan Paul Paragas. All rights reserved.
//
#include <cstdlib>
#include <iostream>
#include <vector>
#include <pthread.h>
#include <unistd.h>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <cstring>
#include <cstdlib>

using namespace std;

//global variables
const int PORTbegin=11600;
const int PORTend=11699;
const int MAXPENDING=10;
const int MAXDISPLAY=4;
pthread_mutex_t LOCK;

//struct declarations
struct leaderboard{
    string username[MAXDISPLAY];
    long userguess[MAXDISPLAY];
};
struct userGuess {
    int socket;
};

//functions
void sendNUM(int sock, long  guess);
long recieveNUM(int sock, bool &socketcheck);
void sendString(int sock, string word);
string recieveString(int sock,bool &socketcheck);
long closeness(long guess, long random);
void printleaderboard();
void topscorecheck(long turn, string name);
void sendBoard(int sort);
leaderboard Display;

//main function
void* fu(void *info ){
    int *cssock= (int*)info;
    int csock=*cssock;
    bool won=false;
    bool finish=false;
    bool deadsocket=false;
    long randomNum=(rand()% 9999+0);
    long recvGuess;
    long getLength;
    string getNAME=recieveString(csock,deadsocket);
    if(deadsocket==false){
    cout<<getNAME<<" has connected"<<endl;
    cout<<"Random Guess for "<<getNAME<<" is "<<randomNum<<endl;
    long countturn=1;
    sendNUM(csock,randomNum); //send back result of random TEST
    while(!finish && deadsocket==false){
        if(deadsocket==false){
        recvGuess= recieveNUM(csock,deadsocket);//error checking
        }
        if(deadsocket==true) { //will shut down thread if return is true
            finish=true;
            cout<<getNAME<<" has left the game!"<<endl;
            break;
        }
        else{
            if(recvGuess<0|| recvGuess>9999){
                cout<<" User has guessed out of bounds"<<endl;
                continue;
            }
            cout<<getNAME<<" has guessed "<<recvGuess<<endl;
            countturn++;
        
        //checks closeness
            long closenum= closeness(recvGuess, randomNum);
            sendNUM(csock,closenum);
            cout<<"Closeness: "<<closenum<<endl;
            if(recvGuess==randomNum){
                countturn=countturn-1;
                finish=true;
            }
        }
    }
    if(finish && deadsocket==false){
        string victory="Congratulations! It took ";
        string victory2=" turns to guess the number!\n";
        sendString(csock, victory);
        sendString(csock,victory2);
        sendNUM(csock, countturn);
        

        cout<<getNAME<<" has guessed right "<<endl;
        //lock
        //make this a function
        pthread_mutex_lock(&LOCK);
        topscorecheck(countturn, getNAME);
        pthread_mutex_unlock(&LOCK);
    
    }
    }
    if(deadsocket==false){ //prints/send in the case where socket is not dead
    printleaderboard();
    sendBoard(csock);
    }
    pthread_detach(pthread_self());
    close(csock);
    delete cssock;
    pthread_exit(NULL);
    
}


int main(int argc, char* argv[]){
    int sock;
    int status;
    //create default leaderboard
    for(int i=0; i<4;i++) {
        Display.username[i]="Empty Spot";
        Display.userguess[i]=0;
    }
    //arg check
    if(argc < 3){
        printf("Need to pass correct number of arguments: 1)IP 2)PORT \n");
        exit(-1);
    }
    //SIGN PORT
    unsigned  short portnum= (unsigned short)atoi(argv[2]);
    if(portnum <PORTbegin || portnum > PORTend){
        printf("Invalid port entry, must be between port number 11600-11699\n");
        exit(-1);
    }
    //initiate lock
    pthread_mutex_init(&LOCK, NULL);
    //socket creation
    sock= socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock <0){
        printf( "Error with Socket\n");
        close(sock);
        exit (-1);
    }
    
    //for IP address
    struct sockaddr_in servAddr;
    servAddr.sin_family=AF_INET;
    servAddr.sin_addr.s_addr= htonl(INADDR_ANY);
    servAddr.sin_port= htons(portnum);
    
    //bind sock address
    status= ::bind(sock,(struct sockaddr *)&servAddr,sizeof(servAddr));
    if (status < 0){
        printf("Error:Cannot Bind to Sock\n");
       close(sock);
        exit(-1);
    }
    
    //check listener
    status= listen(sock,MAXPENDING);
    if (status < 0){
        printf("Error:Listen Error\n");
        close(sock);
    exit(-1);
    }
    
     //accept a client
    printf("Accepting new client \n");
    int csock;
    while(true){
        pthread_t threadcon;
        struct sockaddr_in clientAddr;
        socklen_t addrLen=sizeof(clientAddr);
        csock=accept(sock,(struct sockaddr*)&clientAddr,&addrLen);
        userGuess *userGuest= new userGuess;
        if(csock<0){
            printf("Error: Thread Not Created");
            close(csock);
            exit(-1);
        }
        userGuest->socket=csock;
        printf("waiting for a connection\n");
        status= pthread_create(&threadcon,NULL,&fu,(void*)userGuest);
        if(status){
            printf("Error: Thread Not Created");
            close(csock);
            exit(-1);
        }
    cout<<"THREAD INITALIZED"<<endl;
    }
    close(sock);
}

void sendNUM(int sock, long  number){
    long numsend= htonl(number);
    long bytecount=send(sock,(void*) &numsend, sizeof(numsend),0);
    if(bytecount!=sizeof(numsend)){
        printf("Error: Sending data \n");
        close(sock);
        exit(-1);
    }
}
long recieveNUM(int sock,bool &socketcheck){
    long byteleft=sizeof(long);
    long num;
    char *bp=(char*)&num;
    while(byteleft>0){
        long byterecv=recv(sock,(void*)bp,byteleft,0);
        if(byterecv<=0){
            printf("Error: Recieving data \n");
            socketcheck=true;
            return 0;
        }
        byteleft=byteleft-byterecv;
        bp=bp + byterecv;
    }
    num=ntohl(num); //convert number to host byte
    return num;
}
void sendString(int sock, string msg){
    long msglength =msg.length(); //get length of message
    char msgarr[msglength];
    strcpy(msgarr,msg.c_str());
    sendNUM(sock,msglength); //allows to set recieve length
    long bytesent=send(sock,(void *)msgarr,sizeof(msgarr),0);
    if (bytesent!=msglength){
        close(sock);
        exit(-1);
    }
}
string  recieveString(int sock, bool &socketcheck){
    string msgrec; //sends this string
    long byteleft=recieveNUM(sock,socketcheck);
    char msgarr[byteleft];
    char *bp=msgarr;//check this
    
    while(byteleft>0){
        
        long byteRecieve= recv(sock,(void *)bp,sizeof(msgarr),0);
        
        if(byteRecieve<=0){
            printf("Error: Recieving data \n");
            socketcheck=true;
            return "Error:Disregard Recieve\n";
        }
        byteleft=byteleft-byteRecieve;
        bp=bp+byteRecieve;
    }
    msgrec=string(msgarr);
    return msgrec;
    
}
void printleaderboard(){
    cout<<" SERVER CURRENT LEADERBOARD \n____________________________"<<endl;
    for(int z=0;z<3;z++){
        cout<<z+1<<".  "<<Display.username[z]<< " with "<< Display.userguess[z]<<endl;
    }
}
long closeness(long guess, long random){
    long closenum=0;
    long guessadd=guess, randomadd=random;
   
    for(int i=0; i<4; i++){
        long comparenums=guessadd % 10;
        long comparenums2=randomadd % 10;
        if( guessadd>=10){
            guessadd= (guessadd/10);
        }
        else{
            guessadd=0;
        }
        if(randomadd>=10){
            randomadd= (randomadd/10);
        }
        else{
            randomadd=0;
        }
        comparenums2=comparenums2-comparenums;
        if(comparenums2<0){
            comparenums2=comparenums2 * (-1);
        }
        closenum=closenum+comparenums2;
    }
    return closenum;
}
void topscorecheck(long turn, string name){
    int i=0;
    long countturn=turn;
    while(i!=3){
        long boardnum=Display.userguess[i];
        if(countturn<boardnum){
            for(int j=2;j>0;j--){
                Display.userguess[j]=Display.userguess[j-1];
                Display.username[j]=Display.username[j-1];
            }
            Display.userguess[i]=turn;
            Display.username[i]=name;
            i=3;
            break;
        }
        if(boardnum==0){
            Display.userguess[i]=turn;
            Display.username[i]=name;
            i=3;
            break;
        }
        i++;
    }

}

void sendBoard(int sock){
    for (int i=0; i<MAXDISPLAY;i++){
        string namesend=Display.username[i];
        long guesssend=Display.userguess[i];
        sendString(sock,namesend);
        sendNUM(sock, guesssend);
    }

}

