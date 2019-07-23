//  udp  tcp  ip的报头长度怎么查看?
//  


//--------------------
//简单的http代码
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<error.h>
#include<arpa/inet.h>


int main(int argc,char* argv[] )
{
   if(argc!=3)
   {
	   perror("please using like this  : ./http_sever port ip  ");
	   exit(-1);

	   }
  int sockfd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
  if(sockfd<0)
  {
	  perror("create socket error");
	  return -1;
	  
	  }

   struct sockaddr_in sever;
   sever.sin_family=AF_INET;
   sever.sin_port=htons(argv[1]);
   sever.sin_addr.s_addr=inet_addr(argv[2]);
   socklen_t len=sizeof(sever);

   int ret=bind(sockfd,(struct sockaddr*)&sever,len);
   if(ret<0)
   {
	   perror("bind error");
	   return -2;
	   
	   }

    int lis=listen(sockfd,5);
	if (lis<0)
   {
	   perror("listen error");
	   return -3;
	   } 

while(1)
   {
     struct	sockaddr_in client; 
	 int newfd=accept(sockfd,(struct sockaddr*)&client,&len);
	 if(newfd<0)
	 {
		 perror("accept error");
		 continue;
	 }
	   while(1)
	{
	   char buff[1024] = {0};
		ssize_t rec = recv(newfd, buff, 1023, 0);
		if (ret > 0)
		printf("req:[%s]\n", buff);
	    
    char *rsp = "<h1>hello world</h1>";
    memset(buff, 0x00, 1024);
	    sprintf(buff, "%s\r\n%s%d\r\n%s\r\n%s\r\n\r\n%s", 
        "HTTP/1.1 302 Found", 
		"Content-Length: ", strlen(rsp),
		"Content-Type: text/html; charset=UTF-8",
		"Location: http://www.baidu.com/",rsp);
		send(newfd, buff, strlen(buff), 0);
		close(newfd);
										    
	}
	    } 
	return 0;
	}

