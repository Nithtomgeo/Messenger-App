messenger: messenger_server.c messenger_client.c
	gcc -o messenger_server messenger_server.c -lpthread
	gcc -o messenger_client messenger_client.c
clean:
	rm -rf *o messenger_client
	rm -rf *o messenger_server

