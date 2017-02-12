#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

#define DIM 200

struct info {
char name[DIM];
fd_set* richiesta;//per gestire l'ingresso alle stanze private
int libero;
int stanze_aperte;//indice dell'ultima richiesta presente
};

struct mod_msg{
 char text[256];
 long int scritture;
 long int letture;
};


int server_sock, client_sock, mod_sock, mie_letture;
int stanze_presenti;
struct mod_msg avviso;
struct sockaddr_in server_address, client_address;
size_t addr_len;
//char  names[DIM][FD_SETSIZE];
struct info utenti[FD_SETSIZE];
fd_set readfds, testfds, writefds, utentifds;//liberifds è il set di utenti non impegnati in nessuna chat, e che possono quindi essere aggiunti ad una chat privata
struct timeval timeout;
pthread_mutex_t mreg, mod_avviso;

void eliminaclient(int fd, fd_set* readfds, fd_set* writefds, int flag);
void addmember(int fd,char name[256], fd_set* addset);
void inviamsg(char *msg, fd_set fds);
void *thread_listen();
void *thread_reg(void*arg);
void *private_handler(void*arg);
 int main(){
  int running=1, fd, fd1, connessi;
  //struct sockaddr_in server_address, client_address;
  //size_t addr_len;
  char msg[DIM], newmsg[DIM];
  int result, nreads;
  pthread_t listener;
 
  mod_sock=-1;
  stanze_presenti=1;//quella pubblica
  avviso.scritture=0;
  avviso.letture=0;
  mie_letture=0;
  
  timeout.tv_sec=2;
  timeout.tv_usec=0;
  
  pthread_mutex_init(&mreg,NULL);
  pthread_mutex_init(&mod_avviso,NULL);
  
  //esecuzione
  //creazione socket
  server_sock=socket(AF_INET, SOCK_STREAM, 0);
  if (server_sock==-1){
      perror("Errore creazione socket\n");
      exit(EXIT_FAILURE);
      }
   server_address.sin_family=AF_INET;
   server_address.sin_port=htons(5555);
   server_address.sin_addr.s_addr=htonl(INADDR_ANY);
   addr_len=(socklen_t)sizeof(server_address);
   if (bind(server_sock, (struct sockaddr*)&server_address, addr_len)==-1){
      perror("Errore bind\n");
      exit(EXIT_FAILURE);
      }  
   getsockname(server_sock,(struct sockaddr*)&server_address,(socklen_t*)&addr_len);
   printf("Porta: %d\n", ntohs(server_address.sin_port));   
   if (listen(server_sock, 5)==-1){
      perror("Errore listen\n");
      exit(EXIT_FAILURE);
      }
   printf("In attesa di connessione...\n");
   
   FD_ZERO(&utentifds); 
   FD_ZERO(&readfds);
   //FD_SET(server_sock, &readfds);
   pthread_create(&listener, NULL, thread_listen, NULL);
   if (listener==-1){
   perror("errore creazione listen\n");
   exit(EXIT_FAILURE);
   }
   while(1){
    testfds=readfds;
    writefds=readfds;
    
    if(mie_letture<avviso.scritture){
     inviamsg(avviso.text, writefds);
     pthread_mutex_lock(&mod_avviso);
     avviso.letture++;
     mie_letture++;
     pthread_mutex_unlock(&mod_avviso);
     }
    
    result=select(FD_SETSIZE, &testfds, (fd_set*)0, (fd_set*)0, (struct timeval *)&timeout);
     if(result==-1){
      perror("server not working...\n");
      exit(EXIT_FAILURE);
      }
    if(result==0) continue;  
     for(fd=0; fd< FD_SETSIZE; fd++){
      if(FD_ISSET(fd,&testfds)) {
      
        nreads=read(fd,(void*) msg,(size_t) sizeof(msg));
        if (nreads<=0){
         eliminaclient(fd, &readfds, &writefds,0);
         continue;
         }
        printf("serving client [%s]\n", utenti[fd].name);
        
        if(strncmp("@close", msg, 6)==0){ 
        printf("Ricevuta richiesta di chiusura da %s\n", utenti[fd].name);
        eliminaclient(fd, &readfds, &writefds, 0);
        }
        else {
        sprintf(newmsg, "[%s]", utenti[fd].name);
        nreads=nreads+strlen(newmsg);
        strcat(newmsg, msg);
        printf("%s ::ricevuto\n", newmsg); 
        
        for (fd1=4; fd1<FD_SETSIZE; fd1++){
        if(FD_ISSET(fd1, &writefds) && fd1!=fd){
         printf("invio a %s\n", utenti[fd1].name); 
         write(fd1,(void*) newmsg,(size_t) nreads+1);
           }
           }
          }
         
        } 
      
      
       }
     
      }
     
     }
