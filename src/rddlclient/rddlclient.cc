/*
 * Copyright 2003-2005 Carnegie Mellon University and Rutgers University
 * Copyright 2007 Hakan Younes
 * Copyright 2011 Sungwook Yoon, Scott Sanner (modified for RDDLSim)
 * Copyright 2011 Alan Olsen, Daniel Bryce (modified for POND)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/***********************************************************************/
/*                           INCLUDES / DEFS                           */
/***********************************************************************/

#include "strxml.h"
#include "action.h"
#include "action_group.h"
#include "observation.h"
#include "problem.h"

/** Ubuntu wants the following two libraries **/
#include <stdlib.h>
#include <string.h>
/**********************************************/

#include <string>
#include <iostream>
#include <fstream>
#include <cerrno>
#include <cstdio>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <dirent.h>
#include <signal.h>
#include <algorithm>
#include <utility>
#include <ctime>
#include <cassert>
#include <sys/time.h>
#include <sys/wait.h>

#include <sstream>
#include <unistd.h>

using namespace std;


/***********************************************************************/
/*                             CONSTANTS                               */
/***********************************************************************/
const string CLIENT_NAME =				"Dan_Bryce__Alan_Olsen";
const char *PROGRAM =							"./pond";
const string PROB_SUFFIX =				".po-ppddl";
const string SOFT_RESET_CMD =			"%%))SOFT-RESET((%%";


/***********************************************************************/
/*                          GLOBAL VARIABLES                           */
/***********************************************************************/
string algorithm = "hop";
string rbpf = "32";
string pmg = "16";
string hLimit = "4";
string numSamples = "4";
string mp = "4";

string serverAddr = "localhost";
int serverPort = 2314;
string clientAddr = "localhost";
int clientPort = 2324;

int oldClientSocket = 0;
int clientSocket = 0;
int serverSocket = 0;
pid_t child_id = 0;

vector<pair<string, string> > partialSoln;

vector<Problem> problems;
Problem* curProblem = NULL;

int total_time = 720;		// (seconds) Later multiplied by problems.size()
struct timeval startTime, curTime;

bool no_program = false;

string outPreffix = "./";
ofstream outResults;


/***********************************************************************/
/*                       FUNCTION DECLARATIONS                         */
/***********************************************************************/
void processCommandLine(int argc, char** argv);
void getNextProblem();
bool solveProblem();
int connectToServer();
int connectToClient();
bool replace_node(XMLNode **node);
bool sessionRequestInfo(const XMLNode* node, int& rounds, long& time);
string obsToString(const XMLNode* stateNode);
bool showState(const XMLNode* stateNode);
bool sendState(const XMLNode* stateNode);
void sendAction(string actGrpStr);


/***********************************************************************/
/*                     MAIN ENTRY POINT TO CLIENT                      */
/***********************************************************************/
int main(int argc, char** argv){
	cout << "Running RDDL Client" << endl << endl;

	signal(SIGPIPE, SIG_IGN);
	signal(SIGCHLD,SIG_IGN);	// Needed?

	processCommandLine(argc, argv);

	cout << "Server " << serverAddr << ":" << serverPort << endl
			 << "Client " << clientAddr << ":" << clientPort << endl
			 << "Total time: " << total_time << " seconds" << endl
			 << "Number of problems: " << problems.size() << endl
			 << endl;

	while(!problems.empty()){
		getNextProblem();
		outResults.open((outPreffix + curProblem->getInstance() + ".txt").c_str());
		if(!solveProblem())
		  cerr << "Failed to completely solve problem " << curProblem->getInstance() << endl;
		problems.erase(find(problems.begin(), problems.end(), *curProblem));
		cout << endl;
		outResults.close();
	}

	return 0;
}

