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
void *thread_read();
void *thread_write();
const int flag=0;

void closeconnection();
void request(char nome[256]);
void added(char nome[256]);
 int main(){
  //variabili
  int port, fd, result, nreads;
  struct sockaddr_in addr;
  size_t addr_len;
  pthread_t read1, write1;
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
  addr.sin_port=htons(port);
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
   while(1){
   printf("Inserire nickname:");
   scanf("%s%*c", nome);
   write(sock, (void*)nome, (size_t)sizeof(nome));
   //read(sock,(void*) nome, sizeof(nome));
   //printf("%s", nome);
   //scanf("%s%*c", nome);
   //write(sock, nome, sizeof(nome));
   read(sock, (void*)&result, (size_t)sizeof(int));
   if ((int)result==0) printf("Nome non valido\n");
   else {
   FD_ZERO(&readfds);
   FD_SET(sock, &readfds);
   FD_SET(0, &readfds);//inserisco la tastiera al set di lettura così da rilevare se l'utente ha scritto qualcosa
   fflush(stdin);
   break;
    }
   }
   
   printf("\nLista comandi:\n@new per stanza privata\n@close per chiudere\n\n");
   
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
       if(strncmp("@new", buffer, 4)==0) request(nome);
    else {
        nreads=strlen(buffer);
       //printf("%s\n", buffer);
        write(sock, (void *) buffer, (size_t)nreads+1);
        fflush(stdin);
        if(strncmp("@close", buffer, 6)==0) closeconnection();
        }
      }
    if(FD_ISSET(sock,&testfds)) {
      nreads=read(sock, (void *) buffer, (size_t)sizeof(buffer));
      if(nreads<=0){
       printf("Server non più in funzione\n");
       exit(EXIT_FAILURE);
       }
      if(strncmp("@added", buffer, 6)==0) added(nome);
      fputs(buffer, stdout);
      fflush(stdout);
      }
     
     
    
    
    
    
   } 
   
   
   close(sock);
    
    
 
 
 
 
 
 
 }
void request(char nome[256]){
 pid_t pid;
 pid=fork();
 switch(pid){
  case -1:
          perror("Richiesta fallita\n");
          break;
  case 0: 
          printf("richiesta presa a carico\n");
          
          execl("/usr/bin/xterm", "xterm","-fn","-*-*-*-*-*-*-18-*-*-*-*-*-*-*", "-e", "./client_chat1", nome ,NULL);//cambio immagine programma
          perror("Exec fallita\n");
          break;
  default:
          break;                
 }
 return;


}

void added(char nome[256]){
 pid_t pid;
 pid=fork();
 switch(pid){
  case -1:
          perror("Richiesta fallita\n");
          break;
  case 0: 
          printf("richiesta presa a carico\n");
          
          execl("/usr/bin/xterm", "xterm", "-fn","-*-*-*-*-*-*-18-*-*-*-*-*-*-*","-e","./client_chat2", nome ,NULL);//cambio immagine programma
          perror("Exec fallita\n");
          break;
  default:
          break;                
 }
 return;


}

void closeconnection(){
 char buffer[256];
 //read(sock, (void*) buffer, (size_t)sizeof(buffer));
 //printf("%s\n", buffer);
 close(sock);
 printf("Bye\n");
 exit(EXIT_SUCCESS);
}
