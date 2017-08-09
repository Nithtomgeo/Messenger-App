
#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/select.h>
#include<netinet/in.h>
#include<netdb.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<arpa/inet.h>
#include <fcntl.h>
#include<errno.h>
#include<signal.h>

/*structure similar to the one in the server*/
struct usertable
{
        int csock;
        char username[100];
        char password[100];
        char servhost[200];
        int servport;
        struct contactlist
        {
                char uname[100];
                char servhost[500];
                int servport;
        }contact[100];
        int option; 
}user[100];

struct configuration
{
	char servhost[500];
	char port[500];
	int servport;
}config;

/*structure to store the users data*/
struct usermessage
{
	int csock;
	char choice,word[200];
	char username[100];
	char password[100];
	char message[100];
	int status;
	char servhost[500];
	int servport;
	int ct;
	int fla;
	int invitacpt;
	struct contactl
	{
        int psock;
	char uname[100];
        char servhost[500];
        int servport;
	int ct;
	int ct1;
	int fla;
	int globe;
	}contact[100];
	int option;
}usemessage,recmessage,invite;

/*message packet structre*/
struct messagepakt
{
	char message[100];
	char sender[100];
	char receiver[100];
}mpkt;

/*function declarations*/
void send_message(int,char*,char *,char *);
void receive_message(int,char*);
void newconnection(int,char*,int);
void load(struct usermessage);
void display(struct usermessage);
void empty(struct usermessage);
int peer(char *);
void menu();
void accepting();
void receive(int);
void receivepeer(int);
void sending(int,char *);
void logout(int, char*);

/*variable declarations*/
int i,max_client = 30;int clientsock,peersock[100],peeraccept,addrlen,addrlen1;int globe = 0;
char host[256];fd_set readfds,readfds1;
int majorflag = 0;
char choi[100],data[100];char key[100][100];
struct sockaddr_in peerserver,peeraddress,temp[256];struct hostent *hp1;int maxpeer;int tempsock[100];
struct sockaddr_in client,address;char buffernew[200];
int maxsock;
int globing;

struct sigaction sas;

/*handling SIGNINT*/
void sig_int(int signo)
{
	for(i=0;i<100;i++)
	{
		if(peersock[i]>0)
		{//fprintf(stderr,"it came here");
			close(peersock[i]);
			peersock[i] = -1;
		}
	}
 	close(peeraccept);
	peeraccept  = -1;
	close(clientsock);
	clientsock = -1;
	exit(0);                             	
}

