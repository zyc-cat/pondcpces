#ifndef __PROBLEM_H
#define __PROBLEM_H

#include <string>

class Problem
{
public:
	Problem(const std::string filename);

	std::string getFilename() const { return filename; }
	int getFilesize() const { return filesize; }

	std::string getInstance() const { return instance; }
	std::string getDomain() const { return domain; }
	int getProbNum() const { return probNum; }

	int getPredCount() const { return predCount; }
	int getObsCount() const { return obsCount; }
	int getActCount() const { return actCount; }

	bool operator ==(const Problem &other) const { return this == &other; }

protected:
	std::string filename;
	int filesize;

	std::string instance;
	std::string domain;
	int probNum;

	int predCount;
	int obsCount;
	int actCount;
};

#endif // __PROBLEM_H
