#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>

int main(){
 int sock, flag=3;
 int port, fd, result, nreads;
 struct sockaddr_in addr;
 size_t addr_len;
 struct hostent *hp; 
 char nome[256], buffer[256];
 fd_set readfds, testfds;
  
  printf("Inserire nome:");
  scanf("%s%*c", nome);
  printf("porta:");
  scanf("%d%*c", &port);
 //strcpy(nome, "localhost");
  //creazione socket
 sock=socket(AF_INET, SOCK_STREAM, 0);
 if (sock==-1){
  perror("Errore creazione socket\n");
  exit(EXIT_FAILURE); 
  }
 printf("socket creato\n");
 addr.sin_family=AF_INET;
 addr.sin_port=htons(5555);
 hp=gethostbyname(nome);
 if (hp==NULL){
  perror("host sconosciuto\n");
  exit(EXIT_FAILURE);
  }
 addr.sin_addr=*(struct in_addr*)*hp->h_addr_list;
 addr_len=(socklen_t)sizeof(addr);
 printf("valori assegnati\n");
 printf("provo a connettermi...\n");
 if (connect(sock, (struct sockaddr*)&addr,sizeof(addr))==-1){
  perror("Errore connect\n");
  exit(EXIT_FAILURE);
   }
  printf("Connessione stabilita...\n");
  write(sock,(void*) &flag,(size_t)sizeof(int));
  //read(sock,(void*)buffer,(size_t)sizeof(buffer));
  //printf("%s", buffer);
  
   FD_ZERO(&readfds);
   FD_SET(sock, &readfds);
   FD_SET(0, &readfds);
   
   while(1){
    testfds=readfds;
    result=select(sock+1, &testfds, (fd_set*)0, (fd_set*)0, (struct timeval *) 0);
    if(result<1){
     perror("server not working...\n");
     exit(EXIT_FAILURE);
     }
    if(FD_ISSET(0,&testfds )){
       //nreads=read(0, (void *) buffer, (size_t)sizeof(buffer));
       fgets(buffer, 256 ,stdin);
      
       nreads=strlen(buffer);
       //printf("%s\n", buffer);
       write(sock, (void *) buffer, (size_t)nreads+1);
       fflush(stdin);
        
      }
    if(FD_ISSET(sock,&testfds)) {
      nreads=read(sock, (void *) buffer, (size_t)sizeof(buffer));
      if(nreads<=0){
       printf("Server non più in funzione\n");
       exit(EXIT_FAILURE);
       }
       fputs(buffer, stdout);
       fflush(stdout);
      }
     
     
    
    
    
    
   } 
  



}
