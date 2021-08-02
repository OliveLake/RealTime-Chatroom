# real-time chatroom

> based on socket & multi-thread 

## Environment

- ubuntu 19.04

## How to run this program

Run GNU make in the repository

```
make
```
Then start
```
./server.out
./client.out
```

## How to use this program

client : 
(1) User needs to enter nickname first.  
(2) If other users send a message, terminla shows who is talking and the message.   
(3) If someone leaves chatroom, show [nickname][ip] leaves the chatroom.  

server : 
(1) If someone joins chatroom, show its nickname & ip address.  
(2) If someone sends a message, send this message to other clients with ip & nickname.  
(3) If someone leaves chatroom, show [nickname][ip][sockedfd] leaves the chatroom.  

Command        | Describtion
-------------- | :------------------------------------------
/exit	       |  Leave the chatroom
/connecting    |  Test connection, responds with socketfd 
/help		   |  Show this help






