/**
* 
* @author: Sury Santos Rahoo
* @contact: sury.workspace at gmail.com
* 
*/

#ifndef _WORKSTATION_H
#define _WORKSTATION_H

#include "extra.h"

class Workstation{
public:
	Workstation(){
		this->hostname = "";
		this->ip = "0.0.0.0";
		this->mac = "00:00:00:00:00:00";
		this->awake = false;
		this->missed = 0;
	}

	Workstation(std::string hostname, std::string ip, std::string mac, bool awake)
	{
		this->hostname = hostname;
		this->ip = ip;
		this->mac = mac;
		this->awake = awake;
		this->missed = 0;
	}
	
	std::string hostname;
	std::string ip;
	std::string mac;
	bool awake;
	unsigned int missed;
	
	std::string toString(){
		std::string delim = "#";
		if (this->awake){
			return hostname + delim + ip + delim + mac + delim + "awake";
		}
		return hostname + delim + ip + delim + mac + delim + "asleep";
	}//end of toString()

};//end of class Workstation

// retuns contents of /etc/hostname
std::string wGetHostname(size_t len = 256){	
	char hostname[len];
	gethostname(hostname, len);
	return hostname;
}


// returns IP address as displayed by ifconfig
std::string wGetIP(){
	std::string ip = systemIO(
	"ifconfig | grep \"inet \" | grep -Fv 127.0.0.1 | awk '{print $2}'"
	);
	ip = ip.erase(0,5); 
	return popNewline(ip);
}

// returns MAC address as displayed by ifconfig
std::string wGetMACv1(){	
	// www.codespeedy.com/find-mac-address-of-linux-device-in-cpp/	
	std::string mac = systemIO(
	"ifconfig | grep -o -E '([[:xdigit:]]{1,2}:){5}[[:xdigit:]]{1,2}'"
	);
	return popNewline(mac);
}

//returns mac address from /sys/class/net/eth0/address
std::string wGetMACv2(){
	// www.guru99.com/cpp-file-read-write-open.html#6
	std::string mac = "";
	std::string file_path = "/sys/class/net/eth0/address";

	std::fstream file;

	file.open(file_path, std::ios::in);
	if (!file) {
		print("Error wGetMACv2");
	}
	else {
		char ch;
		while (1) {
			file >> ch;
			if (file.eof()){
				break;
			}
			mac += ch;
		}
	}
	file.close();

	return mac;
}

//erase buggy characters at the end of mac
std::string wParseMAC(std::string mac){
	if(mac.length() > 17){
		return mac.erase(17, mac.length() - 17);
	}else{
		return mac;
	}
}


#endif

