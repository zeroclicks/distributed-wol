/**
* 
* @author: Sury Santos Rahoo
* @contact: sury.workspace at gmail.com
* 
*/

#ifndef _MANAGER_H
#define _MANAGER_H

#include "global.h"

#define DISCOVERY_SLEEP 5
#define MONITORING_SLEEP 5
#define NEWS_SLEEP 5

std::mutex wMutex;

// returns workstation index if found, else returns -1
int mGetWorkstationIndexByHostname(std::string hostname){	
	// stackoverflow.com/questions/15517991/search-a-vector-of-objects-by-object-attribute
	auto it = find_if(workstations.begin(), workstations.end(), [&hostname](const Workstation& w) {
		return w.hostname == hostname;
	});

	if (it != workstations.end()){
		auto index = std::distance(workstations.begin(), it);
		return index;
	}
	return -1;
}

// returns workstation index if found, else returns -1
int mGetWorkstationIndexByIP(std::string ip){
	// stackoverflow.com/questions/15517991/search-a-vector-of-objects-by-object-attribute
	auto it = find_if(workstations.begin(), workstations.end(), [&ip](const Workstation& w) {
		return w.ip == ip;
	});

	if (it != workstations.end()){
		auto index = std::distance(workstations.begin(), it);
		return index;
	}
	return -1;
}

void mAddWorkstation(Workstation w){
	// stackoverflow.com/questions/10376065/pushing-unique-data-into-vector
	if (mGetWorkstationIndexByIP(w.ip) != -1){ return;} // already added
	wMutex.lock();
	workstations.push_back(w);
	wMutex.unlock();
	displayWorkstations();
}

void mDelWorkstation(std::string hostname){
	// stackoverflow.com/questions/48997351/remove-object-from-vector-based-on-object-property
	wMutex.lock();
	workstations.erase(std::remove_if(
		begin(workstations), end(workstations),
		[hostname](Workstation const& w){
			return w.hostname == hostname;
	}), end(workstations));
	wMutex.unlock();
	displayWorkstations();
}

void mModWorkstationStatus(int index, bool awake){
	wMutex.lock();
	workstations.at(index).awake = awake;
	wMutex.unlock();
	displayWorkstations();
}

void mCountWorkstationMissed(int index){
	wMutex.lock();
	workstations.at(index).missed += 1;
	wMutex.unlock();
	displayWorkstations();
}

void mResetWorkstationMissed(int index){
	wMutex.lock();
	workstations.at(index).missed = 0;
	wMutex.unlock();
	displayWorkstations();
}

//SLEEP SERVICE DISCOVERY
void mDiscovery(){
	std::string msg = Packet(
		SLEEP_SERVICE_DISCOVERY, 
		MESSAGE_QUERY, 
		me.hostname, me.ip, me.mac
	).toString();
	echo("discovery packet: " + msg);
	do {
		sendMessage(msg, P_PORT, "", true);
		echo("M sent SLEEP_SERVICE_DISCOVERY");
		usleep(DISCOVERY_SLEEP * 1000000); // seconds * 1000000
	} while(active);
}

//SLEEP SERVICE REQUEST
void mMonitoring(){
	

	std::string msg = Packet(
		SLEEP_SERVICE_REQUEST,
		MESSAGE_QUERY, 
		me.hostname, me.ip, me.mac
	).toString();
	echo("monitoring packet: " + msg);

	do {
		for (int i=0; i < workstations.size(); i++){
			if(workstations.at(i).missed > 1){
				mModWorkstationStatus(i, false);
			}
			mCountWorkstationMissed(i);			
			sendMessage(
				msg, 
				P_PORT, 
				workstations.at(i).ip, 
				false
			);
		}
		echo("M sent SLEEP_SERVICE_REQUEST");
		usleep(MONITORING_SLEEP * 1000000); // seconds * 1000000
	} while(active);
}

//SLEEP SERVICE NEWS
void mNews(){

	std::string stringifiedWorkstations;
	std::string msg;

	do{
		stringifiedWorkstations = "";
		
		// Marshalling
		for (auto w : workstations){
			stringifiedWorkstations += w.toString() + "$";
		}
		
		// debugging in loopback mode
		//stringifiedWorkstations += stringifiedWorkstations; 
		
		msg = Packet(
			SLEEP_SERVICE_NEWS, 
			MESSAGE_QUERY, 
			stringifiedWorkstations, "0", "0"
		).toString();
			
	
		for (int i=0; i < workstations.size(); i++){
			sendMessage(
				msg, 
				P_PORT, 
				workstations.at(i).ip, 
				false
			);
		}

		echo("M sent SLEEP_SERVICE_NEWS");
		usleep(MONITORING_SLEEP * 1000000); // seconds * 1000000
	} while(active);
}

