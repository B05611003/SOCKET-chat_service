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
User & Operator Guide fot client</p>
/help or /h: Displays help information.</p>
/quit or /q: Exit the program.</p>
/list or /l: Displays list of users in chatroom.</p>
/f <username> <fileName1> <fileName2>... Send file to given username.</p>
/a message... Send file to given username.</p>
/m <username> <message> Send a private message to given username.</p>
when an account is currently offline, your message will store in server buffer and sent when the target has login.

