
#ifndef __HTTPD_SERVER_HPP__
#define __HTTPD_SERVER_HPP__

#include <pthread.h>
#include "ProtocolUtil.hpp"
#include "Thread_Pool.hpp"

class HttpdServer{
  private:
    int listen_sock;
    int port;
    ThreadPool *tp;
  public:
    HttpdServer(int port_):port(port_), listen_sock(-1), tp(NULL)
  {}

    void InitServer()
    {
      listen_sock = socket(AF_INET, SOCK_STREAM, 0);
      if(listen_sock < 0){
        LOG(ERROR, "create socket error!");
        exit(2);

      }


      int opt_ = 1;
      setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt_, sizeof(opt_));   //如果断开连接进入timewait，也可以立即重新建立连接

      struct sockaddr_in local_;
      local_.sin_family = AF_INET;
      local_.sin_port = htons(port);
      local_.sin_addr.s_addr = INADDR_ANY;                 //解释参数：  INADDR_ANY：系统中提供的宏 

      if(bind(listen_sock, (struct sockaddr*)&local_, sizeof(local_)) < 0){
        LOG(ERROR, "bind socket error!");
        exit(3);

      }

      if(listen(listen_sock, 5) < 0){
        LOG(ERROR, "listen socket error!");
        exit(4);

  
      }

      LOG(INFO, "initServer success!");
      tp = new ThreadPool();
      tp->initThreadPool();

    }

    void Start()
    {
      LOG(INFO, "Start Server begin!");
      for( ; ;  ){
        struct sockaddr_in peer_;
        socklen_t len_ = sizeof(peer_);
        int sock_ = accept(listen_sock, (struct sockaddr*)&peer_, &len_);
        if(sock_ < 0){
          LOG(WARNING, "accept error!");
          continue;

        }
        
        //pthread_t tid_;
        //pthread_create(&tid_,NULL,Entry:HandlerRequest,(void*)sock_);
        
        Task t;
 
        //分析这里为什么不直接构造函数赋值，而是要调用set
        t.SetTask(sock_, Entry::HandlerRequest);
        tp->PushTask(t);

      }

    }

    ~HttpdServer()
    {
      if(listen_sock != -1){                      
        close(listen_sock);

      }
      port = -1;

    }

};

#endif
 