int main(int args,char *argv[])
{
	int t,k,r;
	struct hostent *hp;
	struct sockaddr_in server;
	char line[250],buffer[1024],data[1024];
	char *input1;	

	FILE *f1;
	f1 = fopen(argv[1],"r");
	if(f1<0)
	{
		perror("Unable to open the file");
	}	
	k = 0;

	/*storing the server address and port number to structure config*/
	while(fgets(line,sizeof(line),f1))
	{
		t=0;
		input1 = strtok(line,":");
		while(input1 != NULL)
		{
			if((t==1)&&(k==0))
			{
				input1 = strtok(input1,"\n");
				strcpy(config.servhost,input1);
			}
			else if((t==1)&&(k==1))
			{ 
				strcpy(config.port,input1);
				config.servport = atoi(config.port);
			}
				input1 = strtok(NULL,":");
			t++;
		}
		k++;
	}
	
	/* socket creation and connection to server*/
	clientsock = socket(AF_INET,SOCK_STREAM,0);
	usemessage.csock = clientsock;

	if(clientsock<0)
	{
		perror("Unable to create the client socket");
	}
	server.sin_family = AF_INET;
	hp = gethostbyname(config.servhost);
	if(hp == 0)
	{
		perror("Could not get the server details");
	}

	server.sin_port = htons(config.servport);
	memcpy(&server.sin_addr,hp->h_addr,hp->h_length);
	
	if((connect(clientsock,(struct sockaddr *)&server,sizeof(server)))<0)
	{
		perror("Could not establish connection with server");
		close(clientsock);
		exit(1);	
	}

	for(i=0;i<100;i++)
	{
		tempsock[i] = -1;
	}
	for(i=0;i<100;i++)
        {
                peersock[i] = -1;
        }

		peeraccept=-1;maxsock = 0;
		menu();
	do
	{
		/* select in client side to deal with multiple clients*/
		sas.sa_handler = &sig_int;
		sigaction(SIGINT,&sas,NULL);		
		memset(&buffer,'0',sizeof(buffer));
		memset(&data,'0',sizeof(data));
		memset(&buffernew,'0',sizeof(buffernew));

		FD_ZERO(&readfds);
		FD_SET(0,&readfds);
		maxsock = clientsock;
		FD_SET(clientsock,&readfds);
		for ( i=0;i<100;i++)
          	{
                if(tempsock[i]>=0)
                {
                FD_SET(tempsock[i],&readfds);
                }                      
                if(tempsock[i]>maxsock)
                {                            
                maxsock = tempsock[i];
                }
          	} 

		for ( i=0;i<100;i++)
                {
                if(peersock[i]>=0)
                {
                FD_SET(peersock[i],&readfds);
                }                      
                if(peersock[i]>maxsock)
                {                            
                maxsock = peersock[i];
                }
                } 

		FD_SET(peeraccept,&readfds);

		if(clientsock<0)
		{
			maxsock = 0;
		}
		 if(peeraccept>maxsock)
                         maxsock = peeraccept;

		select(maxsock+1,&readfds,NULL,NULL,NULL);
		
		/*if the standard input is set*/
		if(FD_ISSET(0,&readfds))
		{
			fgets(buffer,sizeof(buffer),stdin);
			strtok(buffer,"\n");		
			if(((strcmp(buffer,"r")==0)||(strcmp(buffer,"l")==0))||(strcmp(buffer,"peerdetails")==0))	
				send_message(clientsock,buffer,NULL,NULL);
			else
			{
				r = peer(buffer);
			}
		}
		/*if the client socket connected to the server is set*/
		if(FD_ISSET(clientsock,&readfds))
		{
			receive_message(clientsock,data);
		}
		
		/*the socket(when the client act as peer) is accepting connections is set*/
		if(FD_ISSET(peeraccept,&readfds))
                {
			accepting();
                }

		/*the socket(when the client act as peer) is receiving  messages is set*/
		for(i=0;i<100;i++)
		{
		if(tempsock[i]>=0)
		{
		if(FD_ISSET(tempsock[i],&readfds))
                {
			receive(tempsock[i]);
			break;
		}
		}
		}

		/*the socket(when the client act as peer) is requesting connections is set*/
		for(i=0;i<100;i++)
                {
                if(peersock[i]>=0)
                {
                if(FD_ISSET(peersock[i],&readfds))
                {
                        receivepeer(peersock[i]);
			break;
                }
                }
                }  
	}
	while(1);
			
	return 0;
}

/*deals with sending messages to the server to transfer to other clients*/
void send_message(int clientsock,char *buffer,char *key1,char *key2)
{
	int sendmessage;
	if(strcmp(buffer,"peerdetails")==0)
	{
		sendmessage = send(clientsock,(struct usemessage *)&usemessage,sizeof(usemessage),0);
	}
	else if(strcmp(buffer,"sendinvite")==0)			//sending the invite
	{
		recmessage.invitacpt = 0;
		strcpy(invite.username,key1);
		strcpy(invite.message,key2);
		strcpy(invite.contact[0].uname,usemessage.username);
		strcpy(invite.contact[0].servhost,usemessage.servhost);
		invite.contact[0].servport = usemessage.servport;
		invite.status = 3;
		sendmessage = send(clientsock,(struct usemessage *)&invite,sizeof(usemessage),0);
	}
	
	else if(strcmp(buffer,"acceptinvite")==0)		//accepting the invite
        {
		recmessage.invitacpt = 1;
                strcpy(recmessage.message,key2);
                recmessage.status =3;
                strcpy(recmessage.servhost,usemessage.servhost);
                recmessage.servport = usemessage.servport; 
                sendmessage = send(clientsock,(struct usemessage *)&recmessage,sizeof(usemessage),0);
		if(sendmessage < 0)
		{
			perror("Error in sending");
		}
		else
		{
		}
	}
	
	else if(strcmp(buffer,"logout")==0)			//sending logout information
	{
		strcpy(usemessage.message,"logout");
		sendmessage =  send(clientsock,(struct usemessage *)&usemessage,sizeof(usemessage),0);
		strcpy(usemessage.message,"");
	}
	else
	{
	if((strcmp(buffer,"r")==0)||(strcmp(buffer,"l")==0))	//registering /logging in of the user
	{
		fprintf(stderr,"Please enter the username\n");
		scanf("%s",(char *)&usemessage.username);
		strtok(usemessage.username,"\n");
		fprintf(stderr,"Please enter the password\n");
		scanf("%s",(char *)&usemessage.password);
		strtok(usemessage.password,"\n");
		strcpy(&usemessage.choice,buffer);
		sendmessage = send(clientsock,(struct usemessage *)&usemessage,sizeof(usemessage),0);
		if(sendmessage<0)
		{
			if(errno == EINTR)
			//	continue;
			{}
			else
			{
				perror("Unable to send data");
				exit(1);
			}
		}
	}
	else
	{
		fprintf(stderr,"Wrong entry. Please try again\n");
	}
	}
}

