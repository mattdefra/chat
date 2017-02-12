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
int sock;
const int flag=2;

void closeconnection();
 int main(int argc, char *argv[]){
  //variabili
  int port, fd, result, nreads;
  struct sockaddr_in addr;
  size_t addr_len;
  pthread_t read1, write1;
  struct hostent *hp; 
  char nome[256], buffer[256], name[256];
  fd_set readfds, testfds;
  
  strcpy(name, argv[1]);
  
  //printf("%s\n", name);
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
  addr.sin_port=htons(port);
  hp=gethostbyname(nome);
  if (hp==NULL){
   perror("host sconosciuto\n");
   exit(EXIT_FAILURE);
  }
  addr.sin_addr=*(struct in_addr*)*hp->h_addr_list;
  addr_len=(socklen_t)sizeof(addr);
  //printf("valori assegnati\n");
  //printf("provo a connettermi...\n");
  if (connect(sock, (struct sockaddr*)&addr,sizeof(addr))==-1){
   perror("Errore connect\n");
   exit(EXIT_FAILURE);
   }
   //printf("Connessione stabilita...\n");
   write(sock,(void*) &flag,(size_t)sizeof(int));//invio flag
   nreads=strlen(name);
   //printf("%d\n", nreads);
   printf("%s\n", name);
   write(sock, (void*) name, (size_t)nreads+1);//invionome (ottenuto da client_chat)
   //printf("Inserire nome stanza privata:");
   //scanf("%s%*c", nome);
   //write(sock,(void*)nome,(size_t)sizeof(nome));
   FD_ZERO(&readfds);
   FD_SET(sock, &readfds);
   FD_SET(0, &readfds);
   printf("Lista comandi chatroom privata:\n@add <nome> per aggiungere utenti alla chat\n@close per chiudere la sessione corrente\n");
   while(1){
    testfds=readfds;
    result=select(sock+1, &testfds, (fd_set*)0, (fd_set*)0, (struct timeval *) 0);
    if(result<1){
     perror("client not working...\n");
     exit(EXIT_FAILURE);
     }
    if(FD_ISSET(0,&testfds )){
       //nreads=read(0, (void *) buffer, (size_t)sizeof(buffer));
       fgets(buffer, 256 ,stdin);
       nreads=strlen(buffer);
       //printf("%s\n", buffer);
       write(sock, (void *) buffer, (size_t)nreads+1);
       fflush(stdin);
       if(strncmp("@close", buffer, 6)==0) closeconnection();
        
        
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
void closeconnection(){
 char buffer[256];
 //read(sock, (void*) buffer, (size_t)sizeof(buffer));
 //printf("%s\n", buffer);
 close(sock);
 printf("Bye\n");
 exit(EXIT_SUCCESS);
}
