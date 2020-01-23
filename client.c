#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "finalproject.h"

// get a username from the user.
void get_username(char *username){
	while(true)
	{
		printf("Enter a username: ");
		//fflush(stdout);
		memset(username, 0, 21);
		fgets(username, 22, stdin);
		trim_newline(username);

		if(strlen(username) > 20){
			// clear_stdin_buffer();
			puts("Username must be 20 characters or less.");
		} 
		else {break;}
	}
}

// login to a specific account
void login(Connection_info *connection){
	Connection_info *here = connection;
	message msg;
	msg.type = LOGIN;
	char tmpname[21];	

	while(msg.type != LOG_SUCCESS){
		if(msg.type == LOG_FAIL){printf("Username not found, please type again!!\n");}
		get_username(&msg.data);
		strcpy(tmpname,msg.data);
		msg.type = LOGIN;
		if((send((here->socket), &msg, sizeof(msg), 0)) < 0){
			perror("Send failed");
			exit(1);
		}
		if((recv((here->socket), &msg, sizeof(message), 0)) < 0){
			perror("recv failed");
			exit(1);
		}
	}

	msg.type = LOG_PASSWORD;
	strcpy(msg.username,tmpname);
	while(msg.type != LOG_SUCCESS && msg.type != PRIVATE_MESSAGE){
		if (msg.type == LOG_FAIL){printf("Wrong password, please type again!!\n");}
		while(true){
			printf("Enter a password: ");
			fflush(stdout);
			//memset(&msg.data, 0, 256);
			fgets(&msg.data, 22, stdin);
			trim_newline(&msg.data);
			if(strlen(msg.data) > 20){
					puts("password must be 20 characters or less.");
			} 
			else{break;}
		}
		if((send((here->socket), &msg, sizeof(msg), 0)) < 0){
			perror("Send failed");
			exit(1);
		}
		if((recv((here->socket), &msg, sizeof(message), 0)) < 0){
			perror("recv failed");
			exit(1);
		}	
	}
	if (msg.type != LOG_SUCCESS){printf("You have some message while offline\n");}
	while(msg.type != LOG_SUCCESS){
		printf(KWHT "From %s:" KCYN " %s\n" RESET, msg.username, msg.data);
		if((recv((here->socket), &msg, sizeof(message), 0)) < 0){
			perror("recv failed");
			exit(1);
		}
	}
	printf("Welcome back, %s!\n",tmpname );

}
// regiter a new account
void regiter(Connection_info *connection){
	Connection_info *here = connection;
	message msg;
	msg.type = REGISTER;
	
	char tmpname[21];	
	while(msg.type != REG_SUCCESS){
		if(msg.type == REG_FAIL){
			printf("username has been use, please type again!!\n");
		}
		get_username(&msg.data);
		strcpy(tmpname,msg.data);
		msg.type = REGISTER;
		if((send((here->socket), &msg, sizeof(msg), 0)) < 0){
			perror("Send failed");
			exit(1);
		}
		if((recv((here->socket), &msg, sizeof(message), 0)) < 0){
			perror("recv failed");
			exit(1);
		}
	}

	msg.type = REG_PASSWORD;

	while(true){
		printf("Enter a password: ");
		fflush(stdout);
		//memset(&msg.data, 0, 256);
		fgets(&msg.data, 22, stdin);
		trim_newline(&msg.data);
		if(strlen(msg.data) > 20){
				puts("password must be 20 characters or less.");
		} 
		else{
				break;
		}
	}
	strcpy(msg.username,tmpname);
	if(send(connection->socket, (void*)&msg, sizeof(msg), 0) < 0){
		perror("Send failed");
		exit(1);
	}
	if(recv(connection->socket, &msg, sizeof(message), 0) < 0){
		perror("recv failed");
		exit(1);
	}
	if ((msg.type) == REG_SUCCESS){

		printf("Account created! \n");
	}
	return;
}

