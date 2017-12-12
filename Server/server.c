//Nguyen Khanh hung
//14020224
//Chuong trinh Chatroom Client-client qua server
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFF 1024
struct client_info
{
	int sockno;
	char ip[INET_ADDRSTRLEN];
	char name[BUFF];
};



int clients[100];
int n = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void sendtoall(char *msg, int curr)
{
	//sendtoall
	int i;
	pthread_mutex_lock(&mutex);
	for (i = 0; i < n; i++)
	{
		if (clients[i] != curr)
		{
			if (write(clients[i], msg, strlen(msg)) < 0)
			{
				perror("sending failure");
				continue;
			}
		}
	}
	pthread_mutex_unlock(&mutex);
}
void *recvmg(void *sock)
{

	struct client_info cl = *((struct client_info *)sock);
	char msg[500];
	int len;
	int i;
	int j;
	while ((len = read(cl.sockno, msg, 500)) > 0)
	{
		msg[len] = '\0';
		printf("%s", msg);
		sendtoall(msg, cl.sockno);
		memset(msg, '\0', sizeof(msg));
	}
	pthread_mutex_lock(&mutex);
	printf("%s disconnected\n", cl.name);
	for (i = 0; i < n; i++)
	{
		if (clients[i] == cl.sockno)
		{
			j = i;
			while (j < n - 1)
			{
				clients[j] = clients[j + 1];
				j++;
			}
		}
	}
	n--;
	pthread_mutex_unlock(&mutex);
}
int main(int argc, char *argv[])
{
	struct sockaddr_in my_addr, their_addr;
	int my_sock;
	int their_sock;
	socklen_t their_addr_size;
	// int portno;
	pthread_t sendt, recvt;
	char msg[500];
	int len;
	struct client_info cl;
	char ip[INET_ADDRSTRLEN];

	my_sock = socket(AF_INET, SOCK_STREAM, 0);
	memset(my_addr.sin_zero, '\0', sizeof(my_addr.sin_zero));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(7891);
	my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	their_addr_size = sizeof(their_addr);

	if (bind(my_sock, (struct sockaddr *)&my_addr, sizeof(my_addr)) != 0)
	{
		perror("binding unsuccessful");
		exit(1);
	}
	printf("Server Ready\n");
	if (listen(my_sock, 5) != 0)
	{
		perror("listening unsuccessful");
		exit(1);
	}

	while (1)
	{
		if ((their_sock = accept(my_sock, (struct sockaddr *)&their_addr, &their_addr_size)) < 0)
		{
			perror("accept unsuccessful");
			exit(1);
		}

		read(their_sock, msg, sizeof(msg));
		printf("%s ", msg);
		strcpy(cl.name, msg);
		bzero(&msg, sizeof(msg));

		pthread_mutex_lock(&mutex);
		inet_ntop(AF_INET, (struct sockaddr *)&their_addr, ip, INET_ADDRSTRLEN);
		printf(" connected\n");
		cl.sockno = their_sock;
		strcpy(cl.ip, ip);
		clients[n] = their_sock;
		n++;
		pthread_create(&recvt, NULL, recvmg, &cl);
		pthread_mutex_unlock(&mutex);
	}
	return 0;
}