/*receiving the messages directed to the server by other clients*/
void receive_message(int clientsock,char *buffer)
{
	int receivemessage,ind;
	if((receivemessage = recv(clientsock,(struct usermessage *)&recmessage,sizeof(recmessage),0))<0)
	{
		if(errno == EINTR)
		{}
	//		continue;
		else
		{
			perror("Error in receiving");
			exit(1);
		}
	}
	else if(receivemessage == 0)		//when the server is  disconnected, closing the client*/
	{
		fprintf(stderr,"The server got disconnected closing the client\n");
		close(clientsock);
		clientsock = -1;
		exit(0);		
	}
	else
	{
	usemessage.option = recmessage.option;
	if(strcmp(recmessage.message,"logout")==0)	// receiving the logout information of some of the friends
	{
		 for(i=0;i<100;i++)   
                 {
                        if((strcmp(recmessage.username,usemessage.contact[i].uname)==0))//&&(usemessage.contact[i].psock<0))
                        {
                            strcpy(usemessage.contact[i].uname,"");                                
                            strcpy(usemessage.contact[i].servhost,"");
                            usemessage.contact[i].servport = 0;
                            usemessage.contact[i].ct = 0;
                            usemessage.contact[i].ct1 = 0;
                            usemessage.contact[i].fla = 0;
                            usemessage.contact[i].globe = 0;
			    fprintf(stderr,"Online friends updated due to a user logged out\n");
                            display(usemessage);           

                            break;

                        }
		}
	}

	else if(strcmp(recmessage.message,"exit")==0)		//receiving the exit message of a user exit
	{
		 for(i=0;i<100;i++)   
                 {
                        if(strcmp(recmessage.username,usemessage.contact[i].uname)==0)
                        {                       
                            strcpy(usemessage.contact[i].uname,""); 
			    strcpy(usemessage.contact[i].servhost,"");
			    usemessage.contact[i].servport = 0;
                            usemessage.contact[i].ct = 0;
                            usemessage.contact[i].ct1 = 0;
                            usemessage.contact[i].fla = 0;
                            usemessage.contact[i].globe = 0;
                            fprintf(stderr,"Online friends updated due to user exit\n");
                            display(usemessage);           
                            break;

                        }
                }

	}

	else if((recmessage.option ==55)&&(recmessage.status == 500))		//when registeration error
	{
		fprintf(stderr,"%s and exit code %d\n",recmessage.message,recmessage.status);
		fprintf(stderr,"Please enter  r to register the client with the server\n");
		scanf("%s",buffer);
		send_message(clientsock,buffer,NULL,NULL);
	}
	else if((strcmp(recmessage.message,"The user has been successfully registered")==0)&&(recmessage.status == 200))	//successfully registers
        {
		fprintf(stderr,"\n");
		fprintf(stderr,"%s and return code %d\n",recmessage.message,recmessage.status);
		strcpy(&usemessage.choice,"");
		load(recmessage); 
		newconnection(clientsock,buffer,ind);
        }
	else if((recmessage.option = 66)&&(recmessage.status == 500))		//login error
        {
		fprintf(stderr,"%s and exit code %d\n",recmessage.message,recmessage.status);
		fprintf(stderr,"Please enter l to login the client with the server\n");
		scanf("%s",buffer);
                send_message(clientsock,buffer,NULL,NULL);
        }
	else if((strcmp(recmessage.message,"The user has successfulyy logged in")==0)&&(recmessage.status == 200))		//successfully logged in
        {
		fprintf(stderr,"\n");
                fprintf(stderr,"%s and return code %d\n",recmessage.message,recmessage.status);
		load(recmessage); 
		strcpy(&usemessage.choice,"");
		newconnection(clientsock,buffer,ind);
        }
	else if(usemessage.option == 1)			//updating the friend details due to new login from the current client
	{
		fprintf(stderr,"\n");
		load(recmessage);
		fprintf(stderr,"Online friends updated on login:\n");
		for(i=0;i<100;i++)
		{
			if((strcmp(recmessage.contact[i].uname,usemessage.contact[i].uname)==0)&&(peeraccept>=0))//&&(strcmp(recmessage.contact[i].servhost,"")!=0))
			{
				strcpy(usemessage.contact[i].servhost,recmessage.contact[i].servhost);
				usemessage.contact[i].servport = recmessage.contact[i].servport;
			}
		}
		display(usemessage);
	}
	else if(usemessage.option == 2)			//updating the friends due to an event happend to their friend
        {
		fprintf(stderr,"\n");
          	fprintf(stderr,"Online friends updated as a user logged in:\n");               
                for(i=0;i<100;i++)
                {
				if((strcmp(usemessage.contact[i].uname,"")==0)&&(peeraccept>=0))
				{
                                strcpy(usemessage.contact[i].uname,recmessage.username);
                                strcpy(usemessage.contact[i].servhost,recmessage.servhost);
                                usemessage.contact[i].servport = recmessage.servport;
				break;
				}
                }display(usemessage);
        }
	else if(recmessage.status == 3)		//receiving and accepting the chat invitation
	{
		if(recmessage.invitacpt==0)
		{
			fprintf(stderr,"Chat inivitation has been sent from %s . Type ia to accept the invitation.\n",recmessage.contact[0].uname);
		}
		else if(recmessage.invitacpt == 1)
		{
                	fprintf(stderr,"Chat inivitation has been accepted from %s .You can now connect with your new friend.\n",recmessage.username); 
			for(i=0;i<100;i++)
                	{
                        if(strcmp(usemessage.contact[i].uname,"")==0)
                        {
                                strcpy(usemessage.contact[i].uname,recmessage.username);				
                                strcpy(usemessage.contact[i].servhost,recmessage.servhost);
                                usemessage.contact[i].servport = recmessage.servport;
				display(usemessage);
				break;
                        }
                }
		}
		else if(recmessage.invitacpt == 2)
		{
			for(i=0;i<100;i++)   
                	{
                        if(strcmp(usemessage.contact[i].uname,"")==0)
                        {
                                strcpy(usemessage.contact[i].uname,recmessage.username);                                
                                strcpy(usemessage.contact[i].servhost,recmessage.servhost);
                                usemessage.contact[i].servport = recmessage.servport;
                	   	display(usemessage);    
		            	break;
                        }
                	}

		}
	}
	}
}

