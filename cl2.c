// #include <stdio.h>
// #include <sys/socket.h>
// #include <sys/types.h>
// #include <netinet/in.h>
// #include <iostream>
// #include <string.h>


#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>


#include <string>

int main(int argc, char ** argv)
{

	int Socket = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in SockAddr;
	SockAddr.sin_family =AF_INET;
	SockAddr.sin_port =htons(1234);
	SockAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	connect(Socket, (struct sockaddr *)(&SockAddr), sizeof(SockAddr));
	std::string s;
	do{
		std::cout<<"Input your message" << std::endl;
		std::getline(std::cin, s);
        usleep(1000000);
//        int s = send(Socket, s.c_str(), s.size(), MSG_NOSIGNAL);
//	    if(send(Socket, s.c_str(), s.size(), MSG_NOSIGNAL) == -1)
//            break;
    }while(send(Socket, s.c_str(), s.size(), MSG_NOSIGNAL) != -1);

    std::cout << "Bye!"<< std::endl;
//	shutdown(Socket, SHUT_RDWR);
//	close(Socket);
	return 0;
}