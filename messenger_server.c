
#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/select.h>
#include<netinet/in.h>
#include<netdb.h>
#include<errno.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<signal.h>
#include<pthread.h>

pthread_t counter[100];

/*structure to store the user data*/
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

/*structure to send a particular users information*/
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
	struct contactl		// friend details of a particular user	
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
}usemessage,recmessage;

struct configtable
{
	char portt[100];
	int port;
}config;

/*function declarations*/
void send_message(int [], char *,int,struct usermessage usemessage);
void *receivemessage(void*);
void display(int [],char *);
void empty(struct usermessage usemessage);
void logout_msg(int [],int,struct usermessage usemessage);

/*variable declarations*/
int flag = 0,i,l,j,k,p,t,globecount;
int max_client = 30;int sendmessage,sock;
int clientsock[30];
fd_set readfds;
struct sockaddr_in server,address;

/*handling SIGINT*/
struct sigaction sas;
void sig_int(int signo)
{
        close(sock);
	for(i=0;i<30;i++)
	{
		close(clientsock[i]);
	}
	exit(0);                      
}

char buffer[1024];char *argum;char * inpt,lin1[256];
int main(int args,char *argv[])
{
	//int sock;
	int max_sock,count,addrlen,j,k;
	char host[256];int *tsock;
	char *input1,*input2, *input3;
	char line[256],line1[256];
	
	FILE *f1,*f2;
	f1=fopen(argv[1],"r");
	if(f1<0)
	{	
		perror("Unable to open the file");
	}	
	argum =argv[1];
	f2=fopen(argv[2],"r");
	if(f2<0)		
	{	
		perror("Unable to open the file");
	}
	t=0;

	/* reading the contents from user_info_file to  structure array user[]*/

	while(fgets(line,sizeof(line),f1))
	{j=0;
		input1 = strtok(line,"|");
		while(input1!= NULL)
		{
			if(j == 0)
			{
				strcpy(user[t].username,input1);
				fprintf(stderr,"%s\t",user[t].username);
			}
			else if(j == 1)
			{
				strcpy(user[t].password,input1);
				fprintf(stderr,"%s\t",user[t].password);
			}
			else if(j==2)
			{
				input2 = strtok(input1,";");
				k = 0;
				while(input2 != NULL)
				{
					strcpy(user[t].contact[k].uname,input2);
					fprintf(stderr,"%s\t",user[t].contact[k].uname);
					input2 = strtok(NULL,";");
					k++;
				}	
				fprintf(stderr,"\n");
			}
		input1 =  strtok(NULL,"|");
		j++;
		}
	t++;
	}

	/* reading the contents from configuration_file to structure config */

	while(fgets(line,sizeof(line),f2))
	{
		count = 0;
		input3 = strtok(line,":");
		while(input3 != NULL)
		{
			if(count == 1)
			{
				strcpy(config.portt,input3);
				config.port = atoi(config.portt);
			}
			input3 = strtok(NULL,":");
			count ++;
		}
	}
			
	sock = socket(AF_INET,SOCK_STREAM,0);
	server.sin_family 	= AF_INET;
	server.sin_addr.s_addr  = INADDR_ANY;
	server.sin_port		= htons(config.port);
	addrlen			= sizeof(server);

	if(sock<0)						//socket creation
	{
		perror("Failed to create a socket");
		exit(1);
	} 	
	if(bind(sock,(struct sockaddr *)&server,sizeof(server))<0)			//binding the socket
	{
		perror("Unable to bind the address and port number to the socket");
	}
	if((getsockname(sock,(struct sockaddr*)&server,&addrlen))==-1)
	{
		perror("Get sockname failed");
		return -1;
	}
	if((gethostname(host,addrlen))==-1)
	{
		perror("Unable to get the hostname");
		return -1;
	}	
		fprintf(stderr,"Server has been binded with address %s ,port: %d\n",host,ntohs(server.sin_port));
	
	for(i=0;i<max_client;i++)
	{
		clientsock[i] = -1;
	}
		
	listen(sock,30);

	 i = 0;
		while(1)
		{
			int newsock;				
			if(i == 100)
				i = 0;
			if((newsock=accept(sock,(struct sockaddr *)&address,(socklen_t *)&addrlen))<0)	//accepting client connection
			{
				if(errno == EINTR)
					continue;
				else
				{
					perror("Failed to accept the client connection");
					exit(1);
				}
			}
			if(newsock>0)
			{
				tsock = malloc(1);
				printf("Established connection to client,socket_fd is :%d, address is:%s port is: %d\n",newsock,
				inet_ntoa(address.sin_addr),ntohs(address.sin_port));
				*tsock = newsock;
				if(pthread_create(&counter[i],NULL,receivemessage,(void *)tsock)<0)	//creating thread for each client
				{
					perror("Could not create the thread");
					return EXIT_FAILURE;
				}
				else
				{
					i++;
				}
			}			
		}

	return 0;
}

