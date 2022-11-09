#include "problem.h"

#include <string>
#include <sys/stat.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <map>

using namespace std;

extern const string PROB_SUFFIX = ".po-ppddl";

Problem::Problem(const string filename){
	static map<string, int> numActsForDomain;

	cout << "Looking into file: " << filename << endl;

	int i;

	this->filename = filename;

	struct stat st;
	if(!stat(filename.c_str(), &st))
		filesize = st.st_size;
	else
		filesize = 0;

	if(filename.length() >= PROB_SUFFIX.length() && filename.compare(filename.length() - PROB_SUFFIX.length(), PROB_SUFFIX.length(), PROB_SUFFIX) == 0){
		instance = filename.substr(0, filename.length() - PROB_SUFFIX.length());
		int lastSlash = instance.find_last_of('/');
		if(lastSlash == string::npos)
			lastSlash = 0;
		else
			lastSlash++;
		instance = instance.substr(lastSlash);
	}
	else
		instance = "";

	if(!instance.empty()){
		for(i = instance.length() - 1; i >= 0 && isdigit(instance[i]); i--);
		if(i < instance.length() - 1)
		{
			istringstream iss(instance.substr(i+1));
			if(!(iss >> probNum))
				probNum = 0;
		}
		else
			probNum = 0;
		for( ; i >= 0 && instance[i] == '_'; i--);
		domain = instance.substr(0, i + 1);
	}

	predCount = 0;
	obsCount = 0;
	actCount = 0;
	bool inPred = false;
	bool inObs = false;
	bool needPreds = true;
	bool needObs = true;
	bool needActs = (numActsForDomain.count(domain) <= 0);

	string curLine;
	ifstream in(filename.c_str());
	if(!in.is_open())
		cerr << "Failed to open file: " << filename << endl;
	while(in.good() && (needPreds || needObs || needActs)){
		getline(in, curLine);
		string trimmed = curLine;
		trimmed.erase(remove(trimmed.begin(), trimmed.end(), ' '), trimmed.end());
		trimmed.erase(remove(trimmed.begin(), trimmed.end(), '\t'), trimmed.end());

		if(curLine.find("(:predicates") != string::npos)
			inPred = true;
		else if(inPred){
			if(trimmed.compare(")") == 0){
				inPred = false;
				needPreds = false;
			}
			else
				predCount++;
		}

		if(curLine.find("(:observations") != string::npos)
			inObs = true;
		else if(inObs){
			if(trimmed.compare(")") == 0){
				inObs = false;
				needObs = false;
			}
			else
				obsCount++;
		}

		if(curLine.find("(:action") != string::npos)
			actCount++;
	}
	in.close();

	if(needActs)
		numActsForDomain[domain] = actCount;
	else
		actCount = numActsForDomain[domain];
}
