#pragma once
#include "stdafx.h"
#include "FxcmApiStruct.h"
#include "offerTablelistener.h"

//����ǰ����������Ӧ��ͷ�ļ��Ƶ�cpp��
/*
�ڱ�дC++�����ʱ��ż����Ҫ�õ�ǰ������(Forward declaration)��
��Ϊ��A���õ�����B������B��������������A�ĺ��档���û����B��ǰ��˵����
����ĳ��򽫲�ͬͨ�����룬����������������ơ�ȱ������˵�����������ĳ�����ʾ��
*/
class SessionStatusListener;
class ResponseListener;
class OfferTableListener;

//�����ռ�
using namespace std;
using namespace boost;
using namespace boost::python;



///-------------------------------------------------------------------------------------
///API�еĲ������
///-------------------------------------------------------------------------------------

//GILȫ�����򻯻�ȡ�ã�
//���ڰ���C++�̻߳��GIL�����Ӷ���ֹpython����
class PyLock
{
private:
	PyGILState_STATE gil_state;

public:
	//��ĳ�����������д����ö���ʱ�����GIL��
	PyLock()
	{
		gil_state = PyGILState_Ensure();
	}

	//��ĳ��������ɺ����ٸö���ʱ�����GIL��
	~PyLock()
	{
		PyGILState_Release(gil_state);
	}
};



///�̰߳�ȫ�Ķ���
template<typename Data>

class ConcurrentQueue
{
private:
	queue<Data> the_queue;								//��׼�����
	mutable mutex the_mutex;							//boost������
	condition_variable the_condition_variable;			//boost��������

public:

	//�����µ�����
	void push(Data const& data)
	{
		mutex::scoped_lock lock(the_mutex);				//��ȡ������
		the_queue.push(data);							//������д�������
		lock.unlock();									//�ͷ���
		the_condition_variable.notify_one();			//֪ͨ���������ȴ����߳�
	}

	//�������Ƿ�Ϊ��
	bool empty() const
	{
		mutex::scoped_lock lock(the_mutex);
		return the_queue.empty();
	}

	//ȡ��
	Data wait_and_pop()
	{
		mutex::scoped_lock lock(the_mutex);

		while (the_queue.empty())						//������Ϊ��ʱ
		{
			the_condition_variable.wait(lock);			//�ȴ���������֪ͨ
		}

		Data popped_value = the_queue.front();			//��ȡ�����е����һ������
		the_queue.pop();								//ɾ��������
		return popped_value;							//���ظ�����
	}

};


///-------------------------------------------------------------------------------------
///hxcmConnect API�ķ�װ��
///-------------------------------------------------------------------------------------

class HxcmApi
{
private:
	//=========================================================================
	//�����ú���
	//=========================================================================
	void PrintOfferSubscribeStatus(IO2GSession *session, const char *sInstrument, bool IsPrint = false);
	//���ڱ���:��ѯTick��Ϣ��RequestId
	const char * tickRequestId;
	
protected:
	// 1. ���Ĳ���
	//�����߳�ָ�루��python���������ݣ�
	thread * pTask_thread;
	//�������
	ConcurrentQueue<Task> task_queue;

	// 2. HxcmApi��һЩ�������������õ�
	IO2GSession * pSession = NULL;
	SessionStatusListener *pSessionStatusListener = NULL;
	ResponseListener *pResponseListener = NULL;
	OfferTableListener * mOfferTableListener = NULL;
	bool bConnected = false;

	//3. ���Ӳ���
	string  UserName;	//
	string  PWD;		//
	string  URL;		//"http://www.fxcorporate.com/Hosts.jsp"
	string  CONN;		// "Demo" or "Real".
	//�˻�ID
	string AccountID;

	void printPrices(IO2GSession *session, IO2GResponse *response);

	// 4. onTick�¼����߳�
	boost::thread * pOnTick_thread = NULL;
	ConcurrentQueue<Task> OnTick_queue;
	// 5. tick���ݣ�������list<���Ҷ�>�Ķ�Ӧ
	std::map<string, std::list<string>> Tick_instruments;
	//�������̶߳�Ӧ��ϵ
	std::map<string, boost::thread*> TickThread;

public:
	//���캯��
	HxcmApi();
	HxcmApi(string  userName, string   pwd, string  url, string connection, string accounntid);

	//��������
	~HxcmApi();

	//==================================================================================
	//���ܺ�������������
	//==================================================================================
	bool isConnected() { return bConnected; };

	//==================================================================================
	// ��ѯ�˻���Ϣ
	//==================================================================================


	//==================================================================================
	//����Task����Ϣ�ַ��࣬��Ҫ��������ResponseListener����Ҫ����
	//==================================================================================
	void processTask();
	void putTask(Task task);

	//==================================================================================
	// ������Ϣ��python�ͻ���
	//==================================================================================
	void sendMessage(string message);
	//��Ϣ������Ĵ�������
	void processMessage(Task task);
	//��python�ն˵�֪ͨ��Ϣ�ص�����
	virtual void onMessage(boost::python::dict data) {};//��python�е���̳�ʵ��

	//==================================================================================
	// ��½��ǳ�
	//==================================================================================	
	void Login(bool IsBlock = false);
	void Logout();
	void processLogin(Task task);
	virtual void onLogin(boost::python::dict data) {}; //
	//==================================================================================
	// ��ʷ���ݲ�ѯ
	//==================================================================================	
	// ��ѯ��ʷ���ݣ��첽���ã�py������OnRepGetHisPrices�¼��ص������н�������
	bool qryHisPrices(string instrument, string stimeFrame, int maxBars, string beginDate, string endDate);
	bool qryLastHisPrice(string instrument, string stimeFrame);
	//��ȡ��ʷ�г���Ϣ����Ĵ�������
	void processGetHisPrices(Task task);
	//
	//void OnReqGetHisPrices() {};
	//��ʷ�۸��ѯ�Ļص�����
	virtual void onResGetHistoryPrices(dict data, bool last) {};//��python�е���̳�ʵ��

	//==================================================================================
	// ���Ļ��ҶԵ�ʵʱ����
	//==================================================================================
	//���Ļ��ҶԵ�ʵʱ���ۣ�
	void Subscribe(string instrumentId, string status, bool isInstrumentName);
	//���Ļ��Ҷ���Ϣ����Ĵ�������
	void processGetSubScribeData(Task task);
	//���Ļ��Ҷ�offer��Ϣ�Ļص�����
	virtual void onSubscribeInstrument(boost::python::list data) {};//��python�е���̳�ʵ��

	//==================================================================================
	// OnTick
	//==================================================================================
	//����OnTick�¼��߳�
	void StartTick(string timeFrame);
	//��ʱ��ѯ���µ���ʷ����
	void qryTickData(string timeFrame);
	//void qryTickData();
	//ע��Tick����
	void regTick(dict ticks);

	//�����ַ�����tick����
	//void processTick(Task task);

	//OnTick�ص�����
	virtual void onTick(dict data) {};

	//==================================================================================
	// ��ѯ�ຯ�����ṩ��ѯ�˻�����ѯ��λ�Ⱥ���
	//==================================================================================
	// �˻���ѯ����
	dict qryAccount(string accountId);
	//
	void qryPosition(string instrument);
	//���Ļ��Ҷ���Ϣ����Ĵ�������
	void processqryPosition(Task task);
	virtual void onQryPosition(boost::python::list data) {};

	// ��ѯ���ҶԵĻ�����Ϣ
	boost::python::list qryInstrumentInfo();
	// ��ѯ���ҶԵ�ʵʱ��Ϣ
	boost::python::dict qryInstrumentRealtimeInfo(string instrument);

	// ��ѯ���ҶԽ�������
	boost::python::dict qryTradingSettings(string instrument);


	//==================================================================================
	// ����һ��Open Order����ָ���۸�Ķ���
	//==================================================================================
	//�����г�Order
	int SendOpenMarketOrder(string symbol,//���ҶԵ����ƣ�����"EUR/USD"
					   string AccountID,	//�˺�ID
					   string BuyOrSell,	// B ��S ��
					   int	  Amount,
		               //string TimeInForce,	// IOC,FOK
					   double ClientRate,
					   string CustomID 
	);

	// ���г��۸�رղ�λ
	void SendCloseMarketOrder(string tradeId);
	// �ر�ָ�����ҶԵ�ȫ����λ
	void SendCloseAllPositionsByInstrument(string instrument);
	// ����Open Range Order 
	void SendOpenRangeOrder(string instrument, string BuyOrSell, int amount, double rateMin, double rateMax, string CustomID);
	// ����Close Range Order��ָ���۸�Χ
	void SendCloseRangeOrderFloat(string tradeid, double minRate, double maxRate, string customID);
	// ����Close Range Order��ָ������pips
	void SendCloseRangeOrderInt(string tradeid, int minPoint, int maxPoint, string customID);
	// ����Create Open Limit Order 
	void SendOpenLimitOrder(string instrument, int amount, double rate, string buySell, string customID);
	void SendCloseLimitOrder(string tradeid, double rate, string customID);

	// ����Stop Limit Entry Order
	//void SendOpenEntryLimitOrderWithStopLimit();

	// ����Entry Limit Order 
	void SendEntryLimitOrder(string instument, int amount, double rate, string buySell, string customID);

	//����Entry Stop Order 
	void SendEntryStopOrder(string instument, int amount, double rate, string buySell, string customID);

	// ����delete order����
	void SendDeleteOrder(string orderid, string customID);

	// ����Edit Order 
	void SendEditOrder(string orderid, int amount, double rate, int trailStep, string customID);



	//==================================================================================
	// Accounts�����´����¼���
	//==================================================================================
	// Accounts�����´����¼�
	void processAccountsUpdate(Task task);
	virtual void onAccountsUpdate(boost::python::list data) {};

	// TradesTable���´����¼�,sendXXXOrder������ᴥ��
	void processTradesTableUpdate(Task task);
	virtual void onTradesTableUpdate(boost::python::list data) {};

	//ClosedTradeTable�������¼�
	void processClosedTradeTableUpdate(Task task);
	virtual void onClosedTradeTableUpdate(boost::python::list data) {};

	// ��ѯTradesTable
	void qryClosed_TradesTable();
	void processQryClosed_TradesTable(Task task);
	virtual void onQryClosed_TradesTable(boost::python::list data) {};
};
