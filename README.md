Simplified implementation of [Don't Lose Sleep Over Availability: The GreenUp Decentralized Wakeup Service](https://www.usenix.org/conference/nsdi12/technical-sessions/presentation/sen)

> [!NOTE]
> Work done in partial fulfillment of the requirements for the operating systems II course at Instituto de Informática, Universidade Federal do Rio Grande do Sul, Porto Alegre, RS, Brazil

# How to run
Compile via make in order to generate the executable. Instructions for running the executable are on lines 14 to 25 of the greenup.h file. The manager will display a table as soon as a participant appears and, after five seconds, the participant will also be displaying the table. The table is composed of a header and a body. The table header contains relevant workstation fields, while the table body is composed of: (i) manager in the first line and (ii) participants in the remaining lines. Workstations have only one command: EXIT if it is a participant or WAKEUP <hostname> if it is the manager, where <hostname> is the name of one of the participating machines as shown in the table. If a participant falls asleep, the manager will take ten seconds to update the table accordingly. Finally, you can use loopback mode by changing line 29 of the greenup.h file and recompiling. Loopback mode allows testing the system on a single computer using different terminals with the restriction that in addition to the manager, there can be a maximum of one participant.

# Implementation details

### Discovery
Periodically, the manager broadcasts SLEEP_SERVICE_DISCOVERY packets. If the participant responds with packets of the same type, then the manager obtains the sender's data, adding it to the table provided the participant has not been added yet. 

### Management
The table is a vector which contains all participants known to the manager with the exception of those who requested SLEEP_SERVICE_EXIT. Vectors are quite intuitive, which makes implementation easier. Mutual exclusion from the table is implemented through a mutex. 

### Monitoring
Periodically, the manager sends SLEEP_SERVICE_DISCOVERY packets to the workstations shown in the table. If the participating workstation responds immediately with packets of the same type, then it is awake. Otherwise, it is probably asleep. In either case, the manager updates the table accordingly. 

### Interface
Manager and participant interfaces occur via command line and are implemented separately through the getline function, which obtains the typed line. When entering WAKEUP <hostname> as a manager, the manager sends a Wake on Lan (WOL) packet to the hostname. When entering EXIT at the participating station, the participant sends a SLEEP_SERVICE_EXIT packet to the manager communicating their intention to leave. 

## Synchronized data access
All mutual exclusion occurs in the manager.h file via a mutex. Specifically, the mutex is used whenever a participant: (i) is added to the table; (ii) is removed from the table; (iii) has one of its fields changed. Although more prone to logic errors than semaphores, implementation with mutex is quite straightforward.

## Data structures and functions
The `Workstation` class was implemented to model any machine, both the participants and the manager. The distinction is made through the global variable `isManager`. There are five attributes in the class: `string hostname`; `string ip`; `string mac`; `bool awake`; `int missed`. The first four attributes are used to display the table, while the missed attribute counts the number of messages that a given participant did not respond to, thus being a way of determining whether the machine is awake or not. The `Packet` class was implemented to facilitate the exchange of messages between managers and participants via [marshalling/unmarshalling](https://en.wikipedia.org/wiki/Marshalling_(computer_science)). In particular, marshalling occurs via the `toString` method, while unmarshalling occurs via the constructor (overloaded) method. The packet class has five attributes: `packetStype serviceType`; `packetMtype messageType`; `string srcHostname`; `string srcIP`; `string srcMAC`. The first two attributes tell recipients what the message is about, while the last three identify the sender. The name of the main functions is prefixed by the initial of the header that defines them. The manager.h file implements functions for accessing the table, sending magic packets, receiving messages, and subservices, with the last two functionalities also appearing in the participant.h file. Finally, message exchange is defined in the `int sendMessage` and `void receiveMessage` functions.

## Communication primitives
Since it is a distributed system, message exchange (Send/Receive) is used. `Send` adds messages to the communication channel queue and `Receive` removes messages from the queue. In other words, `Send` sends the message to a specific port and IP combination, while `Receive` listens on a single port waiting for new messages. The system uses asynchronous communication since the process which performs `Send` does not block other tasks while the message is not received.

## Overview of difficulties encountered during implementation and how they were resolved (or not).

Challenges faced during implementation: mutual exclusion and debugging. Initially, mutexes were activated for any access made to the table, whether writing or reading. In such manner, however, the table was not being updated as expected, as the mutex was never freed ([starvation](https://en.wikipedia.org/wiki/Starvation_(computer_science))). The problem was solved by triggering the mutex only when writing to the table. Debugging the application at home with only a single computer available allowed only using f loopback mode which does not allow testing wakeonlan. During lab classes, errors occurred that did not occur at home, such as when obtaining the IP and MAC of the machine. Fortunately, such errors were more easily resolved.


## Reference
Sen, Siddhartha, Jacob R. Lorch, Richard Hughes, Carlos Garcia Jurado Suarez, Brian Zill, Weverton Cordeiro, and Jitendra Padhye. "Don't Lose Sleep Over Availability: The GreenUp Decentralized Wakeup Service." In 9th USENIX Symposium on Networked Systems Design and Implementation (NSDI 12), pp. 211-224. 2012.
