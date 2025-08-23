#include "../includes/Client.hpp"

Client::Client()
{

};

Client::~Client()
{

}; 

int Client::GetFd() //-> getter for fd
{
    return Fd;
} 

void Client::SetFd(int fd) //-> setter for fd
{ 
    Fd = fd;
}

void Client::setIpAdd(std::string ipadd) //-> setter for ipadd
{
    IPadd = ipadd;
}