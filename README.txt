*The tar file contains messenger_server.c, messenger_client.c, Makefile, README.txt. Also, to show the format of the input file for which output was coming 
user_info_file,configuration_file and configration_file have been attached with the tar file.

* pthreads have been used in the server side to handle multiple client functions and select has been used in the client side

 Compile:-
* to compile the files messenger_server.c and messenger_client.c, type:- make
* to run the program messenger_server.c, type ./messenger_server.c user_info_file configuration_file
 [ user_info_file is the file containing the user details, and configuration_file contains the portnumber of the server]
* to run the program messenger_client.c, type ./messenger_client.c configration_file
 [ configration_file contains the ipaddress and portnumber of the server]
Note:If running the user_info_file which hasn data in it,please give a semicolon at the end so the output comes out correctly.

* to start the client communications:-

1. type r for registering
    	give username
	give password
	if user name already occupied, follow from step 1 again.
	Once registered, messages in the message format can be given to chat.

2. type l for login
        give username
        give password
        if user name doesn't exist, follow from step 1 again.
        Once logged, messages in the message format can be given to chat.
Note1:- if user has given r, but he couldn't register then he cannot use l command. He has to use r itself. Once logged in, he can use r or l after logout.
Note2:- if a user is registered , then he is automatically logged in.
Note3:- Please make sure to use the same client connection for a particular user, as otherwise it may cause some issues in the output.
Note4:- Once logged in, please give only the commands given below. If error keys are pressed, then program will terminate with error.
3. Once logged in, the online friends will display.
	Commands:-
	
	To send a message
	* m friend_friendname messagecontent_message
	[Replace the friendname with name of the friend and messagecontent with what message to send].
Note: Messages will be displayed in the format friend_friendname>>messagecontent
      Messages cannot accept the spaces.

	To invite a friend
	* i potential_friend_friendname messagecontent_message
	[Replace the friendname with name of the friend and messagecontent with what message to send].

	To accept the invite from a friend
	* ia inviter_friendname messagecontent_message
	[Replace the friendname with name of the friend and messagecontent with what message to send].

Note: while sending an  invite, receiver will receive the invitaion message. The sender will be notified of an accept invite, after invitation accept from the friend.
Then the friends list of both the friends will be updated and shown.

	To logout the user
	* logout
	The user will be logged off and shows the menu page to login. The friends list of the friends wil be updated.
Note:- 
Inorder to have logout working without any errors(if any), please make sure to have a message sent to each of that client's friends so that there is a P2P connection 
established between them.
If there are 4-5 friends, then  log out may cause an issue like updating every friend. As the number of friends are more, logging in after logout may also cause an 
issue. But for 2-3 friends logout and logging afterwards works fine.

	To exit(terminate the client program]
	* exit
	This will terminate the client , updates the friends that user is offline.
Note:- Please make sure exit is given after registering or logging in.

	Cntrl-C
	* If the user gives control-C in the client, SIGINT function will get called, closes the sockets and disconnects the client.At the same times updates the user 
inforamtion in
the file 'user_info_file1'.
	* Cntrl-C in the server will close all the clients as the server is disconnected.

Friends will be updated when a user logouts, exits or perform  Cntl-C and logins.

Note:- Please make sure to give correct friend names and correct commands as mentioned above, as errors in input command will throws errors in output and terminates the 
program.
