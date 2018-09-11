#pragma once
#include "stdafx.h"
#include "FxcmApiStruct.h"

//类提前声明，将相应的头文件移到cpp中
class SessionStatusListener;
class ResponseListener;

//命名空间
using namespace std;
using namespace boost;
using namespace boost::python;



///-------------------------------------------------------------------------------------
///API中的部分组件
///-------------------------------------------------------------------------------------

//GIL全局锁简化获取用，
//用于帮助C++线程获得GIL锁，从而防止python崩溃
class PyLock
{
private:
	PyGILState_STATE gil_state;

public:
	//在某个函数方法中创建该对象时，获得GIL锁
	PyLock()
	{
		gil_state = PyGILState_Ensure();
	}

	//在某个函数完成后销毁该对象时，解放GIL锁
	~PyLock()
	{
		PyGILState_Release(gil_state);
	}
};



///线程安全的队列
template<typename Data>

class ConcurrentQueue
{
private:
	queue<Data> the_queue;								//标准库队列
	mutable mutex the_mutex;							//boost互斥锁
	condition_variable the_condition_variable;			//boost条件变量

public:

	//存入新的任务
	void push(Data const& data)
	{
		mutex::scoped_lock lock(the_mutex);				//获取互斥锁
		the_queue.push(data);							//向队列中存入数据
		lock.unlock();									//释放锁
		the_condition_variable.notify_one();			//通知正在阻塞等待的线程
	}

	//检查队列是否为空
	bool empty() const
	{
		mutex::scoped_lock lock(the_mutex);
		return the_queue.empty();
	}

	//取出
	Data wait_and_pop()
	{
		mutex::scoped_lock lock(the_mutex);

		while (the_queue.empty())						//当队列为空时
		{
			the_condition_variable.wait(lock);			//等待条件变量通知
		}

		Data popped_value = the_queue.front();			//获取队列中的最后一个任务
		the_queue.pop();								//删除该任务
		return popped_value;							//返回该任务
	}

};


///-------------------------------------------------------------------------------------
///hxcmConnect API的封装类
///-------------------------------------------------------------------------------------

class HxcmApi
{
private:
	//=========================================================================
	//调试用函数
	//=========================================================================
	IO2GOfferRow *getOfferAndPrint(IO2GSession *session, const char *sInstrument, bool IsPrint=false);
	//用于保存:查询Tick信息的RequestId
	const char * tickRequestId;
protected:
	// 1. 核心部分
	//工作线程指针（向python中推送数据）
	thread * pTask_thread;
	//任务队列
	ConcurrentQueue<Task> task_queue;

	// 2. HxcmApi的一些东东，用来调用的
	IO2GSession * pSession = NULL;
	SessionStatusListener *pSessionStatusListener = NULL;
	ResponseListener *pResponseListener = NULL;
	bool bConnected = false;

	//3. 链接参数
	string  UserName;	//
	string  PWD;		//
	string  URL;		//"http://www.fxcorporate.com/Hosts.jsp"
	string  CONN;		// "Demo" or "Real".

	void printPrices(IO2GSession *session, IO2GResponse *response);
	// 4. onTick事件的线程
	boost::thread * pOnTick_thread = NULL;
	ConcurrentQueue<Task> OnTick_queue;
	// 5. tick数据：周期与list<货币对>的对应
	std::map<string, std::list<string>> Tick_instruments;
	//周期与线程对应关系
	std::map<string, boost::thread*> TickThread;

public:
	HxcmApi();
	HxcmApi(string  userName, string   pwd, string  url, string connection);

	//析构函数
	~HxcmApi() {};

	//==================================================================================
	//功能函数，或辅助函数
	//==================================================================================
	bool isConnected() { return bConnected; };

	//==================================================================================
	// 查询账户信息
	//==================================================================================
	void qryAccounts();

	//==================================================================================
	//处理Task的消息分发类，需要公开，在ResponseListener类中要调用
	//==================================================================================
	void processTask();
	void putTask(Task task) ;

	//==================================================================================
	// 发送消息给python客户端
	//==================================================================================
	void sendMessage(string message);
	//消息类任务的处理函数
	void processMessage(Task task);
	//对python终端的通知消息回调函数
	virtual void onMessage(boost::python::dict data) {};//由python中的类继承实现

	//==================================================================================
	// 登陆或登出
	//==================================================================================	
	void Login(bool IsBlock = false);
	void Logout();

	//==================================================================================
	// 历史数据查询
	//==================================================================================	
	// 查询历史数据，异步调用，py程序在OnRepGetHisPrices事件回调函数中接收数据
	bool qryHisPrices(string instrument, string stimeFrame, int maxBars, string beginDate, string endDate);
	bool qryLastHisPrice(string instrument, string stimeFrame);
	//获取历史市场信息任务的处理函数
	void processGetHisPrices(Task task);
	//
	//void OnReqGetHisPrices() {};
	//历史价格查询的回调函数
	virtual void onResGetHistoryPrices(dict data, bool last) {};//由python中的类继承实现

	//==================================================================================
	// 订阅货币对的实时报价
	//==================================================================================
	//订阅货币对的实时报价，
	void Subscribe(string instrumentId, string status, bool isInstrumentName);
	//订阅货币对信息任务的处理函数
	void processGetSubScribeData(Task task);
	//订阅货币对offer信息的回调函数
	virtual void onSubscribeInstrument(boost::python::list data) {};//由python中的类继承实现

	//==================================================================================
	// OnTick
	//==================================================================================
	//启动OnTick事件线程
	void StartTick(string timeFrame);
	//定时查询最新的历史数据
	void qryTickData(string timeFrame);
	void qryTickData();
	//注册Tick数据
	void regTick(dict ticks);

	//处理分发来的tick任务
	void processTick(Task task);

	//OnTick回调函数
	virtual void onTick(dict data) {};
	
};