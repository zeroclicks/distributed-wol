/**
* 
* @author: Sury Santos Rahoo
* @contact: sury.workspace at gmail.com
* 
*/

#ifndef _GREENUP_H
#define _GREENUP_H

#include "manager.h"
#include "participante.h"

/* A command starts with ./sleep_server. Choose one for each workstation.
When loopback mode is OFF, i.e, loopback = false:
	Participant mode:
		./sleep_server
	Mananger mode:
		./sleep_server manager

When loopback mode is ON, i.e, loopback = true:
	Participant mode:
		./sleep_server 127.0.0.8
	Mananger mode:
		./sleep_server manager 127.0.0.1
*/

void init(){	
	loopback = false; // loopback mode
	verbose = false; // verbose mode
}

void launch(std::string loopbackIP = ""){
	active = true;

	if(loopback){
		me = Workstation(wGetHostname(), loopbackIP, wGetMACv1(), true);
	}else{
		me = Workstation(wGetHostname(), wGetIP(), wGetMACv2(), true);	
	}
	
	if(isManager){
		print("Welcome manager!");
		
		manager = me;
		echo(me.toString());
		
		usleep(1 * 1000000); // seconds * 1000000
		
		std::thread thr1(receiveMessage, M_PORT); // access to shared resource
		std::thread thr2(mDiscovery); // dismisses shared resource
		std::thread thr3(mMonitoring); // access to shared resource
		std::thread thr4(mNews); // access to shared resource
		mUserInput(); // dismisses shared resource

		thr1.join();
		thr2.join();
		thr3.join();
		thr4.join();
				
	}else{
		print("Bem-vindo participante!");

		manager = Workstation(); //initially assume manager is asleep
		echo(me.toString());

		usleep(1 * 1000000); // seconds * 1000000
		
		std::thread thr1(receiveMessage, P_PORT);
		pUserInput();
		
		thr1.join();
	}
}

void greenUp(int argc, char *argv[]){
	init();

	if(loopback){

		if (argc==2){ //1 argument
			printf("Using loopback IP: %s\n", argv[1]);
			isManager = false;
			launch(argv[1]); //participante
		}else if (argc==3) { 
			// 2 arguments
			printf("Using loopback IP: %s\n", argv[2]);
			isManager = true;
			launch(argv[2]); //manager
		}else {
			throw std::invalid_argument("Loopback mode is ON.\nExpecting 1 or 2 arguments but got " + std::to_string(argc-1) + "!\n");
		}
	}else{
		if (argc==1){ // 0 arguments
			isManager = false;
			launch(); //participante
			
		}else if (argc==2){ // 1 argument
			isManager = true;
			launch(); //manager

		}else{
			throw std::invalid_argument("Loopback mode is OFF.\nExpecting 0 or 1 argument but got " + std::to_string(argc-1) + "!\n");
		}
	}
}


#endif
