# chat
To access the chat, the client version to run is the client_chat.
Once connected, user will be asked a nickname. Although two users cannot have the same nickname if they are both connected, this piece of information is not permanently stored, so no password required.

If the user is in the public room, he can either create a private room or be added to one.  In both cases, he will get a new file descriptor and new console will be opened (through execl() and xterm command, client_chat1 to create a new room, client_chat2 to add user to a room).

The client_mod file is to enter the chat as a moderator. Just one mod at a time is allowed.