/* peer to peer part of the client start*/
void newconnection(int clientsock,char *buffer,int index)
{
	/*socket creation and binding happens here*/
	peeraccept = socket(AF_INET,SOCK_STREAM,0);
	if(peeraccept<0)
        {
                perror("Unable to create the peer socket");
        }

	client.sin_family       = AF_INET;
        client.sin_addr.s_addr  = INADDR_ANY;
        client.sin_port         = 0;
        addrlen1                = sizeof(client);

	if(bind(peeraccept,(struct sockaddr *)&client,sizeof(client))<0)
        {
                perror("Unable to bind the address and port number to the socket");
        }
        if((getsockname(peeraccept,(struct sockaddr*)&client,&addrlen1))==-1)
        {
                perror("Get sockname failed");
        }
        if((gethostname(host,addrlen1))==-1)
        {
                perror("Unable to get the hostname");
        }
                fprintf(stderr,"Peer has been binded with address %s ,port: %d\n",host,ntohs(client.sin_port));

	strcpy(buffer,"peerdetails");
	strcpy(usemessage.servhost,host);
	usemessage.servport = ntohs(client.sin_port);
	listen(peeraccept,30);
	FD_CLR(peeraccept,&readfds);
	send_message(clientsock,buffer,NULL,NULL);
}
	
