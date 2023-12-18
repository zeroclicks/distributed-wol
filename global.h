/**
* 
* @author: Sury Santos Rahoo
* @contact: sury.workspace at gmail.com
* 
*/

#ifndef _GLOBAL_H
#define _GLOBAL_H

#define M_PORT 3999
#define P_PORT 4000

#include "workstation.h"


enum packetSType{ // packet service type
	SLEEP_SERVICE_EXIT = 300, // Exit
	SLEEP_SERVICE_DISCOVERY = 301, // Discovery
	SLEEP_SERVICE_REQUEST = 302, //Monitoring
	SLEEP_SERVICE_NEWS = 303, // News
};

enum packetMType{ // packet message type
	MESSAGE_QUERY = 308,
	MESSAGE_REPLY = 309,
};

class Packet {
public:
	Packet(packetSType serviceType, packetMType messageType, 
	std::string srcHostname, std::string srcIP, std::string srcMAC){
		this->serviceType = serviceType;
		this->messageType = messageType;
		this->srcHostname = srcHostname;
		this->srcIP = srcIP;
		this->srcMAC = srcMAC;
	}
	
	// Unmarshalling
	Packet(std::string stringifiedPacket){
		// stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
		std::string delimiter = " ";
		size_t pos = 0;
		std::vector<std::string> tokens;
		while ((pos = stringifiedPacket.find(delimiter)) != std::string::npos)
		{
			tokens.push_back(stringifiedPacket.substr(0, pos));
			stringifiedPacket.erase(0, pos + delimiter.length());
		}
		tokens.push_back(stringifiedPacket);
		
		this->serviceType = (packetSType)std::stoi(tokens.at(0));
		this->messageType = (packetMType)std::stoi(tokens.at(1));
		this->srcHostname = tokens.at(2);
		this->srcIP       = tokens.at(3);
		this->srcMAC      = tokens.at(4);
	}
	
	packetSType serviceType;
	packetMType messageType;
	std::string srcHostname;
	std::string srcIP;
	std::string srcMAC;
	
	// Marshalling
	std::string toString(){
		std::string delim = " ";
		return ""
			+ std::to_string(serviceType) + delim
			+ std::to_string(messageType) + delim
			+ srcHostname + delim
			+ srcIP + delim
			+ srcMAC;
	}//end of toString()
	
}; //end of class Packet

Workstation manager;
Workstation me;
std::vector<Workstation> workstations;

bool isManager;
bool active;

bool loopback; 
bool verbose;


// print wrapper for debugging
void echo(std::string str, bool endl=true){
	if (verbose){
		print(str, endl);
	}
}

void mReadMessage(Packet packet);
void pReadMessage(Packet packet);
int sendMessage(std::string msg, int dstPort, std::string dstIP, bool broadcast){
	
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1){ print("ERROR opening socket"); } // <sys/socket.h>
	struct sockaddr_in dst; // <netinet/in.h>
	dst.sin_family = AF_INET; // <netinet/in.h>
	dst.sin_port = htons(dstPort); // <netinet/in.h>
	
	int ret, optval = 1;
	if(broadcast){
		ret = setsockopt(
			sockfd, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)
		);
		if (ret == -1){
			print("ERROR setsockopt");
		}

		dst.sin_addr.s_addr = inet_addr("255.255.255.255");
	}else{
		dst.sin_addr.s_addr = inet_addr(dstIP.c_str());
	}
	
	ret = sendto(
		sockfd, 
		msg.c_str(), 
		strlen(msg.c_str()), 
		0,
		(const struct sockaddr *) &dst,
		sizeof(struct sockaddr_in) // <sys/socket.h>
	);
	if (ret < 0){ print("ERROR sendto"); }
	
	close(sockfd); // <unistd.h>
	return ret;
}


void receiveMessage(int port){
	// stackoverflow.com/questions/1955198/when-is-ipproto-udp-required
	int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // 
	if (sockfd == -1){ print("ERROR opening socket"); }

	struct sockaddr_in this_addr, sender_addr;
	this_addr.sin_family = AF_INET;
	this_addr.sin_port = htons(port);
	this_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(this_addr.sin_zero), 8);
	 
	int ret = bind(
		sockfd, 
		(struct sockaddr *) &this_addr, 
		sizeof(struct sockaddr)
	);
	if (ret < 0){ print("ERROR binding"); }
		
	while (active) { 

		char buf[1024];
		socklen_t len = sizeof(struct sockaddr_in);

		ret = recvfrom(
			sockfd, 
			buf, 
			1024, 
			0, 
			(struct sockaddr *) &sender_addr, 
			&len
		);
		if (ret < 0) { print("ERROR on recvfrom"); }

		Packet packet = Packet(buf);
		
		if(active){
			if (isManager){
				mReadMessage(packet);
			}else{
				pReadMessage(packet);
			}
		}
		
	}// end of while
	
	close(sockfd); // <unistd.h>
}

void displayWorkstations(){
	system("clear");
	// www.testingdocs.com/displaying-a-table-of-values-in-c-program/
	int colWidth=18;

	std::cout << std::setfill('*') << std::setw(4*colWidth) << "*" << std::endl;
	std::cout << std::setfill(' ') << std::fixed;
	std::cout << std::setw(colWidth) << "Host" 
		  << std::setw(colWidth) << "IP" 
		  << std::setw(colWidth) << "MAC" 
		  << std::setw(colWidth) << "Status" << std::endl; 
	std::cout << std::setfill('*') << std::setw(4*colWidth) << "*" << std::endl; 
	std::cout << std::setfill(' ') << std::fixed; 
	
	
	
	std::cout << std::setw(colWidth) << manager.hostname
		  << std::setw(colWidth) << manager.ip
		  << std::setw(colWidth) << manager.mac
		  << std::setw(colWidth) << "awake" << std::endl;
	
	
	std::string status;

	for (auto w : workstations){
		status = "awake";
		if(!w.awake){
			status = "asleep";
		}
		std::cout << std::setw(colWidth) << w.hostname
			  << std::setw(colWidth) << w.ip
			  << std::setw(colWidth) << w.mac
			  << std::setw(colWidth) << status << std::endl;
	}

	std::cout << std::setfill('*') << std::setw(4*colWidth) << "*" << std::endl;
}

#endif

