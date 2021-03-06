#pragma once
#include "stdafx.h"
#include "FxcmApiStruct.h"
#include "offerTablelistener.h"

//类提前声明，将相应的头文件移到cpp中
/*
在编写C++程序的时候，偶尔需要用到前置声明(Forward declaration)。
因为类A中用到了类B，而类B的声明出现在类A的后面。如果没有类B的前置说明，
下面的程序将不同通过编译，编译器将会给出类似“缺少类型说明符”这样的出错提示。
*/
class SessionStatusListener;
class ResponseListener;
class OfferTableListener;

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
	void PrintOfferSubscribeStatus(IO2GSession *session, const char *sInstrument, bool IsPrint = false);
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
	OfferTableListener * mOfferTableListener = NULL;
	bool bConnected = false;

	//3. 链接参数
	string  UserName;	//
	string  PWD;		//
	string  URL;		//"http://www.fxcorporate.com/Hosts.jsp"
	string  CONN;		// "Demo" or "Real".
	//账户ID
	string AccountID;

	void printPrices(IO2GSession *session, IO2GResponse *response);

	// 4. onTick事件的线程
	boost::thread * pOnTick_thread = NULL;
	ConcurrentQueue<Task> OnTick_queue;
	// 5. tick数据：周期与list<货币对>的对应
	std::map<string, std::list<string>> Tick_instruments;
	//周期与线程对应关系
	std::map<string, boost::thread*> TickThread;

public:
	//构造函数
	HxcmApi();
	HxcmApi(string  userName, string   pwd, string  url, string connection, string accounntid);

	//析构函数
	~HxcmApi();

	//==================================================================================
	//功能函数，或辅助函数
	//==================================================================================
	bool isConnected() { return bConnected; };

	//==================================================================================
	// 查询账户信息
	//==================================================================================


	//==================================================================================
	//处理Task的消息分发类，需要公开，在ResponseListener类中要调用
	//==================================================================================
	void processTask();
	void putTask(Task task);

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
	void processLogin(Task task);
	virtual void onLogin(boost::python::dict data) {}; //
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
	//void qryTickData();
	//注册Tick数据
	void regTick(dict ticks);

	//处理分发来的tick任务
	//void processTick(Task task);

	//OnTick回调函数
	virtual void onTick(dict data) {};

	//==================================================================================
	// 查询类函数，提供查询账户、查询仓位等函数
	//==================================================================================
	// 账户查询函数
	dict qryAccount(string accountId);
	//
	void qryPosition(string instrument);
	//订阅货币对信息任务的处理函数
	void processqryPosition(Task task);
	virtual void onQryPosition(boost::python::list data) {};

	// 查询货币对的基本信息
	boost::python::list qryInstrumentInfo();
	// 查询货币对的实时信息
	boost::python::dict qryInstrumentRealtimeInfo(string instrument);

	// 查询货币对交易设置
	boost::python::dict qryTradingSettings(string instrument);


	//==================================================================================
	// 创建一个Open Order，即指定价格的订单
	//==================================================================================
	//发送市场Order
	int SendOpenMarketOrder(string symbol,//货币对的名称，例如"EUR/USD"
					   string AccountID,	//账号ID
					   string BuyOrSell,	// B 买，S 卖
					   int	  Amount,
		               //string TimeInForce,	// IOC,FOK
					   double ClientRate,
					   string CustomID 
	);

	// 以市场价格关闭仓位
	void SendCloseMarketOrder(string tradeId);
	// 关闭指定货币对的全部仓位
	void SendCloseAllPositionsByInstrument(string instrument);
	// 创建Open Range Order 
	void SendOpenRangeOrder(string instrument, string BuyOrSell, int amount, double rateMin, double rateMax, string CustomID);
	// 创建Close Range Order，指明价格范围
	void SendCloseRangeOrderFloat(string tradeid, double minRate, double maxRate, string customID);
	// 创建Close Range Order，指明浮动pips
	void SendCloseRangeOrderInt(string tradeid, int minPoint, int maxPoint, string customID);
	// 创建Create Open Limit Order 
	void SendOpenLimitOrder(string instrument, int amount, double rate, string buySell, string customID);
	void SendCloseLimitOrder(string tradeid, double rate, string customID);

	// 创建Stop Limit Entry Order
	//void SendOpenEntryLimitOrderWithStopLimit();

	// 创建Entry Limit Order 
	void SendEntryLimitOrder(string instument, int amount, double rate, string buySell, string customID);

	//创建Entry Stop Order 
	void SendEntryStopOrder(string instument, int amount, double rate, string buySell, string customID);

	// 创建delete order命令
	void SendDeleteOrder(string orderid, string customID);

	// 创建Edit Order 
	void SendEditOrder(string orderid, int amount, double rate, int trailStep, string customID);



	//==================================================================================
	// Accounts表更新处理事件，
	//==================================================================================
	// Accounts表更新处理事件
	void processAccountsUpdate(Task task);
	virtual void onAccountsUpdate(boost::python::list data) {};

	// TradesTable更新处理事件,sendXXXOrder等命令会触发
	void processTradesTableUpdate(Task task);
	virtual void onTradesTableUpdate(boost::python::list data) {};

	//ClosedTradeTable表更新事件
	void processClosedTradeTableUpdate(Task task);
	virtual void onClosedTradeTableUpdate(boost::python::list data) {};

	// 查询TradesTable
	void qryClosed_TradesTable();
	void processQryClosed_TradesTable(Task task);
	virtual void onQryClosed_TradesTable(boost::python::list data) {};
};

