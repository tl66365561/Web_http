#include"HttpdServer.hpp"

void Usage(string prc)
{
	cout<<"Usage: "<<prc<<"port"<<endl;
	
	}



int main(int argc ,char* argv[])
{
	if(argc!=2)
	{
		Usage(argv[0]);
		exit(1);

		}
//申请服务器对象空间

  HttpdSever *serp=new HttpdServer(atoi(argv[1]));  //new后不用进行判断是否成功，new是操作符，申请失败会直接抛异常  可以去try...catch

  
  serp->InitServer();
  serp->Start();







  delete serp;


	
	
	
	return 0;
	}
