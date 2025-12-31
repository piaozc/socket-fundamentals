#include<iostream>
#include<cstring>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
using namespace std;

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

    //accept
    struct sockaddr_in client_add;
    socklen_t client_len=sizeof(client_add);
    int client_fd=accept(server_fd,(struct sockaddr*)&client_add,&client_len);
    if(client_fd==-1){
        cerr<<"accept error"<<endl;
        close(server_fd);
        return 1;
    }

    char buffer[1024];
    memset(buffer,0,sizeof(buffer));
    ssize_t byte_recevied=recv(client_fd,buffer,sizeof(buffer)-1,0);
    if(byte_recevied<0){
        cerr<<"receive error"<<endl;
        close(server_fd);
        return 1;
    }else if(byte_recevied==0){
        cout<<"client closed"<<endl;
    }
    else{
        cout<<"received:"<<buffer<<endl;
    }

    //send
    const char *msg="hello";
    ssize_t byte_send=send(client_fd,msg,strlen(msg),0);
    if(byte_send==-1){
        cerr<<"send eror"<<endl;
    }else{
        cout<<"send success"<<endl;
    }

    close(server_fd);
    close(client_fd);
    



    return 0;
}