int peer(char *buffer)
{
	/*sending the message to other clients, in the prcess establishes the connnection and sends*/
	char *mes1,*mes2;
	char words[100][100];char key[100][100];char tempi[100];
	int count=0,count1=0;
	int sendmsg;
	scanf("%99[^\n]",buffernew); 
	mes1 = strtok(buffernew," ");
	while(mes1 != NULL)
	{
		strcpy(words[count],mes1);
		mes1 = strtok(NULL," ");
		count++;	
	}
		for(i=0;i<count;i++)
		{
		mes2 = strtok(words[i],"_");
	
		while(mes2!=NULL)
		{
			strcpy(key[count1],mes2);
			mes2 = strtok(NULL,"_");
			count1++;
		}
		}	

	for(i = 0;i<100;i++)
        {
        	if(strcmp(usemessage.contact[i].uname,key[2])==0)
                {
                        usemessage.contact[i].globe++;
                        if((usemessage.contact[i].globe == 1)&&(usemessage.contact[i].fla<1))
                        {       
                                peersock[i] = socket(AF_INET,SOCK_STREAM,0);
                                usemessage.contact[i].psock = peersock[i];                                                    
                        }
                }
        }	
	if(strcmp(key[0],"m")==0)			// when the input starts with m
	{
		for(i = 0;i<100;i++)
		{
			if(strcmp(usemessage.contact[i].uname,key[2])==0)
			{
				usemessage.contact[i].ct++;
				peerserver.sin_family = AF_INET;
				hp1 = gethostbyname(usemessage.contact[i].servhost);
				peerserver.sin_port = htons(usemessage.contact[i].servport);
				memcpy(&peerserver.sin_addr,hp1->h_addr,hp1->h_length);
				if(usemessage.contact[i].ct == 1)
				{
					if((connect(usemessage.contact[i].psock,(struct sockaddr *)&peerserver,sizeof(peerserver)))<0)
					{
						if(errno == EINTR)
							continue;
						else
						{
							perror("Unable to stablish connection to peer");
							exit(1);
						}
					}
				}	
				strcpy(mpkt.sender,usemessage.username);
				strcpy(mpkt.message,key[3]);
				strcpy(mpkt.receiver,usemessage.contact[i].uname);
						
				if(usemessage.contact[i].fla == 1)
				{
					if((sendmsg = send(usemessage.contact[i].psock,(struct messagepakt*)&mpkt,sizeof(mpkt),0))<0)
					{
						if(errno == EINTR)
							continue;
						else
						{
							perror("Error in sending");
							exit(1);
						}
					}
				}
					
				else if(usemessage.contact[i].fla == 0)
				{
					if((sendmsg = send(peersock[i],(struct messagepakt*)&mpkt,sizeof(mpkt),0))<0)
					{
						if(errno == EINTR)
							continue;
						else
						{
							perror("Error in sending");
							exit(1);
						}
					}
				}					
				break;
				}	
			}
		}
		else if(strcmp(key[0],"i")==0)		//when the input is for invitation
        	{
			strcpy(buffer,"sendinvite");
			send_message(clientsock,buffer,key[3],key[4]);
                }
		else if(strcmp(key[0],"ia")==0)		//when the input is for accepting invitaion
                {
                        strcpy(buffer,"acceptinvite");
                        send_message(clientsock,buffer,key[2],key[3]);                  
                }
		else if(strcmp(key[0],"logout")==0)	//when the user logs out
		{
			logout(usemessage.csock,usemessage.username);
		}
		else if(strcmp(key[0],"exit")==0)	//when the user gives exit
		{
			close(peeraccept);
			peeraccept  = -1;
			for(i=0;i<100;i++)
                	{
                        close(usemessage.contact[i].psock);
			usemessage.contact[i].psock = -1;
                	}
                        exit(0);
		}
		majorflag = 1;
}