//receive file handler
void recv_file(Connection_info *connection,char* filename){
	message msg;
	//int check;
	FILE *wfile = fopen(filename,"wb");
	if(!wfile) { 
		perror("can't open file"); 
		exit(1);
	}
	if(recv(connection->socket, &msg, sizeof(message), 0) < 0){
		perror("Send failed");
		exit(1);
	}
	//receive data until msg.type = FILE
	while(msg.type != FILEND){

		fwrite(&msg.data, sizeof(char), atoi(msg.username), wfile);
		if(recv(connection->socket, &msg, sizeof(message), 0) < 0){
			perror("Send failed");
			exit(1);
		}
	}

	fclose(wfile);
	return;


}
//send file handler
int sendfile(Connection_info *connection,char* filename,char* toUser){
	message msg;
	msg.type = FILING;
	int check;
	strcpy(msg.username,toUser);
	strcpy(msg.data,filename);
	FILE *file = fopen(filename, "rb");	
	if(!file) { 
		perror("can't open file"); 
		exit(1);
	} 
	//send file initial message
	if(send(connection->socket, &msg, sizeof(message), 0) < 0){
		perror("Send failed");
		exit(1);
	}
	//acception
	if(recv(connection->socket, &msg, sizeof(message), 0) < 0){
		perror("Send failed");
		exit(1);
	}
	if(msg.type == ERROR){
		printf("%s is either offline or do not exist!\n",toUser);
		return 0;
	}
	if(msg.type!=SUCCESS){
		printf("critical error\n");
		fclose(file);
		return 0;
	}
	while(!feof(file)){
		msg.type = DATA;
		memset(msg.data,'\0',256);
		check = fread(&msg.data, sizeof(char), 256, file);
		//printf("%d\n",check);
		sprintf(msg.username,"%d",check);
		if (check == 0){break;}
		if(send(connection->socket, &msg, sizeof(message), 0) < 0){
			perror("Send failed");
			exit(1);
		}	
	}
	msg.type = FILEND;
	if(send(connection->socket, &msg, sizeof(message), 0) < 0){
		perror("Send failed");
		exit(1);
	}
	if(recv(connection->socket, &msg, sizeof(message), 0) < 0){
		perror("Send failed");
		exit(1);
	}
	if (msg.type != SUCCESS){
		fclose(file);
		return 0;
	}
	fclose(file);
	return 1;
}
//send local username to the server.
void set_username(Connection_info *connection){
	message msg;
	msg.type = SET_USERNAME;
	strncpy(msg.username, connection->username, 20);

	if(send(connection->socket, (void*)&msg, sizeof(msg), 0) < 0)
	{
		perror("Send failed");
		exit(1);
	}
}
//stop and close
void stop_client(Connection_info *connection){
	close(connection->socket);
	exit(0);
}

//initialize connection to the server.
void connect_to_server(Connection_info *connection, char *address, char *port){
	char *addr = malloc(strlen(address)*sizeof(char));
	char *por = malloc(strlen(port)*sizeof(char));
	strcpy(addr,address);
	strcpy(por,port);
	while(true)
	{
		//get_username(connection->username);

		//Create socket
		if ((connection->socket = socket(AF_INET, SOCK_STREAM , IPPROTO_TCP)) < 0){
				perror("Could not create socket");
		}

		connection->address.sin_addr.s_addr = inet_addr(addr);
		connection->address.sin_family = AF_INET;
		connection->address.sin_port = htons(atoi(por));

		//Connect to remote server
		if (connect(connection->socket, (struct sockaddr *)&connection->address , sizeof(connection->address)) < 0){
			perror("Connect failed.");
			exit(1);
		}
		break;
	}


	puts("Connected to server.");
	puts("Type /help for usage.");
}