/*sending the message to the clients*/
void send_message(int clientsock[], char *buffer,int indix, struct usermessage usemessage)
{
	int sendmessage;
	if(strcmp(buffer,"frienddetails")==0)
	{
		user[p].option = 1;
		usemessage.option = 1;
		if(strcmp(user[p].servhost,"")!=0)	
		{				
		if((sendmessage = send(user[p].csock,(struct usemessage*)&usemessage,sizeof(usemessage),0))<0)	//sending to a particular client
		{
			if(errno == EINTR)
                	{	
//				continue;
  			}
	              	else
                	{
			}
		}
		}
		strcpy(buffer,"");
		for(i=0;i<100;i++)
		{
			usemessage.option = 0;
			for(l=0;l<100;l++)
			{
			if((strcmp(user[p].contact[i].uname,user[l].username)==0)&&(strcmp(user[l].username,"")!=0)) //sending to the friends of the client in the thread
			{
				user[l].option = 2;	
				usemessage.option = 2;
				if((sendmessage = send(user[l].csock,(struct usemessage*)&usemessage,sizeof(usemessage),0))<0)
				{
					if(errno == EINTR)
                			{	}          
		//                              continue;
                                        else
                                        {
					}
				}
				else
					break;
			}
			}
		}	
			strcpy(usemessage.message,"");
	}
	else if((flag == 1)||(flag == 0))
	{
		if((sendmessage = send(clientsock[indix],(struct usemessage*)&usemessage,sizeof(usemessage),0))<0)
		{
			  if(errno == EINTR)
                          {	}  
			//	continue;
                          else
                          {  
                          }
		}
	}
}

