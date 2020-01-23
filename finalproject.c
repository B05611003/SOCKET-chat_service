#include "finalproject.h"

#include <string.h>
#include <stdlib.h>

//terminate new line
void trim_newline(char *text)
{
  int len = strlen(text) - 1;
  if (text[len] == '\n')
{
      text[len] = '\0';
  }
}

void clear_stdin_buffer()
{
  int c;
  while((c = getchar()) != '\n' && c != EOF)
    /* discard content*/ ;
}

// new a client datadype
Connection_info *newNode(int sockfd, struct sockaddr_in addr) {
	Connection_info *np = (Connection_info *)malloc( sizeof(Connection_info) );
	np->socket = sockfd;
	np->next = NULL;
	np->address = addr;

	return np;
}

// new a buffer message datatype
Buffer_message *newBM(char* usr, struct message msg){
	Buffer_message *nbm = (Buffer_message *)malloc( sizeof(Buffer_message) );
	strcpy(nbm->username,usr);
	nbm->mess = msg;
	nbm->next = NULL;
	return nbm;
}

// new a user datatype
User *newUser(char*usr,char*pwd){
	User *nu = (User *)malloc( sizeof(User) );
	strcpy(nu->username,usr);
	strcpy(nu->password,pwd);
	nu->next = NULL;
	return nu;
}