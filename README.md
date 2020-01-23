SOCKET chat_service
========
A simple linux **command-line-based** **client-server** C program for chat service and file transfering

Written by Alex Tsai 


---
### Usage:
`make`</p>
for server:</p>
`./server <port>`</p>
and for clients:</p>
`./client <ip address> <port>`</p>
User & Operator Guide fot client
/help or /h: Displays help information.
/quit or /q: Exit the program.
/list or /l: Displays list of users in chatroom.
/f <username> <fileName1> <fileName2>... Send file to given username.
/a message... Send file to given username.
/m <username> <message> Send a private message to given username.
when an account is currently offline, your message will store in server buffer and sent when the target has login.

