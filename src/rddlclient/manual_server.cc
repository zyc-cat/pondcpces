#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <sstream>
#include <string.h>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>

using namespace std;

int main(int argc, char *argv[]){
	if(argc < 2){
		cout << "USAGE: manual_server <port>\n";
		return 0;
	}

	int clientPort = atoi(argv[1]);//2324;

	int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(clientSocket < 0){
		 perror("socket");
		 return -1;
	}

	struct sockaddr_in addr;
	memset((char *)&addr, '\0', sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(clientPort);

	int optValue = 1;
	setsockopt(clientSocket, SOL_SOCKET, SO_REUSEADDR, &optValue, sizeof(optValue));

	if(bind(clientSocket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		perror("bind");
		return -1;
	}

	listen(clientSocket, 5);

	socklen_t size = sizeof(addr);
	int oldClientSocket = clientSocket;

	clientSocket = accept(oldClientSocket, (struct sockaddr *)&addr, &size);
	if(clientSocket < 0){
		perror("accept");
		shutdown(oldClientSocket, 2);
		close(oldClientSocket);
		return -1;
	}

	bool firstTime = true;
	bool firstObs = true;

	cout << "Start State? (noop assumed)  ";
	string inMsg, outMsg;
	getline(cin, outMsg);
	outMsg = "noop," + outMsg;
	if(outMsg[outMsg.size()-1] == ',')
		outMsg += ' ';
	outMsg += '\0';
	write(clientSocket, outMsg.c_str(), outMsg.length());

	double time;
	struct timeval startTime, curTime;

	bool doneUs = false;
	while(!doneUs)
	{
		time = 0.0;
		gettimeofday(&startTime, NULL);
		bool doneThem = false;
		while(!doneThem){
			if(time <= 0){
				cout << "Time? ";
				if(firstTime){
					cout << "(0 = end/commit) ";
					firstTime = false;
				}
				cout << " ";
				getline(cin, outMsg);
				istringstream iss(outMsg);
				if(!(iss >> time))
					time = 0.0;
				gettimeofday(&startTime, NULL);
			}

			if(time <= 0.0)
				doneThem = true;
			ostringstream oss;
			oss << time;
			outMsg = oss.str();
			outMsg += '\0';
			write(clientSocket, outMsg.c_str(), outMsg.length());

			char nextChar = '\0';
			string inMsg = "";
			while(read(clientSocket, &nextChar, 1) == 1 && nextChar != '\0')
			{
				inMsg += nextChar;
				nextChar = '\0';
			}
			if(inMsg.empty())
				doneThem = true;
			else
				cout << "Action:  " << inMsg << endl;

			gettimeofday(&curTime, NULL);
			double elapsed = (curTime.tv_sec - startTime.tv_sec) + (curTime.tv_usec - startTime.tv_usec) / 1000000.0;
			if(time < elapsed)
				time = 0.0;
		}

		cout << "Observations? ";
		if(firstObs){
			cout << "(Space seperated. Enter = end, Single space = all false) ";
			firstObs = false;
		}
		cout << " ";
		getline(cin, outMsg);
		if(outMsg.empty())
			doneUs = true;
		outMsg += '\0';
		write(clientSocket, outMsg.c_str(), outMsg.length());
	}

	shutdown(clientSocket, 2);
	close(clientSocket);

	shutdown(oldClientSocket, 2);
	close(oldClientSocket);

	return 0; 
}