void handle_user_input(Connection_info *connection){
	char input[255];
	fgets(input, 255, stdin);
	trim_newline(input);

	if(strcmp(input, "/q") == 0 || strcmp(input, "/quit") == 0){
		stop_client(connection);
	}
	else if (strcmp(input, "/LOGIN")== 0){
		login(connection);
	}
	else if (strcmp(input, "/REG")== 0){
		regiter(connection);
	}
	else if(strcmp(input, "/l") == 0 || strcmp(input, "/list") == 0){
		message msg;
		msg.type = GET_USERS;
		if(send(connection->socket, &msg, sizeof(message),0)== 0){
			perror("recv failed");
			exit(1);
		}
	}
	else if(strcmp(input, "/h") == 0 || strcmp(input, "/help") == 0){
		puts("/quit or /q: Exit the program.");
		puts("/help or /h: Displays help information.");
		puts("/list or /l: Displays list of users in chatroom.");
		puts("/REG: Register a new account");
		puts("/LOGIN: Login a exist account");
		puts("/f <username> <fileName1> <fileName2>... Send file to given username.");
		puts("/a message... Send file to given username.");
		puts("/m <username> <message> Send a private message to given username.");
	}
	else if(strncmp(input, "/f", 2) == 0){
		char *toUsername, *fileName;
		toUsername = strtok(input+3, " ");
		if(toUsername == NULL){
			puts(KRED "The format for sendfile is: /f <username> <filename>" RESET);
			return;
		}
		if(strlen(toUsername) == 0){
			puts(KRED "You must enter a username for a private message." RESET);
			return;
		}
		if(strlen(toUsername) > 20){
			puts(KRED "The username must be between 1 and 20 characters." RESET);
			return;
		}
		fileName = strtok(NULL, " ");

		if(fileName == NULL){
			puts(KRED "You must enter fileName to send to the specified user." RESET);
			return;
		}

		printf("sending file...\n");
		while(fileName!=NULL){
			if(sendfile(connection,fileName,toUsername)){
				printf("%s complete!!\n",fileName);
			}		
			else{
				printf("%s failed!!\n",fileName);
			}
			fileName = strtok(NULL, " ");
		}

	}
	else if(strncmp(input, "/m", 2) == 0){
		message msg;
		msg.type = PRIVATE_MESSAGE;
		char *toUsername, *chatMsg;
		toUsername = strtok(input+3, " ");

		if(toUsername == NULL)
		{
			puts(KRED "The format for private messages is: /m <username> <message>" RESET);
			return;
		}
		if(strlen(toUsername) == 0){
			puts(KRED "You must enter a username for a private message." RESET);
			return;
		}
		if(strlen(toUsername) > 20)
		{
			puts(KRED "The username must be between 1 and 20 characters." RESET);
			return;
		}
		chatMsg = strtok(NULL, "");

		if(chatMsg == NULL)
		{
			puts(KRED "You must enter a message to send to the specified user." RESET);
			return;
		}

		//printf("|%s|%s|\n", toUsername, chatMsg);
		strncpy(msg.username, toUsername, 20);
		strncpy(msg.data, chatMsg, 255);

		if(send(connection->socket, &msg, sizeof(message), 0) < 0){
			perror("Send failed");
			exit(1);
		}
		if (recv(connection->socket, &msg, sizeof(message), 0) < 0){
			perror("Send failed");
			exit(1);	
		}
		if (msg.type != SUCCESS){printf("%s\n",msg.data );}
	}
	else if(strncmp(input, "/a", 2) == 0){//regular public message	
		message msg;
		msg.type = PUBLIC_MESSAGE;
		strncpy(msg.username, connection->username, 20);

		// clear_stdin_buffer();

		if(strlen(input) == 0) {
				return;
		}

		strncpy(msg.data, input, 255);

		//Send some data
		if(send(connection->socket, &msg, sizeof(message), 0) < 0){
				perror("Send failed");
				exit(1);
		}
	}
	else{
		printf("unknown command received\n");
	}
}

void handle_server_message(Connection_info *connection){
	message msg;
	//Receive a reply from the server
	ssize_t recv_val = recv(connection->socket, &msg, sizeof(message), 0);
	if(recv_val < 0){
			perror("recv failed");
			exit(1);

	}
	else if(recv_val == 0)
	{
		close(connection->socket);
		puts("Server disconnected.");
		exit(0);
	}

	switch(msg.type)
	{

		case CONNECT:
			printf(KYEL "%s has connected." RESET "\n", msg.username);
		break;

		case DISCONNECT:
			printf(KYEL "%s has disconnected." RESET "\n" , msg.username);
		break;



		case SET_USERNAME:
			//TODO: implement: name changes in the future?
		break;
		case PUBLIC_MESSAGE:
			printf(KGRN "All chat from%s" RESET ": %s\n", msg.username, msg.data);
		break;

		case GET_USERS:
			printf("%s", msg.data);
		break;

		case PRIVATE_MESSAGE:
			printf(KWHT "From %s:" KCYN " %s\n" RESET, msg.username, msg.data);
		break;

		case FILING:
			printf(KWHT "Receiving file from %s," KCYN"filename:%s\n" RESET,msg.username,msg.data);
			recv_file(connection,msg.data);
			printf("received\n");	
		break;

		default:
			fprintf(stderr, KRED "Unknown message type received." RESET "\n");
		break;
	}
}

int main(int argc, char *argv[]){

	Connection_info connection;
	fd_set file_descriptors;

	if (argc != 3) {
		fprintf(stderr,"Usage: %s <IP> <port>\n", argv[0]);
		exit(1);
	}


	connect_to_server(&connection, argv[1], argv[2]);

	//keep communicating with server
	while(true)
	{
		FD_ZERO(&file_descriptors);
		FD_SET(STDIN_FILENO, &file_descriptors);
		FD_SET(connection.socket, &file_descriptors);
		fflush(stdin);


		if(select(connection.socket+1, &file_descriptors, NULL, NULL, NULL) < 0)
		{
			perror("Select failed.");
			exit(1);
		}
		
		if(FD_ISSET(STDIN_FILENO, &file_descriptors))
		{
			handle_user_input(&connection);
		}
		
		if(FD_ISSET(connection.socket, &file_descriptors))
		{
			
			handle_server_message(&connection);
		}
	}

	close(connection.socket);
	return 0;
}