void processCommandLine(int argc, char** argv){
	bool time_set = false;
	for(int i = 0; i < argc; i++)
	{
		string arg = argv[i];
		for(int j = 0; j < arg.length(); j++)
			arg[j] = tolower(arg[j]);
		if(arg.compare("-h") == 0)
		{
			cout << "USAGE: rddlclient [options]" << endl
					 << "  -h                        Help! ..how else did you get here?" << endl
					 << "  -s [ip address][:port]    Server connection (default = \"" << serverAddr << ":" << serverPort << "\")" << endl
					 << "  -c [ip address][:port]    Client connection, i.e. POND (default = \"" << clientAddr << ":" << clientPort << "\")" << endl
					 << "  -p <problem(s)>           Whitespace-seperated list of 1+ problem files and/or directories" << endl
//					 << "  -t <[[hr:]min:]sec>       Time to run client (default = 12:00 * sizeof(\"-p\"))" << endl
					 << "  -n                        No POND (open socket, but don't start POND)" << endl
					 << "  -o <path>                 Output directory/preffix (default = \"" << outPreffix << "\")" << endl
					 << "  -a <algorithm>            Algorithm (default = \"" << algorithm << "\")" << endl
					 << "  -rbpf <number>            Particles for belief state approximation (default = \"" << rbpf << "\")" << endl
					 << "  -pmg <number>             Heuristic samples (default = \"" << pmg << "\")" << endl
					 << "  -depth <number>           Depth of lookahead (default = \"" << hLimit << "\")" << endl
					 << "  -samples <number>         Number of HOP samples (default = \"" << numSamples << "\")" << endl
					 << "  -mp <number>              Maximum number of preconditions for effects in planning graph (default = \"" << mp << "\")" << endl;
			exit(0);
		}
		else if(arg.compare("-s") == 0)
		{
			if(i+1 < argc && argv[i+1][0] != '-'){
				i++;
				arg = argv[i];
				int colonIndex = arg.find(':');
				string addr = arg.substr(0, colonIndex);
				if(!addr.empty())
					serverAddr = addr;
				if(colonIndex != string::npos){
					istringstream iss(arg.substr(colonIndex+1));
					int port;
					if(iss >> port)
						serverPort = port;
				}
			}
		}
		else if(arg.compare("-c") == 0)
		{
			if(i+1 < argc && argv[i+1][0] != '-'){
				i++;
				arg = argv[i];
				int colonIndex = arg.find(':');
				string addr = arg.substr(0, colonIndex);
				if(!addr.empty())
					clientAddr = addr;
				if(colonIndex != string::npos){
					istringstream iss(arg.substr(colonIndex+1));
					int port;
					if(iss >> port)
						clientPort = port;
				}
			}
		}
		else if(arg.compare("-p") == 0)
		{
			while(i+1 < argc && argv[i+1][0] != '-')
			{
				i++;
				string fn = argv[i];
				DIR *dir;
				if((dir = opendir(fn.c_str())) != NULL)
				{
					string path = fn;
					if(path[path.length() - 1] != '/')
						path += '/';
					struct dirent *entry;
					while((entry = readdir(dir)) != NULL)
					{
						fn = path;
						fn += entry->d_name;
						if(fn.length() >= PROB_SUFFIX.length() && fn.compare(fn.length() - PROB_SUFFIX.length(), PROB_SUFFIX.length(), PROB_SUFFIX) == 0)
							problems.push_back(Problem(fn));
					}
				}
				else if(fn.length() >= PROB_SUFFIX.length() && fn.compare(fn.length() - PROB_SUFFIX.length(), PROB_SUFFIX.length(), PROB_SUFFIX) == 0)
					problems.push_back(Problem(fn));
				cout << endl;
			}
		}
		else if(arg.compare("-t") == 0)
		{
			if(i+1 < argc && argv[i+1][0] != '-'){
				i++;

				arg = argv[i];
				stringstream colonSS(arg);
				string colonItem;
				vector<int> nums;
				while(getline(colonSS, colonItem, ':')){
					istringstream iss(colonItem);
					int num;
					if(!(iss >> num))
						num = 0;
					nums.push_back(abs(num));
				}

				total_time = 0;
				int multiplier = 1;
				for(vector<int>::reverse_iterator num_it = nums.rbegin(); num_it != nums.rend(); num_it++, multiplier *= 60)
					total_time += (*num_it) * multiplier;

				time_set = true;
			}
		}
		else if(arg.compare("-n") == 0)
			no_program = true;
		else if(arg.compare("-o") == 0){
			if(i+1 < argc && argv[i+1][0] != '-')
				outPreffix = argv[++i];
		}
		else if(arg.compare("-rbpf") == 0){
			if(i+1 < argc && argv[i+1][0] != '-')
				rbpf = argv[++i];
		}
		else if(arg.compare("-pmg") == 0){
			if(i+1 < argc && argv[i+1][0] != '-')
				pmg = argv[++i];
		}
		else if(arg.compare("-a") == 0){
			if(i+1 < argc && argv[i+1][0] != '-')
				algorithm = argv[++i];
		}
		else if(arg.compare("-depth") == 0){
			if(i+1 < argc && argv[i+1][0] != '-')
				hLimit = argv[++i];
		}
		else if(arg.compare("-samples") == 0){
			if(i+1 < argc && argv[i+1][0] != '-')
				numSamples = argv[++i];
		}
		else if(arg.compare("-mp") == 0){
			if(i+1 < argc && argv[i+1][0] != '-')
				mp = argv[++i];
		}
	}

	if(!time_set)
		total_time *= problems.size();
}