/*receiving the message from the clients*/ 
 
 void *receivemessage(void *ssock)
{
	struct sockaddr_in temporary;
	temporary = address;
	int index,ind;
	while(1)
	{
		for(i=0;i<100;i++)
		{
			if(clientsock[i]==-1)
			{clientsock[i] = *(int*)ssock;break;}
		}
		pthread_detach(pthread_self());	
		for(i=0;i<100;i++)
		{
			if((clientsock[i]>=0)&&(clientsock[i]==*(int*)ssock))
			{index = i;break;}
		}
	ind = i;
	flag = 0;
	int receivemessage;
	empty(recmessage);
	receivemessage =  recv(clientsock[index],(struct usemessage*)&recmessage,sizeof(usemessage),0);
	if(receivemessage<0)
	{
		  if(errno == EINTR)
		  {	}
  //                	continue;
                  else
                  { 
			continue; 
//                  	perror("Error");
                  }
	}
	else if(receivemessage == 0)		// if client is disconnected
	{
		fprintf(stderr,"The client which got disconnected is socket : %d , address : %s , port :%d\n",clientsock[index],inet_ntoa(temporary.sin_addr),
		ntohs(temporary.sin_port));	
		int tempi;
		for(i=0;i<100;i++)		//notifying the friends that client is disconnected
		{
			if(user[i].csock==clientsock[index])
			{tempi = i;
				strcpy(user[i].servhost,"");
				user[i].servport = 0;
				for(j=0;j<100;j++)
				{
					for(k=0;k<100;k++)
					{
						if((strcmp(user[i].contact[j].uname,user[k].username)==0)&&(strcmp(user[k].username,"")!=0)&&((user[k].csock)>0))
						{
							strcpy(recmessage.username,user[i].username);
							strcpy(recmessage.message,"exit");
							if(sendmessage = send(user[k].csock,(struct usemessage*)&recmessage,sizeof(usemessage),0)<0)
							{
								if(errno == EINTR)
								{}
                                                        	//	continue;
                                        			else
                                        			{  
                                                			perror("Error in disc");
                                                			exit(1);
                                        			}
							}
						break;
						}
					}		
				}
			user[i].csock = -1;
			display(clientsock,buffer);
			break;
			}
		}

		FILE *fp;
		fp = fopen("user_info_file1","w");		//writing data to file user_info_file1
				for(j = 0;j< 30;j++)
				{
					if((strcmp(user[j].username,"")!=0)&&(strcmp(user[j].username," ")!=0)&&(strcmp(user[j].username,";")!=0))
					{
					fprintf(fp,"%s|%s|",user[j].username,user[j].password);
					for(i = 0; i<30;i++)
					{
						if((strcmp(user[j].contact[i].uname,"")!=0)&&(strcmp(user[j].contact[i].uname," ")!=0)&&(strcmp(user[j].contact[i].uname,";")!=0))
						{
							fprintf(fp,"%s;",user[j].contact[i].uname);
						}
					}
					}fprintf(fp,"\n");
				}
		fclose(fp);
		close(clientsock[index]);
		clientsock[index]=-1;
		pthread_exit(&ssock);
	}
	else
	{
		if(recmessage.choice == 'r')			// for registering the users
		{
			for(p=0;p<t;p++)
			{
				if(strcmp(recmessage.username,user[p].username)==0)
				{
					flag = 1;
					break;
				}	
			}
			if(flag == 1)
			{
				strcpy(recmessage.message,"The selected user name has been occupied");
				recmessage.status = 500;
				recmessage.option = 55;
				send_message(clientsock,buffer,index,recmessage);	
			}
			else
			{
				if(strcmp(user[p].username,"")==0)
				{
					strcpy(user[p].username,recmessage.username);
					strcpy(user[p].password,recmessage.password);
					for(j=0;j<100;j++)
					{
						if(strcmp(user[p].contact[j].uname,"")!=0)
						{
							strcpy(recmessage.contact[j].uname,user[p].contact[j].uname);
							strcpy(recmessage.contact[j].servhost,user[p].contact[j].servhost);
							recmessage.contact[j].servport = user[p].contact[j].servport;
						}
					}
					user[p].csock = clientsock[ind];
					strcpy(recmessage.message,"The user has been successfully registered");
					recmessage.status = 200;
					t = p+1;
					globecount = p;
					empty(usemessage);
					send_message(clientsock,buffer,index,recmessage);
				}
			} 
		}
		else if(recmessage.choice == 'l')		//for logging  in the users
		{
			for(p=0;p<20;p++)
                        {
                                if((strcmp(recmessage.username,user[p].username)==0)&&(strcmp(recmessage.password,user[p].password)==0))
                                {
					globecount = p;
                                        flag = 1;
                                        break;
                                }       
                        }
                        if(flag == 1)   
                        {
				user[globecount].csock = clientsock[ind];		
                                strcpy(recmessage.message,"The user has successfulyy logged in");
                                recmessage.status = 200;
				send_message(clientsock,buffer,ind,recmessage); 
                        }
                        else
                        {
                                strcpy(recmessage.message,"Username and passwords do not match.Please send again");
				recmessage.status = 500;
				recmessage.option = 66;
				send_message(clientsock,buffer,index,recmessage);
			}
		}
		else if(recmessage.status == 3)			// for user invites
		{
			if(recmessage.invitacpt == 1)		// if it is an invite/invite accept..if accept notify the sender, update friends list
			{
				for(i=0;i<100;i++)
                                {
                                	if(strcmp(user[i].username,recmessage.contact[0].uname) == 0)
                                        {
                                        	for(j=0;j<100;j++)
                                                {
                                                	if(strcmp(user[i].contact[j].uname,"")==0)
                                                        {
                                                        	strcpy(user[i].contact[j].uname,recmessage.username);
								strcpy(user[i].contact[j].servhost,recmessage.servhost);
								user[i].contact[j].servport = recmessage.servport;		
								break;
                                                        }
                                                }
                                        	if((sendmessage = send(user[i].csock,(struct usemessage*)&recmessage,sizeof(usemessage),0))<0)
						{	
							if(errno == EINTR)
							{}
//                                                        		continue;
                                        		else
                                        		{  
                                                		perror("Error send invite");
                                                		exit(1);
                                        		}
						}
						else
							break;
						}
                                      }	
				for(i=0;i<100;i++)
                                {
                                       if(strcmp(user[i].username,recmessage.username) == 0)        
                                       {       
                                               for(j=0;j<100;j++)
                                               {
                                                       if(strcmp(user[i].contact[j].uname,"")==0)
                                                       {
                                                                strcpy(user[i].contact[j].uname,recmessage.contact[0].uname);
                                                                strcpy(user[i].contact[j].servhost,recmessage.contact[0].servhost);
                                                                user[i].contact[j].servport = recmessage.contact[0].servport;
                                                                strcpy(recmessage.username,recmessage.contact[0].uname);
                                                                strcpy(recmessage.servhost,recmessage.contact[0].servhost);
                                                                recmessage.servport = recmessage.contact[0].servport;
                                                                recmessage.invitacpt = 2;
                                                                if((sendmessage = send(user[i].csock,(struct usemessage*)&recmessage,sizeof(usemessage),0))<0)
                                                              	{
									if(errno == EINTR)
									{}
                                                        				//	continue;
                                        				else
                                        				{  
                                                				perror("Error send accept invite");
                                                				exit(1);
                                        				}
								}
								else
									break;
                                                        }
                                                }
						break;       
                                        }
                               }
			}
			else
			{
				for(i=0;i<100;i++)
                        	{
                                	if(strcmp(recmessage.username,user[i].username)==0)
                                	{
                                        	if((sendmessage = send(user[i].csock,(struct usemessage *)&recmessage,sizeof(usemessage),0))<0)
						{
							if(errno == EINTR)
							{}
                                                                        //	continue;
                                                        else
                                                        {  
                                                        	perror("Error send invites");
                                                                exit(1);
                                                        }
						}
						else
                                			break;
					}
                        	}
			}		
		}
		else if((strcmp(recmessage.servhost,"")!=0)&&(strcmp(recmessage.message,"logout")!=0)) //getting the host and portnumber of the clients
		{
			strcpy(user[p].servhost,recmessage.servhost);
			user[p].servport = recmessage.servport;
			for(j=0;j<100;j++)
                        {
			      for(l=0;l<100;l++)
			      {
                              if((strcmp(user[p].contact[j].uname,user[l].username)==0)&&(user[l].csock>0))
                              {
                                    strcpy(recmessage.contact[j].uname,user[l].username);
                                    strcpy(recmessage.contact[j].servhost,user[l].servhost);
                                    recmessage.contact[j].servport = user[l].servport;
			      }
			      }
                        }
			display(clientsock,buffer);
			strcpy(buffer,"frienddetails");
			send_message(clientsock,buffer,index,recmessage);
		}
		else if(strcmp(recmessage.message,"logout")==0)		//if the user has given logout, notify the friends
		{
			for(i=0;i<100;i++)
			{
				if(strcmp(user[i].username,recmessage.username)==0)
				{
					strcpy(user[i].servhost,"");
					user[i].servport = 0;
					for(j=0;j<100;j++)
					{
						strcpy(user[i].contact[j].servhost,"");
						user[i].contact[j].servport = 0;
					}
				break;
				}
			}
			display(clientsock,buffer);
			logout_msg(clientsock,i,recmessage);
		}
	}
}
}

