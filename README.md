# chat

Simple C chat for Linux, with public and private rooms.
It uses sockets in the internet domain.

# Files


There are five different files needed:

server_chat.c

client_chat.c

client_chat1.c

client_chat2.c

client_mod

# Server


The server is the one that lets the communication happen. It uses the select() to monitor multiple sockets.


The server is multithreaded. There's a thread to accept new connections, which creates new registration threads. The first thing the client sends to the server is a flag with values from 0 to 3. (0: new user, 1: user already in the public room, who wants to create a new private room, 2:user invited to join a private room ,3: moderator). Each private room is also handled by a thread very similar to the main. There's also a thread to wait for new messages coming from the mod_client, if he is present.


Every room handler, including the main(), periodically checks if there's a new moderator message. The server sends a message to the client_mod to confirm each room sent the message to all its members.



# Clients


All the clients are very similar. 
When the user asks to create a new room, the process forks and through execl() and xterm command it runs client_chat1 in a new terminal emulator. Similarly, when the user is added to a private room, the process forks and runs client_chat2.


The client_mod doesn't receive any message, but its messages are broadcast to all rooms.


In the client, I used the select() to monitor both the socket and the keyboard. 


# How to use


To access the chat, the client version to run is the client_chat.
Once connected, user will be asked a nickname. Although two users cannot have the same nickname if they are both connected, this piece of information is not permanently stored, so no password required. 


If the user is in the public room, he can either create a private room or be added to one.  In both cases, he will get a new file descriptor and new console will be opened (through execl() and xterm command, client_chat1 to create a new room, client_chat2 to add user to a room).


To enter the chat as a moderator, the file to run is the client_mod. Just one mod at a time is allowed.


In the public room, the available commands are @help, to open a private room, and @close to exit.
In the private room, the available commands are @add to add a member and @close to exit.



# How to compile


Server:
cc -o server_chat server_chat.c -lpthread

Client:

cc -o client_chat client_chat.c

cc -o client_chat1 client_chat1.c

cc -o client_chat2 client_chat2.c

cc -o client_mod client_mod.c


NOTE: Names must remain as specified, otherwise the exec would fail.