void load(struct usermessage recmessage)		//loading the information of the user to the structure
{
	for(i = 0;i<100;i++)
	{
		if(strcmp(recmessage.contact[i].uname,"")!=0)
		{
			strcpy(usemessage.contact[i].uname,recmessage.contact[i].uname);
			strcpy(usemessage.contact[i].servhost,recmessage.contact[i].servhost);
			usemessage.contact[i].servport = recmessage.contact[i].servport;
		}
	}
}

void display(struct usermessage usemessage)		//displaying the friends information
{        
	int cout = 0;
	for(i=0;i<max_client;i++)
        {
                if(usemessage.contact[i].servport!=0)
                {
                        fprintf(stderr,"%s\t",usemessage.contact[i].uname);
                        fprintf(stderr,"%s\t",usemessage.contact[i].servhost);
                        fprintf(stderr,"%d\n",usemessage.contact[i].servport);
                        fprintf(stderr,"\n");
			cout = 1;
		}
	}	
	if(cout == 0)
		fprintf(stderr,"No online friends to display\n");
}

void menu()						//displaying the menu for login, register , exit , logout
{
	fprintf(stderr,"\n");
        fprintf(stderr,"  Please select one of the following options\n"); 
        fprintf(stderr,"* Enter r to register the client with the server\n");
        fprintf(stderr,"* Enter l to login the client with the server\n");
        fprintf(stderr,"* After logging in, you can close your application by typing exit\n");
        fprintf(stderr,"* After logging in you can type logout to log you out and return to this menu page\n");
}

void accepting()					//accepting the connection from the peer
{
	for(i=0;i<100;i++)
	{
        if(strcmp(usemessage.contact[i].uname,key[1])==0)
	{
		usemessage.contact[i].ct1++;
		if(usemessage.contact[i].ct1 == 1)
		{
        	if((tempsock[i] = accept(peeraccept,(struct sockaddr *)&peeraddress,(socklen_t *)&addrlen1))<0)
                {
			if(errno==EINTR)
				continue;
			else
			{
	                	perror("Failed to accept");
        	                exit(1);
                	}
		}
                else
                {
		       globing = i;
                       temp[i] = peeraddress;
                       fprintf(stderr,"Established connection to the peer, socket fd is :%d, address is :%s port is: %d\n",tempsock[i],
                       inet_ntoa(peeraddress.sin_addr),ntohs(peeraddress.sin_port));
		       majorflag = 0;
		       break;
                }
		}
 	}
	else 
		continue;
	}
}

void receive(int tempsocket)				//receive the messages from the peer(this act as the server side of the client)*/
{
	int recvmsg;
        if((recvmsg = recv(tempsock[globing],(struct messagepakt*)&mpkt,sizeof(mpkt),0))<0)
	{
		if(errno == EINTR)
		{}
		//	continue;
		else
		{
			perror("Error in receiving1");
			exit(1);
		}
	}
	else if(recvmsg == 0)
	{
		for(i=0;i<100;i++)
                {
                        if(usemessage.contact[i].psock == tempsocket)
                        {
		             fprintf(stderr,"The user %s  got disconnected\n",usemessage.contact[i].uname);

                                strcpy(usemessage.contact[i].uname,"");                         
                                strcpy(usemessage.contact[i].servhost,"");
                                usemessage.contact[i].servport = 0;
                                usemessage.contact[i].ct = 0;
                                usemessage.contact[i].ct1 = 0;
                                usemessage.contact[i].fla = 0;
                                usemessage.contact[i].globe = 0;
				fprintf(stderr,"Online friends updated due to user disconnection at receiver side\n");
				close(tempsocket);
                                usemessage.contact[i].psock = -1;
				tempsock[globing]  = -1;//2
				tempsocket = -1;
				display(usemessage);
				break;
                        }
             	}
	}
	else
	{
	fprintf(stderr,"friend_%s >> %s \n",mpkt.sender,mpkt.message);                    
	for(i=0;i<100;i++)
	{
		 if(strcmp(usemessage.contact[i].uname,mpkt.sender)==0)
        	 {
			usemessage.contact[i].psock = tempsock[globing];
			usemessage.contact[i].fla = 1;
			usemessage.contact[i].ct = 1;
			break;
		 }
	}
	}
}