int cercautente(char name[256], fd_set fds){
 for(int fd=0; fd<FD_SETSIZE; fd++){
  if(FD_ISSET(fd, &fds)){
   if(strcmp(utenti[fd].name,name)==0) return fd;
   }
  } 
  return -1;


}     
void inviamsg(char *msg, fd_set fds){
 
 for (int fd1=4; fd1<FD_SETSIZE; fd1++){
  if(FD_ISSET(fd1, &fds) ){
    printf("invio a %s\n", utenti[fd1].name); 
    write(fd1,(void*) msg,(size_t)strlen(msg) +1);
  }
 }
 
}     
void addmember(int fd, char name[256], fd_set* addset){
 int fd1;
 char msg[256];
 
 printf("Provo ad aggiungere\n");
 
 for(fd1=0; fd1<FD_SETSIZE; fd1++){
  if (FD_ISSET(fd1, &utentifds)){
   if(strcmp(name, utenti[fd1].name)==0) break;
  
   }
  }
  if (fd1<FD_SETSIZE) {
    while(!utenti[fd1].libero) sleep(1);
    printf("fd %d\n", fd1);
    utenti[fd1].richiesta=addset;
    utenti[fd1].libero=0;
    strcpy(msg, "Aggiunto\n"); 
    write(fd, (void*)msg, (size_t)strlen(msg)+1);
    strcpy(msg, "@added");
    write(fd1, (void*)msg, (size_t)strlen(msg)+1);
  }
  else {
   strcpy(msg, "Nessun utente registrato con quel nome\n");
   write(fd, (void*)msg, (size_t)strlen(msg)+1);
   
  }
  return;
}
void *mod_handler(){
 char buffer[256];
 int nreads;
 int running;
 
 while(1){
  nreads=read(mod_sock, (void*)buffer, (size_t)sizeof(buffer));
  if(nreads<=0) {
  close(mod_sock); //aggiorno valore eventualmente...
  mod_sock=-1;//significa nessun moderatore presente
  printf("Moderatore disconnesso\n");
  pthread_exit(NULL);
   }
  running=1;
  pthread_mutex_lock(&mod_avviso);
  sprintf(avviso.text, "[Moderatore] %s", buffer);
  avviso.letture=0;
  avviso.scritture++;
  pthread_mutex_unlock(&mod_avviso);
  sleep(2);
  while(running){
   pthread_mutex_lock(&mod_avviso);
   if(avviso.letture<stanze_presenti){
   pthread_mutex_unlock(&mod_avviso);
   strcpy(buffer,"Invio in corso...\n");
   write(mod_sock, (void*)buffer,(size_t)strlen(buffer)+1);
   sleep(2);
    }
  else {
   pthread_mutex_unlock(&mod_avviso);
   strcpy(buffer,"Messaggio recapitato a tutti gli utenti\n");
   write(mod_sock,(void*)buffer,(size_t)strlen(buffer)+1);
   running=0;
   } 
  
  }  

 }  
}   
void *private_handler(void*arg){
 int fd=*((int*)arg), nreads, fd1, result, mie_letture1;
 char msg[256], newmsg[256];
 fd_set myfds, testfds;
 pthread_t add;
 
 stanze_presenti++;
 mie_letture1=mie_letture;
 
 FD_ZERO(&myfds);
 FD_SET(fd, &myfds);
 strcpy(msg, "Stanza creata con successo\n");
 write(fd, (void*)msg, (size_t) sizeof(msg));
 while(1){
    testfds=myfds;
    
    if(mie_letture1<avviso.scritture){
     inviamsg(avviso.text, myfds);
     pthread_mutex_lock(&mod_avviso);
     avviso.letture++;
     mie_letture1++;
     pthread_mutex_unlock(&mod_avviso);
     }
    
    result=select(FD_SETSIZE, &testfds, (fd_set*)0, (fd_set*)0, (struct timeval*)&timeout);
     if(result==-1){
      perror("server not working...\n");
      exit(EXIT_FAILURE);
      }
    
     for(fd=0; fd< FD_SETSIZE; fd++){
      if(FD_ISSET(fd,&testfds)) {
       
        nreads=read(fd,(void*) msg,(size_t) sizeof(msg));
        if (nreads<=0){
         eliminaclient(fd, &myfds, NULL, 1); 
         continue;
         }
        printf("serving client [%s]\n", utenti[fd].name);
        if(strncmp("@add", msg, 4)==0) {
         char name[256];
         printf("Copio nome\n");
         sscanf(msg, "%s %s",newmsg,name);
         printf("%s\n", name);
         if (cercautente(name, myfds)!=-1) {
          strcpy(newmsg, "Utente già presente\n");
          printf("%s", newmsg);
          write(fd, (void*)newmsg,(size_t)strlen(newmsg)+1);
          }
         
         else addmember(fd ,name ,&myfds);
         }
         else if(strncmp("@close", msg, 6)==0){ 
          printf("Ricevuta richiesta di chiusura da %s\n", utenti[fd].name);
          eliminaclient(fd, &myfds, NULL, 1);
         }
         
       
        else {
         sprintf(newmsg, "[%s]", utenti[fd].name);
         nreads=nreads+strlen(newmsg);
         strcat(newmsg, msg);
         printf("%s ::ricevuto\n", newmsg); 
         for (fd1=4; fd1<FD_SETSIZE; fd1++){
          if(FD_ISSET(fd1, &myfds) && fd1!=fd){
           printf("invio a %s\n", utenti[fd1].name); 
           write(fd1,(void*) newmsg,(size_t) nreads+1);
            }
           }
          }
        } 
      
      
       }
     
      }



}     
void *thread_reg(void*arg){
  int sock, flag, fd, result,op;
  pthread_mutex_lock(&mreg);
  sock=*((int*)arg);
  pthread_mutex_unlock(&mreg);
  char msg[256], nome[256];
  read(sock, (void*)&flag,(size_t)sizeof(int));
  op=(int)flag;
  pthread_t private, mod;
  switch(op){
  case 0: 
          do{
           result=1;
           read(sock,(void*)nome,(size_t) sizeof(nome));
           for(fd=0; fd<FD_SETSIZE; fd++){
            if (FD_ISSET(fd, &utentifds)){
             if(strcmp(nome, utenti[fd].name)==0){
             result=0;
             write(sock, (void*)&result,(size_t)sizeof(int));
             continue;
              }
          
             } 
            
            } 
            
           }while(!result);
          result=1;
          write(sock, (void*)&result,(size_t)sizeof(int)); 
          strcpy(utenti[sock].name, nome);
          printf("[%s] collegato\n", utenti[sock].name);
          utenti[sock].libero=1;
          utenti[sock].stanze_aperte=0;
          strcpy(msg, "Utente collegato\n");
          write(sock,(void*) msg,(size_t)sizeof(msg));
          FD_SET(sock,&utentifds);
          FD_SET(sock,&readfds); //provo scrittura, ma dipende dalle scelte fatte...
          break;
  case 1://utente già presente, crea stanza privata, verifico nome
          read(sock, (void*)nome, (size_t) sizeof(nome));//attendo di ricevere il nome
          strcpy(utenti[sock].name,nome);
          result=1;//assegno il nome al nuovo file descriptor
          for(fd=0; fd<FD_SETSIZE; fd++){
           if(FD_ISSET(fd, &utentifds)){
            result=strcmp(utenti[fd].name,nome);
            if(result==0) break;
            }
           } 
          if(result!=0){
           printf("Client mai registrato\n");
           strcpy(msg, "Nome non valido");
           write(sock,(void*) msg,(size_t)sizeof(msg));
           close(sock);
           }
          else {
           utenti[fd].stanze_aperte++;
           static int priv_sock;
           priv_sock=sock;
           pthread_create(&private, NULL, private_handler,(void*)&priv_sock);
           
           }
            
          break;
  case 2://utente aggiunto a stanza privata
          read(sock, (void*)nome, (size_t) sizeof(nome));//attendo di ricevere il nome
          strcpy(utenti[sock].name,nome);//assegno il nome al nuovo file descriptor
          result=1;
          for(fd=0; fd<FD_SETSIZE; fd++){
           if(FD_ISSET(fd, &utentifds)){
            result=strcmp(utenti[fd].name,nome);
            if(result==0) break;
            }
           } 
          if(result!=0){
           strcpy(msg, "Nome non valido");
           write(sock,(void*) msg,(size_t)sizeof(msg));
           close(sock);
           }
          else {
           FD_SET(sock,utenti[fd].richiesta);
           utenti[fd].libero=1;
           utenti[fd].stanze_aperte++;
           printf("Utente aggiunto a stanza privata\n");
          }
           break;
  case 3: //moderatore
          if(mod_sock!=-1) {
           strcpy(msg, "Moderatore già presente\n");
           write(sock,(void*)msg,(size_t)strlen(msg)+1);
           close(sock);
          }
          else {
           mod_sock=sock;
           strcpy(msg,"Benvenuto\n");
           write(mod_sock,(void*)msg,(size_t)strlen(msg)+1);
           pthread_create(&mod, NULL, mod_handler,NULL);
          } 
          break;        
  default: strcpy(msg, "Flag non valido\n");
           write(sock, (void*)msg, (size_t) sizeof(msg));
           close(sock);
           pthread_exit(NULL);                
  }
} 


     
void *thread_listen(){
 
 pthread_t reg;
 

 
 while(1){
  pthread_mutex_lock(&mreg);
  addr_len = sizeof(client_address);
  client_sock = accept(server_sock, (struct sockaddr*)&client_address,(socklen_t*) &addr_len);
  printf("trovata richiesa %d\n", client_sock);
  pthread_mutex_unlock(&mreg);
  pthread_create(&reg, NULL, thread_reg, (void*)&client_sock);
  if(reg==-1){
  perror("Impossibile registrare utente\n");
  //invio messaggio di errore
  }
  else sleep(2);//lascio spazio al thread di registrazione 
 }
} 
    
void eliminaclient(int fd, fd_set* readfds, fd_set* writefds, int flag){
 char msg[256], nome[200];//per avvisare gli altri utenti
 printf("Operazione di chiusura\n");
 int fd1;
 
 FD_CLR(fd, readfds);
 strcpy(nome, utenti[fd].name);
 switch(flag){
  case 0://fd di stanza pubblica
         FD_CLR(fd, writefds);
         if (utenti[fd].stanze_aperte==0) {
         FD_CLR(fd,&utentifds);
         close(fd);
         }
         else utenti[fd].stanze_aperte--;
         break;
  case 1://fd di stanza privata
         fd1=cercautente(utenti[fd].name, utentifds);
         if (fd1==-1){
         printf("Errore di ricerca\n");
         return;
         }
         utenti[fd1].stanze_aperte--;
         if (utenti[fd1].stanze_aperte==-1) {
         FD_CLR(fd,&utentifds);
         close(fd1);
         }
         close(fd);
         break;
   default: return;              
 
 }
 sprintf(msg,"[%s] ha abbandonato\n", nome);
 inviamsg(msg, *readfds);
 return;

}     
