#ifndef  _THREAD_POOL_HPP_
#define  _THREAD_POOL_HPP_

#include<iostream>
#include<queue>

//线程池


typedef int (*handler_t) (int);

class Task{

pricate:
   int sock;                
   handler_t handler;

public:
   Task(int sock_,handler_t handler_)
                      :sock(sock_),
                       handler(handler_)
  {   }

   void Run()
   {
      handler(sock);

     }
       



   ~Task()
   {}




}






class ThreadPool{

  private:
    int thread_total_num;    //线程总数
    int thread_idle_num;     //休眠的线程数
    std::queue<task>task_queue; 
    pthread_mutex_t lock; 
    pthread_cond_t cond;


  public:
    ThreadPool(){
   
   


    }

    ~ThreadPool{




    }





}
















#endif