void getNextProblem()
{
	// Find the instance with the smallest problem number for each domain
	map<string, Problem*> firstProbForDomain;
	for(vector<Problem>::iterator prob_it = problems.begin(); prob_it != problems.end(); prob_it++){
		Problem* prob = &(*prob_it);
		string domain = prob->getDomain();
		if(firstProbForDomain.count(domain) <= 0 || firstProbForDomain[domain]->getProbNum() > prob->getProbNum())
			firstProbForDomain[domain] = prob;
	}

	// Select the domain instance with the smallest problem number /*number of actions*/ /*filesize*/
	curProblem = NULL;
	for(map<string, Problem*>::iterator map_it = firstProbForDomain.begin(); map_it != firstProbForDomain.end(); map_it++){
		Problem* prob = (*map_it).second;
//		if(curProblem == NULL || curProblem->getFilesize() > prob->getFilesize())
//		if(curProblem == NULL || curProblem->getActCount() > prob->getActCount())
		if(curProblem == NULL || curProblem->getProbNum() > prob->getProbNum())
			curProblem = prob;
	}

	assert(curProblem != NULL);

	cout << "Selected problem: " << curProblem->getInstance() << endl
			 << "Predicates: " << curProblem->getPredCount() << endl
			 << "Observations: " << curProblem->getObsCount() << endl
			 << "Actions: " << curProblem->getActCount() << endl
			 << "Filesize: " << curProblem->getFilesize() << endl
			 << endl;
}

