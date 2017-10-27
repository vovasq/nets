#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <map>
#include <vector>
#include <spdlog/spdlog.h>

const int  MAX_MESSAGE_SIZE = 4;
//const int  MAX_CLIENTS = 2;
const int  COMMAND_NUM = 4;

auto logger = spdlog::rotating_logger_mt("clients_logger", "logthreads", 1024*1024, 1);
auto mainlog = spdlog::rotating_logger_mt("main_logger", "log", 1024*1024, 1);

std::vector< std::pair<pthread_t, long> > clients;
pthread_mutex_t main_log_mtx;
//std::array<bool, MAX_CLIENTS> idArray;
std::array<std :: string, COMMAND_NUM> command_list{{
//        "SEND",
        "kill",
        "killall",
        "ls",
        "help"}
};

bool kill_client(int id)
{
//    pthread_mutex_lock(&main_log_mtx);
    logger->info("start to kill client # {}", id);
    if(clients.size() < id)
    {
        logger->info("no client # {}", id);
//        pthread_mutex_unlock(&main_log_mtx);
        return false;
    }
    auto it = clients.begin() + (id - 1);
//    std::cout << "it->first = "<< it->first << std::endl;
    if(shutdown(it->second, SHUT_RDWR) == 0)
        logger->info("success sock off client # {}", it->second);
    else
    {
        logger->error("error sock off client # {} , error = ", it->second, errno);
//        pthread_mutex_unlock(&main_log_mtx);
        return false;
    }
    if(close(it->second) == 0)
        logger->info("success sock close client # {}", it->second);
    else
    {
        logger->error("error sock close client  # {}, error = ", id, errno);
//        pthread_mutex_unlock(&main_log_mtx);
        return false;
    }
    if(pthread_join(it->first, NULL) == 0)
        logger->info("success thread join client  # {}", it->second);
    else
    {
        logger->error("error thread join client  # {}", it->second);
//        pthread_mutex_unlock(&main_log_mtx);
        return false;
    }
    clients.erase(it);
//    pthread_mutex_unlock(&main_log_mtx);
    return true;
}

void showCommands()
{
    std::cout << "List of the commands is:\n";
    for(auto it = command_list.begin();
        it != command_list.end(); it++)
    {
        std::cout << *it <<"\n";
    }

}

void show()
{
    pthread_mutex_lock(&main_log_mtx);
    if(clients.size() != 0)
    {
        int i = 1;
        for (auto it : clients)
            std::cout << "client id" << i++ << " thread: "<< it.first
                      << " number of socket: " << it.second << std::endl;
    }
    else
        std::cout << "no connected clients"<< std::endl;
    pthread_mutex_unlock(&main_log_mtx);
}

int compare_string(char *first, char *second)
{
    return (strcmp(first, second) == 0);
//    while (*first == *second) {
//        if (*first == '\0' || *second == '\0')
//            break;
//
//        first++;
//        second++;
//    }
//
//    if (*first == '\0' && *second == '\0')
//        return 1;
//    else
//        return 0;
}

//
int readn(int sock, char * buff, int size, int flag)
{
    int recv_size = 0;
    while(recv_size < size)
    {
        char temp_buff[size];
        for (int j = 0; j < size ; ++j) {
            temp_buff[j] = 0;
        }
        int temp_recv = recv(sock, temp_buff, size,0);
        if(temp_recv < 0)
            return -1;
//        std::cout << "temp_recv = " << temp_recv<<std::endl;
        for(int i = 0; i < temp_recv; i++)
            buff[i + recv_size] = temp_buff[i];
        recv_size += temp_recv;
//        std::cout << "buff = " << buff << std::endl;
    }
//    std::cout<<"kek" << std::endl;
    return recv_size;
}
void *run_client(void *param)
{
    int i = 0;
    long sock = (long)param;
    while(true)
    {
        char Buffer[MAX_MESSAGE_SIZE];
        for (int j = 0; j < MAX_MESSAGE_SIZE ; ++j) {
            Buffer[j] = 0;
        }
//        logger->info("sock number {}", sock);
//        if(recv(sock, Buffer, MAX_MESSAGE_SIZE, MSG_NOSIGNAL) == -1)
        if(readn(sock, Buffer, MAX_MESSAGE_SIZE, MSG_NOSIGNAL) ==  -1)
        {
            break;
        }
        if(compare_string(Buffer, "exit"))
        {
            logger->info("bryyyak sock {}",sock);
            break;
        }
//        std::cout << Buffer <<std::endl;
        logger->info( "message from sock {}: {}",sock, Buffer);
    }



    if(shutdown(sock, SHUT_RDWR) == 0)
    {
        logger->info("success sock off client with sock # {}", sock);
        if(close(sock) == 0)
        {
            auto it = clients.cbegin();
            for(; it !=  clients.cend(); it++)
            {
                if(it->second == sock)
                {
                    logger->info("success delete client with sock # {}", sock);
                    break;
                }
            }
            if(it != clients.cend())
                clients.erase(it);
            logger->info("success sock close client with sock # {}", sock);
        }
        else
        {
            logger->info("error sock close client  with sock # {}", sock);
        }
    }
    else
    {
        if(errno == EBADF)
            logger->error("sock is not connected # {}", sock);
        else
            logger->error("error unreachable sock # {}", sock);
    }
    logger->info("we finish {}", sock);
    pthread_exit(NULL);
}

