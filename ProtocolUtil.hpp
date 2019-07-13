#ifndef __PROTOCOL_UTIL_HPP__
#define __PROTOCOL_UTIL_HPP__

//处理逻辑
// Entry     -> 
// Request   ->  
// Response  ->
// connect   ->      accept得到的一个新的socket   从连接上读数据 ，往连接上发数据   



#include <iostream>
#include <string>
#include <string.h>
#include <strings.h>         //和<string.h>的区别在于 一个变量的定义 
#include <sstream>           //c++字符串流
#include <unordered_map>     //无序的map
#include <unistd.h>          //大量对于系统调用的封装
#include <stdlib.h>          
#include <sys/types.h>       //基本系统数据类型
#include <sys/stat.h>        // stat()函数返回一个结构，里边包含文件的全部属性
#include <fcntl.h>             
#include <sys/socket.h>      
#include <arpa/inet.h>       //网络编程中的头文件
#include <netinet/in.h>      //互联网地址族
#include <sys/sendfile.h>   
#include <sys/wait.h>
#include "Log.hpp"

#define OK 200
#define NOT_FOUND 404
#define BAD_REQUEST 400
#define SERVER_ERROR 500

#define WEB_ROOT "wwwroot"
#define HOME_PAGE "index.html"
#define PAGE_404 "404.html"

#define HTTP_VERSION "http/1.0"

std::unordered_map<std::string, std::string> stuffix_map{
  {".html","text/html"},
    {".htm", "text/html"},
    {".css", "text/css"},
    {".js", "application/x-javascript"}

};

class ProtocolUtil{
  public:
    static void MakeKV(std::unordered_map<std::string, std::string> &kv_,\
        std::string &str_)
    {
      std::size_t pos = str_.find(": ");
      if(std::string::npos == pos){
        return;

      }

      std::string k_ = str_.substr(0, pos);
      std::string v_ = str_.substr(pos+2); 

      kv_.insert(make_pair(k_, v_));

    }
    static std::string IntToString(int code)
    {
      std::stringstream ss;
      ss << code;
      return ss.str();

    }
    static std::string CodeToDesc(int code)
    {
      switch(code){
        case 200:
          return "OK";
        case 400:
          return "Bad Request";
        case 404:
          return "Not Found";
        case 500:
          return "Internal Server Error";
        default:
          return "Unknow";

      }

    }
    static std::string SuffixToType(const std::string &suffix_)
    {
      return stuffix_map[suffix_];

    }

};

class Request{
  public:
    std::string rq_line;    //请求行
    std::string rq_head;    //请求报头
    std::string blank;      //空行
    std::string rq_text;    //请求正文
  private:
    std::string method;
    std::string uri;
    std::string version;
    bool cgi; //method=POST, GET-> uri(?)

    std::string path;
    std::string param;

    int resource_size;
    std::string resource_suffix;
    std::unordered_map<std::string, std::string> head_kv;
    int content_length;

  public:
    Request():blank("\n"), cgi(false), path(WEB_ROOT), resource_size(0),content_length(-1), resource_suffix(".html")
  {}
    std::string &GetParam()
    {
      return param;

    }
    int GetResourceSize()
    {
      return resource_size;

    }
    void SetResourceSize(int rs_)
    {
      resource_size = rs_;

    }
    std::string &GetSuffix()
    {
      return resource_suffix;

    }
    void SetSuffix(std::string suffix_)
    {
      resource_suffix = suffix_;

    }
    std::string &GetPath()
    {
      return path;

    }
    void SetPath(std::string &path_)
    {
      path = path_;

    }
    
    //解析请求的第一行  将请求行拆开，数据写到相关的变量中;
    void RequestLineParse()
    {

      std::stringstream ss(rq_line);
      ss >> method >> uri >> version;

    }
    void UriParse()
    {
      if(strcasecmp(method.c_str(), "GET") == 0){
        std::size_t pos_ = uri.find('?');
        if(std::string::npos != pos_){
          cgi = true;
          path += uri.substr(0, pos_);
          param = uri.substr(pos_+1);

        }
        else{
          path += uri;

        }

      }
      else{
        path += uri;

      }

      if(path[path.size() -1] == '/'){
        path += HOME_PAGE;

      }

    }
    bool RequestHeadParse()
    {
      int start = 0;
      while(start < rq_head.size()){
        std::size_t pos = rq_head.find('\n', start);
        if(std::string::npos == pos){
          break;

        }

        std::string sub_string_ = rq_head.substr(start, pos - start);
        if(!sub_string_.empty()){
          LOG(INFO, "substr is not empty!");
          ProtocolUtil::MakeKV(head_kv, sub_string_);

        }
        else{
          LOG(INFO, "substr is empty!");
          break;

        }
        start = pos + 1;

      }
      return true;

    }
    int GetContentLength()
    {
      std::string cl_ = head_kv["Content-Length"];
      if(!cl_.empty()){
        std::stringstream ss(cl_);
        ss >> content_length;

      }
      return content_length;

    }
   
  
    //判断方法是否合法：我们认为忽略大小写
    bool IsMethodLegal()
    {
      if( strcasecmp(method.c_str(),"GET") == 0 ||\
          (cgi = (strcasecmp(method.c_str(),"POST") == 0)) ){
        return true;

      }

      return false;

    }
    bool IsPathLegal()
    {
      struct stat st;
      if(stat(path.c_str(), &st) < 0){
        LOG(WARNING, "path not found!");
        return false;

      }

      if(S_ISDIR(st.st_mode)){
        path += "/";
        path += HOME_PAGE;

      }
      else{
        if((st.st_mode & S_IXUSR ) ||\
            (st.st_mode & S_IXGRP) ||\
            (st.st_mode & S_IXOTH)){
          cgi = true;

        }

      }

      resource_size = st.st_size;
      std::size_t pos = path.rfind(".");
      if(std::string::npos != pos){
        resource_suffix = path.substr(pos);

      }
      return true;

    }
    bool IsNeedRecvText()
    {
      if(strcasecmp(method.c_str(), "POST") == 0){
        return true;

      }
      return false;

    }
    bool IsCgi()
    {
      return cgi;

    }
    ~Request()
    {}

};

