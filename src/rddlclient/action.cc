#include "action.h"

#include <string>
#include <vector>
#include <sstream>

using namespace std;

Action::Action(const string &ppddlAct)
: Fluent()
{
	const string delimStart = "__";
	const string delimArgs = "_";
	int endIndex;

	endIndex = ppddlAct.find(delimStart);
	if(endIndex == string::npos)
		endIndex = ppddlAct.length();
	if(endIndex != 0)
	{
		name = ppddlAct.substr(0, endIndex);

		for(int startIndex = endIndex + delimStart.length(); startIndex < ppddlAct.length(); startIndex = endIndex + delimArgs.length())
		{
			endIndex = ppddlAct.find(delimArgs, startIndex);
			if(endIndex == string::npos)
				endIndex = ppddlAct.length();
			if(startIndex != endIndex)
				args.push_back(ppddlAct.substr(startIndex, endIndex - startIndex));
		}
	}
}

std::string Action::asXML()
{
	ostringstream os;

	if((name.compare("noop") != 0 && name.compare("NOOP") != 0) || !args.empty())
	{
		os << "<action>";
		os << "<action-name>" << name << "</action-name>";
		for(vector<string>::iterator arg_it = args.begin(); arg_it != args.end(); arg_it++)
			os << "<action-arg>" << (*arg_it) << "</action-arg>";
		os << "<action-value>true</action-value>";
		os << "</action>";
	}

	return os.str();
}
