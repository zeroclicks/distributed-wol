/**
* 
* @author: Sury Santos Rahoo
* @contact: sury.workspace at gmail.com
* 
*/

#ifndef _EXTRA_H
#define _EXTRA_H

#include "include.h"

std::string popNewline(std::string str){
	if (str.back() == '\n'){ str.pop_back(); }
	return str;
}

// cout wrapper
void print(std::string str, bool endl=true){
	if(endl){
		std::cout << str << std::endl;	
	}else{
		std::cout << str;
	}
}

// returns a random string
std::string randomString(const int len) {	
	// stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c

	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";
	std::string tmp_s;
	tmp_s.reserve(len);
	for (int i = 0; i < len; ++i) {
		tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
	}
	return tmp_s;
}


// runs a system command and returns its output
std::string systemIO(const char* cmd) {
	// stackoverflow.com/questions/478898/how-do-i-execute-a-command-and-get-the-output-of-the-command-within-c-using-po
	std::array<char, 128> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
	if (!pipe) {
		throw std::runtime_error("popen() failed!");
	}
	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
		result += buffer.data();
	}
	return result;
}

/*
	Drafts

int getPortOffset(std::string ip){
	if (loopback){
		return std::stoi(ip.substr(ip.find_last_of('.')+1, ip.length()));		
	}
	return 0;
}

*/



#endif