class Response{
  public:
    std::string rsp_line;   //响应行
    std::string rsp_head;   //响应报头
    std::string blank;      //空行
    std::string rsp_text;   //响应正文
    int fd;
    int code;
  public:
    Response():blank("\n"), code(OK), fd(-1)
  {}

    void MakeResponseHead(Request *&rq_)
    {

    }




class Connect{
  private:
    int sock;
  public:
    Connect(int sock_):sock(sock_)
  {}
    //从socket里边 读取一行; 3中行分割符： \n , \r, \r\n
    //将读到的任何一行以\n结尾；
    int RecvOneLine(std::string &line_)
    {
      char c = 'X';                         //warning : 注意c一定要初始化，不然随机值可能会导致循环直接失败
      while(c != '\n'){
        ssize_t s = recv(sock, &c, 1, 0);
        if(s > 0){
          if( c == '\r'  ){
            recv(sock, &c, 1, MSG_PEEK);        //recv的窥探功能
            if( c== '\n'  ){
              recv(sock, &c, 1, 0);

            }
            else{
              c = '\n';

            }

          }

          line_.push_back(c);

        }
        else{
          break;

        }

      }
      return line_.size();

    }
    void RecvRequestHead(std::string &head_)
    {
      head_ = "";
      std::string line_;
      while(line_ != "\n"){
        line_ = "";
        RecvOneLine(line_);
        head_ += line_;

      }

    }
    void RecvRequestText(std::string &text_, int len_, std::string &param_)
    {
      char c_;
      int i_ = 0;
      while( i_ < len_ ){
        recv(sock, &c_, 1, 0);
        text_.push_back(c_);
        i_++;

      }

      param_ = text_;

    }
    void SendResponse(Response *&rsp_, Request *&rq_,bool cgi_)
    {
      

    }
    ~Connect()
    {
      if(sock >= 0){
        close(sock);

      }

    }

};

class Entry{
  public:
    static void ProcessNonCgi(Connect *&conn_, Request *&rq_, Response *&rsp_)
    {

               //

    }
    
    
    
    static int  PorcessResponse(Connect *&conn_, Request *&rq_, Response *&rsp_)
    {
      if(rq_->IsCgi()){
        ProcessCgi(conn_, rq_, rsp_);

      }else{
        ProcessNonCgi(conn_, rq_, rsp_);

      }

    }
    static void Process404(Connect *&conn_, Request *&rq_, Response *&rsp_)
    {
      std::string path_ = WEB_ROOT;
      path_ += "/";
      path_ += PAGE_404;
      struct stat st;
      stat(path_.c_str(), &st);

      rq_->SetResourceSize(st.st_size);
      rq_->SetSuffix(".html");
      rq_->SetPath(path_);

      ProcessNonCgi(conn_, rq_, rsp_);

    }
    
    //处理异常
    static void HandlerError(Connect *&conn_, Request *&rq_, Response *&rsp_)
    {
      int &code_ = rsp_->code;
      switch(code_){
        case 400:
          break;
        case 404:
          Process404(conn_, rq_, rsp_);
          break;
        case 500:
          break;
        case 503:
          break;

      }

    }
    
    
    //@@@1
    static int HandlerRequest(int sock_)             //void  ...  (void*)  线程的执行函数的返回值和形参的区别？
    {
      Connect *conn_ = new Connect(sock_);
      Request *rq_ = new Request();
      Response *rsp_ = new Response();

      int &code_ = rsp_->code;                  //状态码

      conn_->RecvOneLine(rq_->rq_line);               //放回到请求的第一行
      rq_->RequestLineParse();                      //解析第一行
      
    
      if( !__glibcxx_requires_partitioned_upper_pred_->IsMethodLegal()  ){
        conn_->RecvRequestHead(rq_->rq_head);
         code_ = BAD_REQUEST;
        goto end;

      }

//对参数的处理就涉及到CGI


      rq_->UriParse();

      if( !rq_->IsPathLegal()  ){
        conn_->RecvRequestHead(rq_->rq_head);
        code_ = NOT_FOUND;
        goto end;

      }

      LOG(INFO, "request path is OK!");

      conn_->RecvRequestHead(rq_->rq_head);
      if(rq_->RequestHeadParse()){
        LOG(INFO, "parse head done");

      }else{
        code_ = BAD_REQUEST;
        goto end;

      }

      if(rq_->IsNeedRecvText()){
        conn_->RecvRequestText(rq_->rq_text, rq_->GetContentLength(),\
            rq_->GetParam());

      }
      //request recv done!
      //            PorcessResponse(conn_, rq_, rsp_);
      //         
      //
      //         end:
      //                        if(code_ != OK){
      //                         HandlerError(conn_, rq_, rsp_);
      //                                         }
      //                            delete conn_;
      //                            delete rq_;
     //                                                                                        delete rsp_;
     //                                                                                                    return code_;
      //                                                                                                            }
      //                                                                                                            };
      //
}
}