// returns MAC address in WOL format
std::string mParseMAC(const std::string &hardware_addr){
	std::string ether_addr;
	std::string s;
	unsigned hex;

	for (size_t i = 0; i < hardware_addr.length();){
		s = hardware_addr.substr(i, 2);		
		hex = 0;

		for (size_t j = 0; j < s.length(); ++j){
			hex <<= 4;
			if (isdigit(s[j])){
				hex |= s[j] - '0';
			}else if (s[j] >= 'a' && s[j] <= 'f'){
				hex |= s[j] - 'a' + 10;
			}else if (s[j] >= 'A' && s[j] <= 'F'){
				hex |= s[j] - 'A' + 10;
			}else{
				throw std::runtime_error("ERROR mParseMAC: " + s);
			}
		}

		i += 2;
		// 0xFF = 00000000 00000000 00000000 11111111 (bin)
		// hex & 0xFF: bitwise AND that leaves only the last 8 bites
		ether_addr += static_cast<char>(hex & 0xFF); 
		if (hardware_addr[i] == ':'){
			++i;
		}
	}

	if (ether_addr.length() != 6){
		throw std::runtime_error(hardware_addr + " is not a valid MAC address");
	}
	return ether_addr;
}

// returns a magic packet
std::string mMagicPacket(std::string mac){
	// stackoverflow.com/questions/50791461/create-magic-packet-on-c
	// www.codeproject.com/Articles/11469/Wake-On-LAN-WOL 
	std::string parsedMAC = mParseMAC(mac);
	std::string buffer("\xff\xff\xff\xff\xff\xff");
	for (int i = 0; i < 16; i++){
		buffer.append(parsedMAC);
	}
	return buffer;
}

// sends a magic packet
std::string mWol(int index){
	int ret = sendMessage(
		mMagicPacket(workstations.at(index).mac).c_str(), 
		P_PORT, 
		workstations.at(index).ip, 
		false
	);
	if (ret < 0){ return "Failed to send magic packet"; }
	return "Successfully sent magic packet";
}

bool mWakeUp(std::string hostname){
	int i = mGetWorkstationIndexByHostname(hostname);
	if(i==-1){ return false; }
	std::string mac = workstations.at(i).mac;
	
	if(1!=1){
		// WIP
		print(systemIO(("wakeonlan" + mac).c_str()));
	}else{
		print(mWol(i));
	}
	
	return true;
}


void mUserInput(){
	std::string userInput;
	while (getline(std::cin, userInput)){
		std::string command = userInput.substr(0, userInput.find(" "));
		if (command == "WAKEUP"){
			std::string hostname = userInput.substr(userInput.find(" ") + 1);
			if (mWakeUp(hostname)){
				print("WOL sent to " + hostname);
			}else{
				print("Host not found!");
			}

		}else{
			print("Usage: WAKEUP hostname");
		}
	}
	active = false;
}

void mReadMessage(Packet packet){
	switch(packet.serviceType){
		case SLEEP_SERVICE_EXIT:
			echo("M received SLEEP_SERVICE_EXIT");
			mDelWorkstation(packet.srcHostname);
		break;

		case SLEEP_SERVICE_DISCOVERY:
			echo("M received SLEEP_SERVICE_DISCOVERY");
			mAddWorkstation(Workstation(
				packet.srcHostname, 
				packet.srcIP,
				wParseMAC(packet.srcMAC),
				true
			));
		break;

		case SLEEP_SERVICE_REQUEST:
		{
			echo("M received SLEEP_SERVICE_REQUEST");
			// stackoverflow.com/questions/5685471/error-jump-to-case-label-in-switch-statement
			int i = mGetWorkstationIndexByIP(packet.srcIP);
			if(i==-1){
				print("Error mReadMessage SLEEP_SERVICE_REQUEST");
				break;
			}
			mResetWorkstationMissed(i);
			mModWorkstationStatus(i, true);
		}
		break;

		default:
			print("Error mReadMessage default");
		break;
	}
}


#endif

