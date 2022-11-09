#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include<unistd.h>
#include <sstream>
#include <string.h>
#include <string>
#include <cstdio>

using namespace std;

int main(int argc, char *argv[]){
	if(argc < 2){
		cout << "USAGE: manual_client <address:port>\n";
		return 0;
	}

	string serverAddr = "localhost";
	int serverPort = 2324;

	string arg = argv[1];
	int colonIndex = arg.find(':');
	string addrStr = arg.substr(0, colonIndex);
	if(!addrStr.empty())
		serverAddr = addrStr;
	if(colonIndex != string::npos){
		istringstream iss(arg.substr(colonIndex+1));
		int port;
		if(iss >> port)
			serverPort = port;
	}

	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(serverSocket < 0){
		perror("socket");
		return -1;
	}

	struct hostent *host = gethostbyname(serverAddr.c_str());
	if(host == NULL){
		perror("gethostbyname");
		return -1;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(serverPort);
	addr.sin_addr = *((struct in_addr *)host->h_addr);
	memset(&(addr.sin_zero), '\0', 8);

	cout << "Connecting" << flush;
	while(connect(serverSocket,(struct sockaddr *) &addr,sizeof(addr)) < 0){
		cout << "." << flush;
		usleep(500000);
	}
	cout << endl;

	char nextChar;
	string inMsg, outMsg;

	inMsg = "";
	while(read(serverSocket, &nextChar, 1) == 1 && nextChar != '\0')
		inMsg += nextChar;
	cout << "Start State:  " << inMsg << endl;

	double time = 0.0;
	bool done_them = false;
	while(!done_them)
	{
		bool done_us = false;
		while(!done_us){
			inMsg = "";
			while(read(serverSocket, &nextChar, 1) == 1 && nextChar != '\0')
				inMsg += nextChar;
			istringstream iss(inMsg);
			if(!(iss >> time))
				time = 0.0;
			if(time <= 0.0)
				done_us = true;
			cout << "Time remaining:  " << time << endl;

			if(done_us)
				outMsg = "";
			else{
				cout << "Action?  ";
				getline(cin, outMsg);
			}

			if(outMsg.empty())
				done_us = true;

//			cout << "(Action: " << outMsg << ")\n";
			outMsg += '\0';
			write(serverSocket, outMsg.c_str(), outMsg.length());
		}

		inMsg = "";
		while(read(serverSocket, &nextChar, 1) == 1 && nextChar != '\0')
			inMsg += nextChar;
		cout << "Observation:  " << inMsg << endl;
		if(inMsg.empty())
			done_them = true;
	}

	shutdown(serverSocket, 2);
	close(serverSocket);

	return 0;
}
