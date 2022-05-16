#include "../include/mosquittohandler.h"
#include <iostream>
#include <sstream>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

void print(std::string str, bool endl = true);


int main(){

    std::cout << "Inside the main function!" << std::endl;

    MosquittoHandler* m_mosquitto = new MosquittoHandler();

    m_mosquitto->init("10");

    bool connect_ = m_mosquitto->connectToBroker("localhost", 1883);

    std::cout << "connect: " << connect_ << std::endl; 

    m_mosquitto->publish("/test/topic", "Hello!");

    int socket; 

    int flag_ = m_mosquitto->getSocket(socket); 

    std::cout << "socket: " << socket << " flag_: " << flag_ << std::endl;

    return 0;
    
}