/* Constructs an XML client and actually runs all of the server interaction*/
bool solveProblem(){
	bool result = true;

	int total_rounds = 0;
	long round_time = 0;
	XMLNode *node = NULL;

  char nextChar;
  string inMsg;
	int count;

	outResults << "Problem: " << curProblem->getInstance() << endl;

	try
	{
		serverSocket = connectToServer();
		if(serverSocket <= 0)
		{
			cerr << "Could not connect to Server at " << serverAddr << ':' << serverPort << endl;
			goto spFail;
		}

		if(replace_node(&node))
		{
			if(!sessionRequestInfo(node, total_rounds, round_time))
			{
				cerr << "Error in server's session-request response" << endl;
				goto spFail;
			}
		}

		outResults << "Algorithm: " << algorithm << endl;
		gettimeofday(&startTime, NULL);

		partialSoln.clear();
		// Connect to client (i.e. POND)
		clientSocket = connectToClient();
		if(clientSocket <= 0)
		{
			cerr << "Could not connect to Client at " << clientAddr << ':' << clientPort << endl;
			goto spFail;
		}

		// Do a round
		for(int rounds_count = 1; rounds_count <= total_rounds; rounds_count++)
		{
			bool first = true;

			partialSoln.clear();

			cout << "***********************************************" << endl;
			cout << ">>> ROUND INIT " << rounds_count << "/" << total_rounds << "; time remaining = " << round_time << endl;
			cout << "***********************************************" << endl;

			ostringstream os;
			os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" <<	"<round-request/>" << '\0';
			write(serverSocket, os.str().c_str(), os.str().length());

			if(!replace_node(&node) || node->getName() != "round-init"){
				cerr << "Error in server's round-request response" << endl;
				goto spFail;
			}

			while(replace_node(&node) && node->getName() != "round-end"){
				// Display the state / observations
				if(!showState(node))
				{
					cerr << "Invalid state response: " << node << endl;
					goto spFail;
				}
				if(first)
					first = false;
				else{
					partialSoln.back().second = obsToString(node);
					sendState(node);
				}

				// Send an action
				string bestAction = "";
				double time_remaining = 500.0;//problem_time / horizons_left;

				bool done = false;
				while(!done){
					os.str("");
					os << time_remaining << '\0';
					write(clientSocket, os.str().c_str(), os.str().length());

					if(time_remaining <= 0.0)
						done = true;

					inMsg = "";
					int num_read;
					do{
					  while((num_read = read(clientSocket, &nextChar, 1)) == 1 && nextChar != '\0')
							inMsg += nextChar;
						if(num_read != 1){
							// Restart POND
							clientSocket = connectToClient();
							if(clientSocket <= 0)
							{
								cerr << "Could not connect to Client at " << clientAddr << ':' << clientPort << endl;
								goto spFail;
							}
							os.str("");
							os << time_remaining << '\0';
							write(clientSocket, os.str().c_str(), os.str().length());
						}
					} while(num_read != 1);

					if(!inMsg.empty())
						bestAction = inMsg;
					else
						done = true;
					
//					time_remaining--;
				}

				if(bestAction.empty())
					bestAction = "noop";
				sendAction(bestAction);
			}

			if(rounds_count == total_rounds)
				write(clientSocket, "\0", 1);				// (send '\0') "No more rounds!"
			else
				write(clientSocket, SOFT_RESET_CMD.c_str(), SOFT_RESET_CMD.length() + 1);		// "Get ready for new round"

			if(node == NULL)
			{
				cerr << "Invalid state response" << endl;
				goto spFail;
			}

			outResults << "Policy trajectory: ";
			for(vector<pair<string, string> >::iterator step_it = partialSoln.begin(); step_it != partialSoln.end(); step_it++){
				pair<string, string>& step = *step_it;
				if(step_it != partialSoln.begin())
					outResults << ',';
				outResults << step.first << ',' << step.second;
			}
			outResults << endl;

			outResults << "Reward:\t";
			string s;
			if(node->dissect("round-reward", s))
			{
				float reward = atof(s.c_str());
				cout << "***********************************************" << endl;
				cout << ">>> END OF ROUND -- REWARD RECEIVED: " << reward << endl;
				cout << "***********************************************\n" << endl;
				outResults << reward;
			}
			outResults << endl;

			gettimeofday(&curTime, NULL);
			outResults << "Time: " << ((curTime.tv_sec - startTime.tv_sec) + (curTime.tv_usec - startTime.tv_usec) / 1000000.0) << endl;
		}

		outResults << "Total reward: ";
		if(replace_node(&node))
		{
			string s;
			if(node->dissect("total-reward", s))
			{
				float reward = atof(s.c_str());
				cout << "***********************************************" << endl;
				cout << ">>> END OF SESSION -- OVERALL REWARD: " << reward << endl;
				cout << "***********************************************\n" << endl;
				outResults << reward;
			}
		}
		outResults << endl << endl;
	}
	catch(const exception& e)
	{
		cerr << "rddlclient: " << e.what() << endl;
		goto spFail;
	}
	catch(...)
	{
		cerr << "rddlclient: fatal error" << endl;
		goto spFail;
	}

	goto spExit;

spFail:
	result = false;

spExit:
	if(node != NULL)
		delete node;

	if(serverSocket > 0){
		shutdown(serverSocket, 2);
		close(serverSocket);
		serverSocket = 0;
	}

	if(clientSocket > 0){
		shutdown(clientSocket, 2);
		close(clientSocket);
		clientSocket = 0;
	}

	if(oldClientSocket > 0){
		shutdown(oldClientSocket, 2);
		close(oldClientSocket);
		oldClientSocket = 0;
	}

	if(child_id > 0){
		killpg(child_id, SIGTERM);
		waitpid(child_id, NULL, WNOHANG);
		child_id = 0;
	}

	return result;
}

