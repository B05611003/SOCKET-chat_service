#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include "finalproject.h"

int server_sockfd = 0, client_sockfd = 0, clientcount = 0;

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
FILE *serverlog ;


Connection_info *server_info;
Connection_info *root,*now;
User *usroot;
Buffer_message *bmroot;

void encryptDecrypt(char* inpString) 
{ 
    // Define XOR key 
    // Any character value will work 
    char xorKey = 'P'; 
  
    // calculate length of input string 
    int len = strlen(inpString); 
  
    // perform XOR operation of key 
    // with every caracter in string 
    for (int i = 0; i < len; i++) 
    { 
        inpString[i] = inpString[i] ^ xorKey; 
        //printf("%c",inpString[i]); 
    } 
} 

void catch_ctrl_c_and_exit(int sig) {
	
	printf("server shutting down\n");
	fclose(serverlog);
	exit(EXIT_SUCCESS);
}
void initialize_server(int port){
	serverlog = fopen("server.log","w");

	server_info = (Connection_info *)malloc(sizeof(Connection_info));
	if((server_info->socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Failed to create socket");
		exit(1);
	}

	server_info->address.sin_family = AF_INET;
	server_info->address.sin_addr.s_addr = INADDR_ANY;
	server_info->address.sin_port = htons(port);

	if(bind(server_info->socket, (struct sockaddr *)&server_info->address, sizeof(server_info->address)) < 0)
	{
		perror("Binding failed");
		exit(1);
	}

	const int optVal = 1;
	const socklen_t optLen = sizeof(optVal);
	if(setsockopt(server_info->socket, SOL_SOCKET, SO_REUSEADDR, (void*) &optVal, optLen) < 0)
	{
		perror("Set socket option failed");
		exit(1);
	}


	if(listen(server_info->socket, 5) < 0) {
		perror("Listen failed");
		exit(1);
	}

	//Accept and incoming connection

	printf("Start Server on: %s:%d\n", inet_ntoa(server_info->address.sin_addr), ntohs(server_info->address.sin_port));
	printf("Waiting for incoming connections...\n");
	fprintf(serverlog,"Start Server on: %s:%d\n", inet_ntoa(server_info->address.sin_addr), ntohs(server_info->address.sin_port));
	fprintf(serverlog,"Waiting for incoming connections...\n");



}


void delete_connection(Connection_info *p_client){
	Connection_info *cur = root,*prev;
	while(cur != NULL){
		if(cur->socket == p_client->socket){
			if(cur == root){
				root = cur->next;
			}
			else{
				prev->next = cur->next;
			}
			free(cur);
			clientcount--;
			return;
		}
		prev = cur;
		cur = cur->next;
	}
	printf("critical error\n");
	return;
}

void check_login(Connection_info *p_client,char* usr){
	message msg;
	msg.type = LOGIN;
	User *cur ;
	char *nowname = usr;

	while(msg.type != LOG_PASSWORD){
		cur = usroot;

		if(cur == NULL){msg.type = LOG_FAIL;}
		else{
			while(1){
				if (!strcmp(cur->username,nowname)){
					msg.type = LOG_SUCCESS;
					break;
				}
				if (cur->next == NULL){
					msg.type = LOG_FAIL;
					break;
				}
				cur = cur->next;
			}
		}
		if((send(p_client->socket, &msg, sizeof(msg), 0)) < 0){
			perror("Send failed");
			exit(1);
		}
		if((recv(p_client->socket, &msg, sizeof(msg), 0)) < 0){
			perror("Send failed");
			exit(1);
		}		
		nowname = msg.data;
	}
	while(strcmp(cur->password,msg.data)!=0){
		msg.type = LOG_FAIL;
		// if (!strcmp(cur->password,nowname)){
		// 	msg.type = LOG_SUCCESS;
		// 	break;
		// }
		if((send(p_client->socket, &msg, sizeof(msg), 0)) < 0){
			perror("Send failed");
			exit(1);
		}
		if((recv(p_client->socket, &msg, sizeof(msg), 0)) < 0){
			perror("Send failed");
			exit(1);
		}		
	}
	//printf("coo start\n");
	Connection_info *curr = root;

	while(curr->socket != p_client->socket){
		//printf("fd:%d,%d\n", curr->socket,p_client->socket);
		curr = curr->next;
	}
	//printf("coo end\n");
	strcpy(curr->username,cur->username);
	printf("client %s:%d has login as %s.\n",inet_ntoa(p_client->address.sin_addr), ntohs(p_client->address.sin_port),curr->username);
	pthread_mutex_lock( &mutex2 );
	fprintf(serverlog, "client %s:%d has login as %s.\n",inet_ntoa(p_client->address.sin_addr), ntohs(p_client->address.sin_port),curr->username);
	pthread_mutex_unlock( &mutex2 );
	Buffer_message *currr = bmroot,*tmp;
	bool del = 0;
	while(currr!=NULL){
		//printf("check\n");
		del = 0;
		if(!strcmp(cur->username,currr->username)){
			//printf("find\n");
			del = 1;
			currr->mess.type = PRIVATE_MESSAGE;
			if((send(p_client->socket, &currr->mess, sizeof(currr->mess), 0)) < 0){
				perror("Send failed");
				exit(1);
			}
			//delete
			if(tmp != NULL){
				//printf("delete\n");
				tmp->next = currr->next;
			}
			else{
				bmroot =NULL;
			}
		}
		if (!del){tmp =currr;}
		currr = currr->next;
	}
	msg.type = LOG_SUCCESS;
	if((send(p_client->socket, &msg, sizeof(msg), 0)) < 0){
		perror("Send failed");
		exit(1);
	}
	return;
}

//register user
void check_user(Connection_info *p_client,char* usr){
	message msg;
	msg.type = REGISTER;
	User *cur ;
	User *tmp ;
	char *nowname = usr;
	
	while(msg.type != REG_PASSWORD){
		cur = usroot;
		
		if(cur == NULL){msg.type = REG_SUCCESS;}
		else{
			while(1){
				if (!strcmp(cur->username,nowname)){
					msg.type = REG_FAIL;
					break;
				}
				if (cur->next == NULL){
					msg.type = REG_SUCCESS;
					break;
				}
				cur = cur->next;
			}			
		}
		
		if((send(p_client->socket, &msg, sizeof(msg), 0)) < 0){
			perror("Send failed");
			exit(1);
		}
		if((recv(p_client->socket, &msg, sizeof(msg), 0)) < 0){
			perror("Send failed");
			exit(1);
		}		
		nowname = msg.data;
	}

	tmp = newUser(msg.username,msg.data);
	if (cur == NULL){
		usroot = tmp;
	}
	else{
		cur->next = tmp;
	}

	msg.type = REG_SUCCESS;

	if((send(p_client->socket, &msg, sizeof(msg), 0)) < 0){
		perror("Send failed");
		exit(1);
	}	
	return;
}

void send_public_message(Connection_info *p_client, char *message_text){
	message msg;
	msg.type = PUBLIC_MESSAGE;
	printf("public message sent by %s:%s\n", p_client->username,  message_text);
	strncpy(msg.username, p_client->username, 20);
	strncpy(msg.data, message_text, 256);
	pthread_mutex_lock( &mutex2 );
	fprintf(serverlog, "public message sent by %s:%s\n", p_client->username,  message_text);
	pthread_mutex_unlock( &mutex2 );
	Connection_info *curr = root;
	while(curr != NULL){
		if(send(curr->socket, &msg, sizeof(msg), 0) < 0)
		{
			perror("Send failed");
			exit(1);
		}
		curr = curr->next;
	}

	return;
}

//sending file 
void sendfile(Connection_info *p_client, char *username, char *filename){
	message msg;
	msg.type = FILING;
	printf("%s->%s:[file]%s \n", p_client->username,username,  filename);
	pthread_mutex_lock( &mutex2 );
	fprintf(serverlog, "%s->%s:[file] %s \n", p_client->username,username,  filename);
	pthread_mutex_unlock( &mutex2 );
	strncpy(msg.username, p_client->username, 21);
	strncpy(msg.data, filename, 256);
	Connection_info *curr = root;
	while(curr != NULL){
		if(strcmp(curr->username, username) == 0){
			//call target to initial
			if(send(curr->socket, &msg, sizeof(msg), 0) < 0){
				perror("Send failed");
				exit(1);
			}
			msg.type = SUCCESS;
			//call source that can start
			if(send(p_client->socket, &msg, sizeof(msg), 0) < 0){
				perror("Send failed");
				exit(1);
			}
			//recv first data 
			if(recv(p_client->socket, &msg, sizeof(msg), 0) < 0){
				perror("Send failed");
				exit(1);
			}
			while(msg.type == DATA){
				//send target the data 
				if(send(curr->socket, &msg, sizeof(msg), 0) < 0){
					perror("Send failed");
					exit(1);
				}
				//receive data from source 
				if(recv(p_client->socket, &msg, sizeof(msg), 0) < 0){
					perror("Send failed");
					exit(1);
				}
			}
			if(send(curr->socket, &msg, sizeof(msg), 0) < 0){
				perror("Send failed");
				exit(1);
			}
			msg.type = SUCCESS;
			//tell source finish
			if(send(p_client->socket, &msg, sizeof(msg), 0) < 0){
				perror("Send failed");
				exit(1);
			}
			return;
		}
		curr = curr->next;
	}
	msg.type = ERROR;
	if(send(p_client->socket, &msg, sizeof(msg), 0) < 0){
		perror("Send failed");
		exit(1);
	}
	return;

}
//sending private message 
void send_private_message(Connection_info *p_client, char *username, char *message_text){
	message msg;
	msg.type = PRIVATE_MESSAGE;
	printf("%s->%s:%s \n", p_client->username,username,  message_text);
	pthread_mutex_lock( &mutex2 );
	fprintf(serverlog, "%s->%s:%s \n", p_client->username,username,message_text);
	pthread_mutex_unlock( &mutex2 );
	strncpy(msg.username, p_client->username, 21);
	strncpy(msg.data, message_text, 256);
	Connection_info *curr = root;
	User *cur = usroot;
	while(curr != NULL){
		if(strcmp(curr->username, username) == 0){
			if(send(curr->socket, &msg, sizeof(msg), 0) < 0){
				perror("Send failed");
				exit(1);
			}
			msg.type = SUCCESS;
			if(send(p_client->socket, &msg, sizeof(msg), 0) < 0){
				perror("Send failed");
				exit(1);
			}
			return;
		}
		curr = curr->next;
	}
	
	while(cur != NULL){
		if(strcmp(cur->username, username) == 0){
			//store in buffer 
			Buffer_message *currr = bmroot,*tmpp;
			message go;
			strncpy(go.username, p_client->username, 21);
			strncpy(go.data, message_text, 256);
			go.type = PRIVATE_MESSAGE;
			tmpp = newBM(username,go);
			if(currr == NULL){
				bmroot = tmpp;
			}
			else{
				while(1){
					if(currr->next == NULL){
						currr->next = tmpp;
						break;
					}
					currr = currr->next;
				}
			}


			msg.type = ERROR;
			sprintf(msg.data, "User \"%s\" is currrently offline and will recieve when online\n", username);
			if(send(p_client->socket, &msg, sizeof(msg), 0) < 0){
				perror("Send failed");
				exit(1);
			}
			

			return;
		}
		cur = cur->next;
	}


	msg.type = ERROR;
	// sprintf(msg.data, "User \"%s\" is currrently offline and will recieve when online", username);
	sprintf(msg.data, "Username \"%s\" does not exist\n", username);

	if(send(p_client->socket, &msg, sizeof(msg), 0) < 0)
	{
		perror("Send failed");
		exit(1);
	}
	return;
}

void send_connect_message(Connection_info *p_client){
	message msg;
	msg.type = SUCCESS;
	strncpy(msg.username, p_client->username, 21);
	if(send(p_client->socket, &msg, sizeof(msg), 0) < 0){
		perror("Send failed");
		exit(1);
	}


}

void send_user_list(Connection_info *p_client) {
	message msg;
	msg.type = GET_USERS;
	char *list =(char *)malloc( sizeof(char)*256 );
	Connection_info *curr = root;
	while(curr != NULL){
		if (curr->username != NULL ){
			strcat(list, curr->username);
			strcat(list, "\n");
		}
		curr = curr->next;
	}
	strcpy(msg.data,list);
	if(send(p_client->socket, &msg, sizeof(msg), 0) < 0){
			perror("Send failed");
			exit(1);
	}
}


void handle_client_message(void *p_client){
	int read_size;
	int leave_flag = 0;
	Connection_info *pt = (Connection_info *)p_client;
	message msg;
	//printf("handle\n");
	while(1){
		if (leave_flag) {
			break;
		}
		if((read_size = recv(pt->socket, &msg, sizeof(message), 0)) == 0){
			printf("Client %s:%d disconnected.\n", inet_ntoa(pt->address.sin_addr), ntohs(pt->address.sin_port));
			pthread_mutex_lock( &mutex2 );
			fprintf(serverlog, "Client %s:%d disconnected.\n", inet_ntoa(pt->address.sin_addr), ntohs(pt->address.sin_port));
			pthread_mutex_unlock( &mutex2 );
			leave_flag = 1;
		} 
		else{

			switch(msg.type)
			{
				case REGISTER:
					//check duplicate userrname
					check_user(pt,msg.data);			
				break;

				case LOGIN:
					check_login(pt,msg.data);
					break;

				case GET_USERS:
					send_user_list(pt);
				break;

				case SET_USERNAME: 
					strcpy(pt->username, msg.username);
					printf("User connected: %s\n", pt->username);
					send_connect_message(pt);
				break;

				case FILING:
					sendfile(pt, msg.username, msg.data);
				break;

				case PUBLIC_MESSAGE:
					send_public_message(pt, msg.data);
				break;

				case PRIVATE_MESSAGE:
					send_private_message(pt, msg.username, msg.data);

				break;

				default:
					fprintf(stderr, "Unknown message type received.\n");
				break;
			}
		}
	}

	
	pthread_mutex_lock( &mutex1 );
	delete_connection(pt);
	pthread_mutex_unlock( &mutex1 );
	close(pt->socket);
	pthread_exit(NULL);
}

int main(int argc, char *argv[]){
	signal(SIGINT, catch_ctrl_c_and_exit);
	puts("Starting server...");
	
	int server_len,client_len;
	struct sockaddr_in client;

	if (argc != 2){
		fprintf(stderr, "Usage: %s <port>\n", argv[0]);
		exit(1);
	}

	initialize_server(atoi(argv[1]));

	while(true)
	{
		client_sockfd = accept(server_info->socket, (struct sockaddr*)&server_info->address, (socklen_t*)&server_len);

		if (client_sockfd < 0){
			perror("Accept Failed");
			exit(1);
		}

		getpeername(client_sockfd, (struct sockaddr*) &client, (socklen_t*) &client_len);
		printf("Client %s:%d come in.\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
		pthread_mutex_lock( &mutex2 );
		fprintf(serverlog, "Client %s:%d come in.\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
		pthread_mutex_unlock( &mutex2 );
		
		if (clientcount ==0){
			root = newNode(client_sockfd,client);
			now = root;
			clientcount++;
		}
		else{
			Connection_info *c;
			c = newNode(client_sockfd,client);
			now = root;
			while(now!=NULL){
				if(now->next == NULL){
					now->next = c;
					now = c;
					break;
				}
				now = now->next;
			}			
			clientcount++;
		}

		
		pthread_t id;
		if (pthread_create(&id, NULL, (void *)handle_client_message, (void *)now) != 0) {
			perror("Create pthread error!\n");
			exit(EXIT_FAILURE);
		}
	}
	return 0;
}