void *run_server(void *param)
{
    // have to init array of clients id
//    initIDArr();

    long MainSock = (long)param;
    while(1)
    {
        pthread_t thrd_tmp;
        long tmp_sock = accept(MainSock, 0, 0);
        if (tmp_sock < 0) {
            break;
        }
        if (pthread_create(&thrd_tmp, 0, run_client, (void *) (tmp_sock)) == 0)
        {
            clients.push_back(std::pair<pthread_t, long>(thrd_tmp, tmp_sock));
            mainlog->info("New thread num is {}", tmp_sock);
        }
        else
        {
            mainlog->info("Did not create new thread");
            return NULL;
        }
    }

    pthread_mutex_lock(&main_log_mtx);
    for (auto it: clients)
    {
        if(shutdown(it.second, SHUT_RDWR) == 0)
            mainlog->info("success shutdown sock {}", it.second);
        else
            mainlog->error("error shutdown sock {}", it.second);

        if(close(it.second) == 0)
            mainlog->info("success close sock {}", it.second);
        else
            mainlog->error("error close sock {}", it.second);

        if(pthread_join(it.first, NULL) == 0)
            mainlog->info("success join client thread: {}  number of socket: {}", it.first, it.second);
        else
            mainlog->error("error join client thread: {}  number of socket: {}", it.first, it.second);
    }
    clients.clear();
    pthread_mutex_unlock(&main_log_mtx);
    mainlog->info( "server clients thread is done");
    pthread_exit(NULL);
}

int main (int argc, char ** argv)
{
    pthread_t server_init;

    long MainSock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in SockAddr;
    SockAddr.sin_family = AF_INET;
    SockAddr.sin_port = htons(1234);
    SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(MainSock, (struct sockaddr *) (&SockAddr), sizeof(SockAddr)) == 0)
        mainlog->info("Server is bind");
    else
        mainlog->info("Server bind failed\n");
    if(listen(MainSock, SOMAXCONN) == 0)
        mainlog->info("Server is ready");
    else
        mainlog->info("Server does not listen");

    if(pthread_create(&server_init, NULL, run_server, (void *) (MainSock)) == 0)
        std::cout << "Server is started" << std::endl;
    else
        std::cout << "Error: can not create server_init thread" << std::endl;
    while(true)
    {
        std :: string cmd_string;
        std :: cout << "Enter your command:" << std::endl;

//        std::getline(std::cin, cmd_string);
        std::cin >> cmd_string;
        if(cmd_string.compare("help") == 0)
        {
            showCommands();
        }
        else if(cmd_string.compare("kill") == 0) {
            int client_to_del;
            if(clients.size() > 0)
                while(true){
                    std::cout << "Input number of client:"<<std::endl;
                    std::cin >> client_to_del;
                    if(!(client_to_del >= 0 && client_to_del - 1 < clients.size()))
                        std::cout<<"WRONG input retry!\n";
                    else
                    {
                        if(kill_client(client_to_del) == true)
                        {
                            std::cout << "client removed succesfully"<< std::endl;
                        }
                        else
                            std::cout << "error remove client see logthread.txt"<< std::endl;
                        break;
                    }
                }
            else
                std::cout << "no clietns to remove"<< std::endl;

        }
        else if(cmd_string.compare("killall")== 0)
        {
            std::cout << "The server is turning off...\n";
            if(shutdown(MainSock, 2) == 0)
                std::cout<<"shutdown main sock\n";
            else
                std::cout<<"main sock vipil dva stakana\n";
            if(close(MainSock) == 0)
                std::cout<<"close main sock\n";
            break;
        }
        else if(cmd_string.compare("ls")== 0)
        {
            std::cout << "All the clients connected to the server are:" << std::endl;
            show();
        }
        else
            std::cout << "No such a command:  " << cmd_string <<"  ! Retry!"<<std::endl;

    }
    if(pthread_join(server_init, NULL) == 0)
        std::cout << "Join is done # "<<  server_init << "\nBye!\n";
    else
        printf("Join fault # %li\n", server_init);
    return 0;
}