/**
* 
* @author: Sury Santos Rahoo
* @contact: sury.workspace at gmail.com
* 
*/

#ifndef _PARTICIPANTE_H
#define _PARTICIPANTE_H

#include "global.h"

void pUserInput(){	
    std::string userInput;
    while (getline(std::cin, userInput)){
        std::string command = userInput.substr(0, userInput.find(" "));
        if (command == "EXIT"){
            std::string msg = Packet(
            	SLEEP_SERVICE_EXIT, 
            	MESSAGE_QUERY, 
            	me.hostname, 
            	me.ip, 
            	me.mac
            ).toString();
            sendMessage(msg, M_PORT, manager.ip, false);
            print("Sent SLEEP SERVICE EXIT to manager");
            active = false;
            break;
        }else{
            print("Usage: EXIT");
        }
    }
    active = false;
}

void pReadMessage(Packet packet){
	
	switch(packet.serviceType){
		case SLEEP_SERVICE_NEWS:
		{
			std::string x = packet.srcHostname; // stringifiedWorkstations
			echo("pReadMessage SLEEP_SERVICE_NEWS");
			echo(x);
			
			// Unmarshalling
			std::string delimiter = "$";
			size_t pos = 0;
			std::vector<std::string> y;
			while ((pos = x.find(delimiter)) != std::string::npos){
				y.push_back(x.substr(0, pos));
				x.erase(0, pos + delimiter.length());
			}
			
			delimiter = "#";
			std::string hostname, ip, mac;
			bool awake;
			workstations.clear();
			
			for(auto item : y){
				pos = item.find(delimiter);
				hostname = item.substr(0, pos);
				item.erase(0, pos + delimiter.length());
				
				pos = item.find(delimiter);
				ip = item.substr(0, pos);
				item.erase(0, pos + delimiter.length());
				
				pos = item.find(delimiter);
				mac = item.substr(0, pos);
				item.erase(0, pos + delimiter.length());
				
				pos = item.find(delimiter);
				awake = item.substr(0, pos) == "awake";
				
				workstations.push_back(Workstation(
					hostname,
					ip,
					mac,
					awake
				));

				//echo("-> " + item);

			}
			displayWorkstations();
		}
		break;

		case SLEEP_SERVICE_DISCOVERY:
		case SLEEP_SERVICE_REQUEST:
		{
			// stackoverflow.com/questions/5685471/error-jump-to-case-label-in-switch-statement
			
			if(!manager.awake){
				// update manager only once
				manager = Workstation(
					packet.srcHostname, 
					packet.srcIP, 
					wParseMAC(packet.srcMAC), 
					true
				);			
			}

			std::string msg = Packet(
				packet.serviceType, 
				MESSAGE_REPLY, 
				me.hostname, 
				me.ip,  
				me.mac
			).toString();

			echo("My MAC is: " + me.mac);
			echo("My MAC' is: " + wParseMAC(me.mac));

			sendMessage(msg, M_PORT, manager.ip, false);
		}
		break;

		default:
			print("Error pReadMessage default");
		break;
	}
}

		
#endif