/* helper connect function */
int connectToServer(){
	struct hostent *host = gethostbyname(serverAddr.c_str());
	if(host == NULL)
	{
		perror("gethostbyname");
		return -1;
	}

	serverSocket = socket(PF_INET, SOCK_STREAM, 0);
	if(serverSocket == -1)
	{
		perror("socket");
		return -1;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(serverPort);
	addr.sin_addr = *((struct in_addr *)host->h_addr);
	memset(&(addr.sin_zero), '\0', 8);

	if(connect(serverSocket, (struct sockaddr*)&addr, sizeof(addr)) == -1)
	{
		perror("connect");
		return -1;
	}

	ostringstream os;
	os.str("");
	os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
		 << "<session-request>"
		 <<	"<problem-name>" << curProblem->getInstance() << "</problem-name>"
		 <<	"<client-name>" << CLIENT_NAME << "</client-name>"
		 <<	"<no-header/>"
		 << "</session-request>"
		 << '\0';
	write(serverSocket, os.str().c_str(), os.str().length());

	return serverSocket;
	//remember to call close(serverSocket) when you're done
}

/* helper connect function */
int connectToClient(){
	if(clientSocket > 0){
		shutdown(clientSocket, 2);
		close(clientSocket);
		clientSocket = 0;
	}

	if(oldClientSocket > 0){
		shutdown(oldClientSocket, 2);
		close(oldClientSocket);
		oldClientSocket = 0;
	}

	if(child_id > 0){
		killpg(child_id, SIGTERM);
		waitpid(child_id, NULL, WNOHANG);
		child_id = 0;
	}

	clientSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(clientSocket == -1)
	{
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

	if(no_program)
	{
		listen(clientSocket, 5);
		socklen_t size = sizeof(addr);
		oldClientSocket = clientSocket;
		clientSocket = accept(oldClientSocket, (struct sockaddr *)&addr, &size);
		if(clientSocket < 0)
			return -1;
	}
	else
	{
		if(child_id > 0){
			killpg(child_id, SIGTERM);
			waitpid(child_id, NULL, WNOHANG);
			child_id = 0;
		}
		child_id = fork();
		if(child_id < 0) // Error
		{
			cerr << "Failed to fork!\n";
			return -1;
		}
		else if(child_id == 0) // Child
		{
			ostringstream oss;
			oss << clientAddr << ':' << clientPort;
			execl(PROGRAM, PROGRAM, curProblem->getFilename().c_str(), "1", "-on", oss.str().c_str(), "depth", hLimit.c_str(), "-s", algorithm.c_str(), "samples", numSamples.c_str(), "-rbpf", rbpf.c_str(), "-h", "lugrp", "-pmg", pmg.c_str(), "-pg", "global", "-gpr", "1.0", "-mp", mp.c_str(), "-w", "1", (char *)NULL);
			cerr << "Failed to completely run program\n";
			return -1;
		}
		else // Parent
		{
			listen(clientSocket, 5);
			socklen_t size = sizeof(addr);
			oldClientSocket = clientSocket;
			clientSocket = accept(oldClientSocket, (struct sockaddr *)&addr, &size);
			if(clientSocket < 0)
				return -1;
		}
	}

	if(/*true || */!partialSoln.empty()){
		ostringstream os;///*
		bool first = true;
		for(int i = 0; i < partialSoln.size(); i++){
			if(first)
				first = false;
			else
				os << ',';
			os << partialSoln[i].first << ',' << partialSoln[i].second;
		}//*/os << "noop,person_in_elevator_going_up_obs__e0,noop,person_in_elevator_going_up_obs__e0,noop,person_in_elevator_going_up_obs__e0,noop,person_in_elevator_going_up_obs__e0,noop,person_in_elevator_going_up_obs__e0";
		os << '\0';
		write(clientSocket, os.str().c_str(), os.str().length());
	}
	else
		write(clientSocket, "\0", 1);

	return clientSocket;
	//remember to call close(clientSocket) when you're done
}

bool replace_node(XMLNode **node){
	if(*node != NULL)
	{
		delete *node;
		*node = NULL;
	}
	*node = (XMLNode *)read_node(serverSocket);
	return (*node != NULL);
}

/* Extracts session request information. */
bool sessionRequestInfo(const XMLNode* node, int& rounds, long& time)
{
	if(node == NULL)
		return false;

	string s;
	if(!node->dissect("num-rounds", s))
		return false;
	rounds = atoi(s.c_str());

	if(!node->dissect("time-allowed", s))
		return false;
	time = atol(s.c_str());

	return true;
}

string obsToString(const XMLNode* stateNode){
	ostringstream os;

	if(stateNode->size() != 2 || stateNode->getChild(1)->getName().compare("no-observed-fluents") != 0)
	{
		bool first = true;
		for(int i = 0; i < stateNode->size(); i++)
		{
			Observation obs(stateNode->getChild(i));
			if(obs.isTrue())
			{
				if(first)
					first = false;
				else
					os << ' ';
				os << obs.asPPDDL();
			}
		}
	}

	if(os.str().empty())
		os << ' ';

	return os.str();
}

/* Extracts a state (multiple fluents/values) from the given XML node. */
bool showState(const XMLNode* stateNode){
	if(stateNode == NULL || stateNode->getName() != "turn")
		return false;

	cout << "==============================================\n" << endl;

	if(stateNode->size() == 2 && stateNode->getChild(1)->getName() == "no-observed-fluents")
	{
		// The first turn for a POMDP will have this null observation
		cout << "No state/observations received.\n" << endl;
	}
	else
	{
		// Show all state or observation fluents for this turn
		cout << "True state/observation variables:" << endl;
		for (int i = 0; i < stateNode->size(); i++) {
			const XMLNode* cn = stateNode->getChild(i);
			Observation obs(cn);
			// Only display true fluents
			if(obs.isTrue())
				cout << "- " << obs.asRDDL() << endl;
		}
		cout << endl;
	}

	return true;
}

bool sendState(const XMLNode* stateNode){
	if(stateNode == NULL || stateNode->getName() != "turn")
		return false;

	ostringstream os;
	os << partialSoln.back().second;
	os << '\0';

	while(write(clientSocket, os.str().c_str(), os.str().length()) <= 0)
	{
		// Restart POND
		clientSocket = connectToClient();
		if(clientSocket <= 0)
		{
			cerr << "Could not connect to Client at " << clientAddr << ':' << clientPort << endl;
			return false;
		}
	}

	return true;
}

/* Sends action(s) to server on the given socket. */
void sendAction(string actGrpStr){
	ostringstream os;
	ActionGroup grp(actGrpStr);
	cout << "--> Action taken: " << grp.asPPDDL() << "\n\n";
	os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" << grp.asXML() << '\0';
	partialSoln.push_back(make_pair(actGrpStr, string("")));
	write(serverSocket, os.str().c_str(), os.str().length());
}
