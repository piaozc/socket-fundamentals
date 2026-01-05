#include<iostream>
#include<cstring>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
using namespace std;

int handdle_client(int client_fd,fd_set& master,int& maxfd,const int server_fd){
    //client_fd:客户端socket; master:select位图; maxfd:可能要更新的最大值; server_fd:用于更新maxfd时默认值
    //返回值：0：客户关闭 -1：error  >0:正常且表示收到的数据字节数
    char buffer[1024];
    memset(buffer,0,sizeof(buffer));
    ssize_t byte_receive=recv(client_fd,buffer,sizeof(buffer),0);
    if(byte_receive<0){//出现错误
        cerr<<"receive error"<<strerror(errno)<<endl;
        close(client_fd);
        FD_CLR(client_fd,&master);
        if(client_fd==maxfd){//若删除的是maxfd，遍历master修改maxfd
            int i;
            for(i=maxfd-1;i>=0;i--){
                if(FD_ISSET(i,&master)){
                    maxfd=i;
                    break;
                }
            }
            if(i==0){
                maxfd=server_fd;
            }
        }
        return -1;
    }else if(byte_receive==0){//客户端关闭
        std::cout<<"client closed"<<endl;
        close(client_fd);
        FD_CLR(client_fd,&master);
        if(client_fd==maxfd){//若删除的是maxfd，遍历master修改maxfd
            int i;
            for(i=maxfd-1;i>=0;i--){
                if(FD_ISSET(i,&master)){
                    maxfd=i;
                    break;
                }
            }
            if(i==0){
                maxfd=server_fd;
            }
        }
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
    status=getaddrinfo(NULL,"8999",&hints,&res);
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

    //创建select位图
    fd_set master,read_fds;
    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    FD_SET(server_fd,&master);
    int fdmax=server_fd;

    int client_fd;//用于accept
    struct sockaddr_in client_addr;//客户端地址
    socklen_t addrlen;

    //主循环
    while(true){
        //处理select
        read_fds=master;//拷贝位图
        if(select(fdmax+1,&read_fds,NULL,NULL,NULL)==-1){
            cerr<<"select error"<<strerror(errno)<<endl;
            break;
        }

        //遍历位图
        for(int i=0;i<=fdmax;i++){
            if(FD_ISSET(i,&read_fds)){
                if(i==server_fd){//客户端连接到达,调用accept（）
                    addrlen=sizeof(client_addr);
                    client_fd=accept(server_fd,(struct sockaddr*)&client_addr,&addrlen);
                    if(client_fd==-1){
                        cerr<<"accept error"<<strerror(errno)<<endl;
                        continue;
                    }
                    FD_SET(client_fd,&master);
                    fdmax=max(client_fd,fdmax);
                    char client_ip[INET_ADDRSTRLEN];
                    inet_ntop(client_addr.sin_family,
                        &client_addr.sin_addr,
                        client_ip,INET_ADDRSTRLEN);
                    std::cout<<"select server: new connection from"<<client_ip<<"on socket"<<i<<endl;
                }else{//处理客户端数据
                    handdle_client(i,master,fdmax,server_fd);
                }
            }
        }
    }
    freeaddrinfo(res);

    return 0;
}