void receivepeer(int tempsocket)			/*receiving the message from the peer.this act as the client side of the peer*/
{
        int recvmsg;
        if((recvmsg = recv(tempsocket,(struct messagepakt*)&mpkt,sizeof(mpkt),0))<0)
	{
		if(errno == EINTR)
		{}
		//	continue;
		else
		{
			perror("Error receive2");
			exit(1);
		}
	}
	else if(recvmsg == 0)
	{
		for(i=0;i<100;i++)
		{
			if(usemessage.contact[i].psock == tempsocket)
			{
				fprintf(stderr,"The user %s got disconnected\n",usemessage.contact[i].uname);
				strcpy(usemessage.contact[i].uname,"");				
				strcpy(usemessage.contact[i].servhost,"");
				usemessage.contact[i].servport = 0;
				usemessage.contact[i].ct = 0;
				usemessage.contact[i].ct1 = 0;
				usemessage.contact[i].fla = 0;
				usemessage.contact[i].globe = 0;	
				close(tempsocket);
				usemessage.contact[i].psock = -1;
				peersock[i]  = -1;
				break;
			}
		}
	}
	else
	{
        fprintf(stderr,"friend_%s >> %s \n",mpkt.sender,mpkt.message);                    

 	for(i=0;i<100;i++)
        {
                 if(strcmp(usemessage.contact[i].uname,mpkt.sender)==0)
                 {
                        usemessage.contact[i].fla = 0;
                        usemessage.contact[i].ct = 1;
                        break;
                 }
        }
	}
}

void logout(int csk, char *usname)			/*logout function of the client*/
{
	int y ;
	usemessage.ct = 0;
	usemessage.fla = 0;
	fprintf(stderr,"User has been logged out\n");
	for(i=0;i<100;i++)
	{
			globing = 0;
			strcpy(usemessage.contact[i].uname,"");
			usemessage.contact[i].servport = 0;
			usemessage.contact[i].globe = 0;
			usemessage.contact[i].ct = 0;
			usemessage.contact[i].ct1 = 0;
			usemessage.contact[i].fla = 0;
			usemessage.contact[i].psock = -1;
			y = usemessage.contact[i].psock;
	}
			for(i=0;i<100;i++)
			{
				if(tempsock[i]>=0)
				{close(tempsock[i]);tempsock[i] = -1;}
				if(peersock[i]>=0)
				{close(peersock[i]);peersock[i] = -1;}
			}
			close(peeraccept);
			peeraccept = -1;
	send_message(clientsock,"logout",NULL,NULL);
	close(y);
	menu();
	fgets(buffernew,sizeof(buffernew),stdin);
}

void empty(struct usermessage usemessage)       /*emptying the structure*/
{
        strcpy(&usemessage.choice,"");
        strcpy(usemessage.word,"");
        strcpy(usemessage.username,"");
        strcpy(usemessage.password,"");
        strcpy(usemessage.message,"");
        strcpy(usemessage.servhost,"");
        usemessage.servport = 0;
        usemessage.status = 0;
        usemessage.ct = 0;
        usemessage.fla = 0;
        usemessage.invitacpt = 0;
	usemessage.option = 0;
	for(i=0;i<100;i++)
	{
		if(usemessage.contact[i].psock>=0)
		{
			strcpy(usemessage.contact[i].uname,"");		
			strcpy(usemessage.contact[i].servhost,"");		
			usemessage.contact[i].ct = 0;
			usemessage.contact[i].ct1 = 0;
			usemessage.contact[i].fla = 0;
			usemessage.contact[i].globe = 0;
		}
	}
}


