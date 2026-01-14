#include<iostream>
#include<cstring>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<sys/epoll.h>
using namespace std;
#define MAX_EVENT 1023

int handdle_client(int client_fd,int epollfd){
    //client_fd:客户端socket; epollfd:用于更新内核数据结构的epollfd
    //返回值：0：客户关闭 -1：error  >0:正常且表示收到的数据字节数
    char buffer[1024];
    memset(buffer,0,sizeof(buffer));
    ssize_t byte_receive=recv(client_fd,buffer,sizeof(buffer),0);
    if(byte_receive<0){//出现错误
        cerr<<"receive error"<<strerror(errno)<<endl;
        close(client_fd);
        epoll_ctl(epollfd,EPOLL_CTL_DEL,client_fd,NULL);
        return -1;
    }else if(byte_receive==0){//客户端关闭
        std::cout<<"client closed"<<endl;
        close(client_fd);
        epoll_ctl(epollfd,EPOLL_CTL_DEL,client_fd,NULL);
        return 0;
    }else{//正常接收数据
        std::cout<<"receive:"<<buffer<<endl;
        return byte_receive;
    }
}

int main(){
    int status;
    struct addrinfo hints;
    struct addrinfo *res;
    memset(&hints,0,sizeof(hints));
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_STREAM;
    hints.ai_flags=AI_PASSIVE;

    //获取ip
    status=getaddrinfo(NULL,"8998",&hints,&res);
    if(status!=0){
        cerr<<"get address error"<<strerror(errno)<<endl;
        return 1;
    }

    //创建socket
    int server_fd=socket(res->ai_family,res->ai_socktype,res->ai_protocol);
    if(server_fd<0){
        cerr<<"socket error"<<strerror(errno)<<endl;
        return 1;
    }

    //防止address already is ues
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        cerr << "setsockopt failed: " << strerror(errno) << endl;
    }

    //bind
    if(bind(server_fd,res->ai_addr,res->ai_addrlen)<0){
        cerr<<"bind error"<<strerror(errno)<<endl;
        return 1;
    }
    std::cout<<"bind success"<<endl;

    //listen
    if(listen(server_fd,10)==-1){
        cerr<<"listen error"<<strerror(errno)<<endl;
        return 1;
    }
    std::cout<<"server: waiting for connection"<<endl;

    //创建epoll instance
    struct epoll_event ev,events[MAX_EVENT];
    int epollfd=epoll_create1(0);
    if(epollfd==-1){
        cerr<<"epoll create error:"<<strerror(errno)<<endl;
        return 1;
    }
    ev.events=EPOLLIN;
    ev.data.fd=server_fd;

    //注册监听事件
    if(epoll_ctl(epollfd,EPOLL_CTL_ADD,server_fd,&ev)==-1){
        cerr<<"epoll control error:"<<strerror(errno)<<endl;
        return 1;
    }

    int n,client_fd;
    struct sockaddr_in client_addr;
    socklen_t client_len=sizeof(client_addr);
    //主循环
    while(true){
        n=epoll_wait(epollfd,events,MAX_EVENT,-1);
        for(int i=0;i<n;i++){
            if(events[i].data.fd==server_fd){
                //accept
                client_fd=accept(server_fd,(struct sockaddr*)&client_addr,&client_len);
                if(client_fd==-1){
                    cerr<<"accept error:"<<strerror(errno)<<endl;
                    return 1;
                }                
                ev.events=EPOLLIN;
                ev.data.fd=client_fd;
                //注册客户端事件
                if(epoll_ctl(epollfd,EPOLL_CTL_ADD,client_fd,&ev)==-1){
                    cerr<<"epoll control error:"<<strerror(errno)<<endl;
                    return 1;
                }
            }else{
                //处理客户端逻辑
                handdle_client(events[i].data.fd,epollfd);
            }
        }
    }
    freeaddrinfo(res);

    return 0;
}