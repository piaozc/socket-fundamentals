#include<iostream>
#include<cstring>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<thread>
#include<atomic>
#include<vector>
using namespace std;

void handle_client(int client_fd,const string& client_ip,int client_port,int client_id){
    //client_fd:客户端socket  client_ip:客户端ip   client_port:客户端端口  client_id:客户端id
    //处理接收数据
    char buffer[1024];
    while(true){
        memset(buffer,0,sizeof(buffer));
        ssize_t byte_received=recv(client_fd,buffer,sizeof(buffer)-1,0);
        if(byte_received<0){
            cerr<<"received error"<<strerror(errno)<<endl;
            close(client_fd);
            return;
        }else if(byte_received==0){
            cout<<"client closed"<<endl;
            close(client_fd);
            return;
        }else{
            cout<<"receive:"<<buffer<<endl;
        }

        const char* respond="server received";
        ssize_t total_len=strlen(respond);
        ssize_t total_send=0;
        while(total_send<total_len){
            ssize_t byte_send=send(client_fd,respond+total_send,total_len-total_send,0);
            if(byte_send<0){
                cerr<<"send error"<<strerror(errno)<<endl;
                break;
            }
            total_send+=byte_send;
        }
    }
    close(client_fd);
    cout<<"client closed"<<endl;
}

int main(){
    //创建socket
    int server_fd=socket(AF_INET,SOCK_STREAM,0);
    if(server_fd==-1){
        cerr<<"socket error"<<strerror(errno)<<endl;
        return 1;
    }
    cout<<"socket success"<<endl;

    //bind
    struct sockaddr_in server_add;
    memset(&server_add,0,sizeof(server_add));
    server_add.sin_family=AF_INET;
    server_add.sin_addr.s_addr=INADDR_ANY;
    server_add.sin_port=htons(8888);

    if(bind(server_fd,(struct sockaddr*)&server_add,sizeof(server_add))==-1){
        cerr<<"bind error"<<strerror(errno)<<endl;
        close(server_fd);
        return 1;
    }
    cout<<"bind success"<<endl;

    //listen
    if(listen(server_fd,10)==-1){
        cerr<<"listen error"<<endl;
        return 1;
    }
    cout<<"server: waiting for connection"<<endl;

    int total_client=0;//client_id
    atomic<int> client_counter{0};

    while(true){
        //accept
        struct sockaddr_in client_add;
        memset(&client_add,0,sizeof(client_add));
        socklen_t client_len=sizeof(client_add);
        int client_fd=accept(server_fd,(struct sockaddr*)&client_add,&client_len);
        if(client_fd<0){
            cerr<<"accept error"<<strerror(errno)<<endl;
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        memset(client_ip,0,sizeof(client_ip));
        inet_ntop(AF_INET,&client_add.sin_addr,client_ip,sizeof(client_ip));
        int client_port=ntohs(client_add.sin_port);
        total_client++;
        client_counter++;
        int client_id=total_client;
        thread client_thread(handle_client,client_fd,client_ip,client_port,client_id);
        client_thread.detach();
        cout<<"client:"<<client_ip<<" port:"<<client_port<<" connected"<<endl;

    }
    close(server_fd);
    return 0;
}