void display(int clientsock[],char *buffer)			//displaying the users
{
	fprintf(stderr,"The number of users online are:\n");
	for(i=0;i<(max_client)&&(i<=100);i++)
	{
		if(strcmp(user[i].servhost,"")!=0)
		{
			fprintf(stderr,"%s\n",user[i].username);
		}
	}
}

void empty(struct usermessage usemessage)			// empty the structure received
{
	strcpy(usemessage.username,"");
	strcpy(usemessage.password,"");
	strcpy(usemessage.servhost,"");
	usemessage.servport = 0;
	usemessage.status = 0;
	strcpy(usemessage.message,"");
	strcpy(&usemessage.choice,"");
}

void logout_msg(int clientsock[],int i,struct usermessage logmessage)	//logout operation
{
	
	for(i=0;i<100;i++)
        {
		strcpy(logmessage.message,"logout");
		if(strcmp(logmessage.username,user[i].username)==0)
		{
                        for(l=0;l<100;l++)
                        {
			for(k=0;k<100;k++)
			{
                        if((strcmp(user[i].contact[l].uname,user[k].username)==0)&&(strcmp(user[k].username,"")!=0))
                        {
				if(user[k].csock>=0)
				{
                                if((sendmessage = send(user[k].csock,(struct logmessage*)&logmessage,sizeof(logmessage),0))<0)
                                {
                                        if(errno == EINTR)
                                        {       }          
                //                              continue;
                                        else
                                        {
                                        }
                                }
                                else
				{
                                break;
				}
				}
                        }
			}
                        }
		}
	}     
}

