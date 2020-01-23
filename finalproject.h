
#ifndef _FINALPROJECT_H_
#define _FINALPROJECT_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>


//color codes
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define RESET "\033[0m"

//Enum of different messages possible.
typedef enum
{
	CONNECT,
	DISCONNECT,
	GET_USERS,
	SET_USERNAME,
	PUBLIC_MESSAGE,
	PRIVATE_MESSAGE,
	FILING,
	FILEND,
	USERNAME_ERROR,
	SUCCESS,
	ERROR,
	REGISTER,
	REG_SUCCESS,
	REG_PASSWORD,
	REG_FAIL,
	LOGIN,
	LOG_SUCCESS,
	LOG_FAIL,
	LOG_PASSWORD,
	DATA

} message_type;


//message structure
typedef struct message{
	message_type type;
	char username[21];
	char data[256];
} message;
//buffer messages structure
typedef struct buffer_message{
	char username[21];
	struct message mess;
	struct buffer_message *next;
}Buffer_message;

//user structure
typedef struct user{
	char username[21];
	char password[21];
	struct user *next; 
} User;

//structure to hold client connection information
typedef struct connection_info{
	int socket;
	struct sockaddr_in address;
	char username[21];
	struct connection_info *next;
} Connection_info;

//new buffer message
Buffer_message *newBM(char* usr, struct message msg);

//new connection
Connection_info *newNode(int sockfd, struct sockaddr_in addr);

// Removes the trailing newline character from a string.
void trim_newline(char *text);

//create a new user
User *newUser(char*usr,char*pwd);

// discard any remaining data on stdin buffer.
void clear_stdin_buffer();

#endif
