// hxcmapi.cpp: 定义 DLL 应用程序的导出函数。
//
#include "stdafx.h"
#include "hxcmapi.h"
#include "Tools.h"
#include "SessionStatusListener.h"
#include "ResponseListener.h"
#include <ATLComTime.h>
#include <string>
#include "boost/timer/timer.hpp"
#include "Account.h"
//#include <cstdlib>

using namespace std;


HxcmApi::HxcmApi()
{
	function0<void> f = boost::bind(&HxcmApi::processTask, this);
	thread t(f);
	this->pTask_thread = &t;
	//
	pSession = NULL;
	pSessionStatusListener = NULL;

};

HxcmApi::HxcmApi(string  userName, string   pwd, string  url, string connection,string accountId)
{
	//
	this->UserName = userName;
	this->PWD = pwd;
	this->URL = url;
	this->CONN = connection;

	this->AccountID = accountId;

	//processTask在另一个线程中运行，提高效率
	function0<void> f = boost::bind(&HxcmApi::processTask, this);
	thread t(f);
	this->pTask_thread = &t;

	//创建会话
	this->pSession = CO2GTransport::createSession();

	//确定使用TableManager
	this->pSession->useTableManager(Yes, 0);

	// 创建会话的StatusListener
	this->pSessionStatusListener = new SessionStatusListener(
															this,
															pSession,
															userName.c_str(),
															pwd.c_str());
	this->pSession->subscribeSessionStatus(pSessionStatusListener);

	// 创建会话的ResponseListener
	this->pResponseListener = new ResponseListener(this->pSession, this);
	this->pSession->subscribeResponse(pResponseListener);

	//创建offerTablelistener实例
	this->mOfferTableListener = new OfferTableListener(this);
	this->pSession->getTableManager()->getTable(Offers)->subscribeUpdate(Update, mOfferTableListener);

}

HxcmApi::~HxcmApi()
{
	this->pSession->release();
	this->mOfferTableListener->release();
	this->pSessionStatusListener->release();
	this->pResponseListener->release();

	delete pSessionStatusListener;
	delete pResponseListener;
	delete mOfferTableListener;
	delete pSession;
	delete pTask_thread;
}

//=========================================================================
//调试用函数
//=========================================================================
void HxcmApi::PrintOfferSubscribeStatus(IO2GSession *session, const char *sInstrument,bool IsPrint)
{
	if (!session || !sInstrument)
		return ;

	IO2GOfferRow *resultOffer = NULL;
	O2G2Ptr<IO2GLoginRules> loginRules = session->getLoginRules();
	if (loginRules)
	{
		O2G2Ptr<IO2GResponse> response = loginRules->getTableRefreshResponse(Offers);
		if (response)
		{
			O2G2Ptr<IO2GResponseReaderFactory> readerFactory = session->getResponseReaderFactory();
			if (readerFactory)
			{
				O2G2Ptr<IO2GOffersTableResponseReader> reader = readerFactory->createOffersTableReader(response);
				for (int i = 0; i < reader->size(); ++i)
				{
					O2G2Ptr<IO2GOfferRow> offer = reader->getRow(i);
					if (offer)
					{
						// 输出货币对的订阅状态
						if (IsPrint)
						{
							if (strcmp(offer->getSubscriptionStatus(), O2G2::SubscriptionStatuses::ViewOnly) == 0)
							{
								std::cout << offer->getInstrument() << "\t:\t[V]iew only" << std::endl;
							}
							else if (strcmp(offer->getSubscriptionStatus(), O2G2::SubscriptionStatuses::Disable) == 0)
							{
								std::cout << offer->getInstrument() << "\t:\t[D]isabled" << std::endl;
							}
							else if (strcmp(offer->getSubscriptionStatus(), O2G2::SubscriptionStatuses::Tradable) == 0)
							{
								std::cout << offer->getInstrument() << "\t:\t[T]rade" << std::endl;
							}
							else
							{
								std::cout << offer->getInstrument() << "\t:\t" << offer->getSubscriptionStatus() << std::endl;
							}
							//std::cout << std::flush;
						}																			
						
					}
				}
			}
		}
	}
	return ;
}

//=====================================================================================

void HxcmApi::printPrices(IO2GSession *session, IO2GResponse *response)
{
	if (response != 0)
	{
		if (response->getType() == MarketDataSnapshot)
		{
			std::cout << "Request with RequestID='" << response->getRequestID() << "' 已经完成:" << std::endl;
			O2G2Ptr<IO2GResponseReaderFactory> factory = session->getResponseReaderFactory();
			if (factory)
			{
				O2G2Ptr<IO2GMarketDataSnapshotResponseReader> reader = factory->createMarketDataSnapshotReader(response);
				if (reader)
				{
					string sTime;
					for (int ii = reader->size() - 1; ii >= 0; ii--)
					{
						DATE dt = reader->getDate(ii);
						Tools::formatDate(dt, sTime);
						if (reader->isBar())
						{
							printf("DateTime=%s, BidOpen=%f, BidHigh=%f, BidLow=%f, BidClose=%f, AskOpen=%f, AskHigh=%f, AskLow=%f, AskClose=%f, Volume=%i\n",
								sTime, reader->getBidOpen(ii), reader->getBidHigh(ii), reader->getBidLow(ii), reader->getBidClose(ii),
								reader->getAskOpen(ii), reader->getAskHigh(ii), reader->getAskLow(ii), reader->getAskClose(ii), reader->getVolume(ii));
						}
						else
						{
							printf("DateTime=%s, Bid=%f, Ask=%f\n", sTime, reader->getBid(ii), reader->getAsk(ii));
						}
					}
				}
			}
		}
	}
}
// 发送消息给python客户端
void HxcmApi::sendMessage(string msg)
{
	//std::cout << "call HxcmApi::sendMessage" << __LINE__ << std::endl;
	Task task = Task();
	task.task_name = OnMessage_smart;
	task.task_data = "HxcmApi Message : " + msg;
	//不能是中文,
	//传出去之后字符串内容就消失了 ，struct 中不能用string吗！！
	//task.task_data = msg;
	//strcpy_s(task.message, strlen(msg.c_str()), msg.c_str());
	this->putTask(task);
}
//======================================================================================
//任务处理部函数，不输出到python中
//======================================================================================

void HxcmApi::putTask(Task task)
{
	this->task_queue.push(task);
}
void HxcmApi::processTask()
{
	while (1)
	{
		//取出队列中的数据
		Task task = this->task_queue.wait_and_pop();
		switch (task.task_name)
		{
			//历史数据
		case OnGetHisPrices_smart:
		{
			this->processGetHisPrices(task);
			break;
		}
		case OnGetSubScribeData_smart:
		{
			this->processGetSubScribeData(task);
			break;
		}			
		case OnMessage_smart:		
		{
			this->processMessage(task);
			break;
		}
		case OnQryPosition_smart:
		{
			this->processqryPosition(task);
			break;
		}
		case OnTradesTableUpdate_smart:
		{
			this->processTradesTableUpdate(task);
			break;
		}
		case OnAccountsTableUpdate_smart:
		{
			this->processAccountsUpdate(task);
			break;
		}
		case OnClosedTradeTableUpdate_smart:
		{
			this->processClosedTradeTableUpdate(task);
			break;
		}
		case OnQryClosed_TradesTable_smart:
		{
			this->processQryClosed_TradesTable(task);
			break;
		}
		case OnLogin_smart:
		{
			this->processLogin(task);
			break;
		}
		default:
			break;
		}
	}
}

//历史数据查询的respons数据处理
void HxcmApi::processGetHisPrices(Task task)
{
	//std::cout << "IN HxcmApi::processGetHisPrices" << __LINE__ << std::endl;
	PyLock lock;
	try
	{
		O2G2Ptr<IO2GMarketDataSnapshotResponseReader> reader = any_cast<O2G2Ptr<IO2GMarketDataSnapshotResponseReader>>(task.task_data);
		boost::python::list rows;
		string sTime;
		for (int i = reader->size() - 1; i >= 0; i--)
		{
			boost::python::dict rowdata;
			if (reader->isBar())
			{
				//rowdata["instrument"] = reqData.instrument;
				DATE dt = reader->getDate(i);
				//将时间由UTC时间转换为本地时间
				DATE dtemp = Tools::ConvertUTC2Local(this->pSession, dt);
				Tools::formatDate(dtemp, sTime);
				rowdata["Instrument"] = task.instrument;
				rowdata["Date"] = sTime;
				rowdata["BidOpen"] = reader->getBidOpen(i);
				rowdata["BidHigh"] = reader->getBidHigh(i);
				rowdata["BidLow"] = reader->getBidLow(i);
				rowdata["BidClose"] = reader->getBidClose(i);
				rowdata["AskOpen"] = reader->getAskOpen(i);
				rowdata["AskHigh"] = reader->getAskHigh(i);
				rowdata["AskLow"] = reader->getAskLow(i);
				rowdata["AskClose"] = reader->getAskClose(i);
				rowdata["Volume"] = reader->getVolume(i);

			}
			else
			{
				//rowdata["instrument"] = reqData.instrument;
				rowdata["Bid"] = reader->getBid(i);
				rowdata["Ask"] = reader->getAsk(i);
			}
			rows.append(rowdata);

		}
		boost::python::dict data;
		data["instrument"] = "usd_cnh";
		data["data"] = rows;
		data["nums"] = reader->size();

		//this->onResGetHistoryPrices( (boost::python::dict) task.task_data, false);
		this->onResGetHistoryPrices(data, false);
	}
	catch (const std::exception& ee)
	{
		std::cout << ee.what() << __LINE__ << std::endl;
		return;
	}	
}

//货币对市场报价的订阅返回数据
void HxcmApi::processGetSubScribeData(Task task)
{
	PyLock lock;
	//1.数据类型转换	
	O2G2Ptr<IO2GTablesUpdatesReader> reader;
	try
	{
		reader = any_cast<O2G2Ptr<IO2GTablesUpdatesReader>>(task.task_data);
	}
	catch (const std::exception& ee)
	{
		std::cout << ee.what() << std::endl;
		return;
	}
	// 2. 从reader中遍历数据
	if (reader)
	{
		boost::python::list rows;
		if (reader->size() < 1)
		{
			return;
		}
		for (int i = 0; i < reader->size(); ++i)
		{
			if (reader->getUpdateType(i) == Update)
			{
				boost::python::list rowdata;
				O2G2Ptr<IO2GOfferRow> offerRow = reader->getOfferRow(i);
				rowdata.append(offerRow->getInstrument());
				rowdata.append(offerRow->getAsk());
				rowdata.append(offerRow->getBid());
				//
				rows.append(rowdata);
			}
		}
		// 3. 发送给事件处理函数
		this->onSubscribeInstrument(rows);
	}
}


//消息类任务的处理函数，将消息封装为python兼容数据类型
void HxcmApi::processMessage(Task task)
{
	PyLock lock;	
	try
	{
		boost::python::dict data;
		string temp = any_cast<string>(task.task_data);
		data["data"] = temp;
		//std::cout << temp << __LINE__ <<std::endl;
		this->onMessage(data);
	}
	catch(boost::bad_any_cast &e) {
		std::cout << e.what() << __LINE__ << std::endl;
		return;
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << __LINE__ <<std::endl;
		return;
	}
}

//查询仓位的response处理函数
void HxcmApi::processqryPosition(Task task)
{
	PyLock lock;
	try
	{
		O2G2Ptr<IO2GTradesTableResponseReader> reader = any_cast<O2G2Ptr<IO2GTradesTableResponseReader>>(task.task_data);
		boost::python::list rows;		
		for (int i = reader->size() - 1; i >= 0; i--)
		{
			boost::python::dict rowdata;
			O2G2Ptr<IO2GTradeRow> trade = reader->getRow(i);
			
			rowdata["OfferID"] = trade->getOfferID();
			rowdata["OfferInstrument"] = Tools::OfferID2OfferName(this->pSession, trade->getOfferID());
			rowdata["AccountName"] = trade->getAccountName();
			rowdata["AccountKind"] = trade->getAccountKind();
			rowdata["Amount"] = trade->getAmount();
			rowdata["BuySell"] = trade->getBuySell();
			rowdata["Commission"] = trade->getCommission();
			rowdata["OpenOrderID"] = trade->getOpenOrderID();
			rowdata["OpenOrderReqID"] = trade->getOpenOrderReqID();
			rowdata["OpenOrderRequestTXT"] = trade->getOpenOrderRequestTXT();
			rowdata["OpenQuoteID"] = trade->getOpenQuoteID();
			rowdata["OpenRate"] = trade->getOpenRate();
			string opentime = "";
			Tools::formatDate(trade->getOpenTime(), opentime);
			rowdata["OpenTime"] = opentime;
			rowdata["Parties"] = trade->getParties();
			rowdata["RolloverInterest"] = trade->getRolloverInterest();
			rowdata["TradeID"] = trade->getTradeID();
			rowdata["TradeIDOrigin"] = trade->getTradeIDOrigin();
			rowdata["UsedMargin"] = trade->getUsedMargin();
			rowdata["ValueDate"] = trade->getValueDate();

			//
			rows.append(rowdata);
		}
		// 发送给事件处理函数
		this->onQryPosition(rows);
	}
	catch (const std::exception& ee)
	{
		PRINTLINE(ee.what());
		return;
	}
}

//
void HxcmApi::processAccountsUpdate(Task task)
{
	PyLock lock;
	try
	{
		O2G2Ptr<IO2GAccountRow> accountRow = any_cast<O2G2Ptr<IO2GAccountRow>>(task.task_data);
		if (accountRow->getAccountID() != this->AccountID )
		{
			return;
		}
		boost::python::list rows;
		boost::python::dict row;
		row["AccountID"] = accountRow->getAccountID();
		row["AccountName"] = accountRow->getAccountName();
		row["AmountLimit"] = accountRow->getAmountLimit(); // 最大的订单金额
		row["ARPID"] = accountRow->getARPID();
		row["ATPID"] = accountRow->getATPID();
		row["Balance"] = accountRow->getBalance(); //期末余额
		row["BaseUnitSize"] = accountRow->getBaseUnitSize();
		row["HadgeMarginPCT"] = accountRow->getHadgeMarginPCT();
		DATE MarginCallDate = Tools::ConvertUTC2Local(this->pSession, accountRow->getLastMarginCallDate());
		string temp = "";
		Tools::formatDate(MarginCallDate, temp);
		row["MarginCallDate"] = temp;
		row["LeverageProfileID"] = accountRow->getLeverageProfileID();
		row["M2MEquity"] = accountRow->getM2MEquity();
		row["MaintenanceFlag"] = accountRow->getMaintenanceFlag();
		row["MaintenanceType"] = accountRow->getMaintenanceType();
		row["ManagerAccountID"] = accountRow->getManagerAccountID();
		row["MarginCallFlag"] = accountRow->getMarginCallFlag();
		row["NonTradeEquity"] = accountRow->getNonTradeEquity();
		row["TableType"] = Tools::getO2GTableTypeName(accountRow->getTableType());
		
		rows.append(row);
		// 发送给事件处理函数
		this->onAccountsUpdate(rows);
	}
	catch (const std::exception& ee)
	{
		PRINTLINE(ee.what());
		return;
	}
}

void HxcmApi::processClosedTradeTableUpdate(Task task)
{
	PyLock lock;
	try
	{
		O2G2Ptr<IO2GClosedTradeRow> closedTradeRow = any_cast<O2G2Ptr<IO2GClosedTradeRow>>(task.task_data);
		if (closedTradeRow->getAccountID() != this->AccountID)
		{
			return;
		}
		boost::python::list rows;
		boost::python::dict row;
		//
		row["TradeID"] = closedTradeRow->getTradeID();
		row["AccountID"] = closedTradeRow->getAccountID();
		row["AccountName"] = closedTradeRow->getAccountName();

		row["Amount"] = closedTradeRow->getAmount();
		row["BuySell"] = closedTradeRow->getBuySell();
		row["Commission"] = closedTradeRow->getCommission();
		row["OfferID"] = closedTradeRow->getOfferID();
		row["Instrument"] = Tools::OfferID2OfferName(this->pSession, closedTradeRow->getOfferID());
		row["OpenOrderID"] = closedTradeRow->getOpenOrderID();
		row["OpenOrderReqID"] = closedTradeRow->getOpenOrderReqID();
		row["OpenOrderRequestTXT"] = closedTradeRow->getOpenOrderRequestTXT();
		row["OpenQuoteID"] = closedTradeRow->getOpenQuoteID();
		row["OpenRate"] = closedTradeRow->getOpenRate();
		DATE dt = Tools::ConvertUTC2Local(this->pSession, closedTradeRow->getOpenTime());
		string temp = "";
		Tools::formatDate(dt, temp);
		row["OpenTime"] = temp;


		row["RolloverInterest"] = closedTradeRow->getRolloverInterest();
		row["TableType"] = Tools::getO2GTableTypeName(closedTradeRow->getTableType());

		row["TradeIDOrigin"] = closedTradeRow->getTradeIDOrigin();
		row["ValueDate"] = closedTradeRow->getValueDate();

		row["CloseOrderID"] = closedTradeRow->getCloseOrderID();
		row["CloseOrderParties"] = closedTradeRow->getCloseOrderParties();
		row["CloseOrderReqID"] = closedTradeRow->getCloseOrderReqID();
		row["CloseOrderRequestTXT"] = closedTradeRow->getCloseOrderRequestTXT();
		row["CloseQuoteID"] = closedTradeRow->getCloseQuoteID();
		row["CloseRate"] = closedTradeRow->getCloseRate();
		row["CloseTime"] = closedTradeRow->getCloseTime();

		row["GrossPL"] = closedTradeRow->getGrossPL();
		row["TradeIDOrigin"] = closedTradeRow->getTradeIDOrigin();
		row["TradeIDRemain"] = closedTradeRow->getTradeIDRemain();

		rows.append(row);
		// 发送给事件处理函数
		this->onClosedTradeTableUpdate(rows);
	}
	catch (const std::exception& ee)
	{
		PRINTLINE(ee.what());
		return;
	}
}

void HxcmApi::processQryClosed_TradesTable(Task task)
{
	PyLock lock;
	try
	{
		O2G2Ptr<IO2GClosedTradesTableResponseReader> reader = any_cast<O2G2Ptr<IO2GClosedTradesTableResponseReader>>(task.task_data);
		boost::python::list rows;
		for (int i = reader->size() - 1; i >= 0; i--)
		{
			boost::python::dict row;
			O2G2Ptr<IO2GClosedTradeRow > closedTradeRow = reader->getRow(i);

			row["TradeID"] = closedTradeRow->getTradeID();
			row["AccountID"] = closedTradeRow->getAccountID();
			row["AccountName"] = closedTradeRow->getAccountName();

			row["Amount"] = closedTradeRow->getAmount();
			row["BuySell"] = closedTradeRow->getBuySell();

			row["OfferID"] = closedTradeRow->getOfferID();
			row["Instrument"] = Tools::OfferID2OfferName(this->pSession, closedTradeRow->getOfferID());
			//
			row["CloseOrderID"] = closedTradeRow->getCloseOrderID();
			row["CloseOrderParties"] = closedTradeRow->getCloseOrderParties();
			row["CloseOrderReqID"] = closedTradeRow->getCloseOrderReqID();
			row["CloseOrderRequestTXT"] = closedTradeRow->getCloseOrderRequestTXT();
			row["CloseQuoteID"] = closedTradeRow->getCloseQuoteID();
			row["CloseRate"] = closedTradeRow->getCloseRate();
			row["CloseTime"] = closedTradeRow->getCloseTime();
			//
			row["Commission"] = closedTradeRow->getCommission();
			row["GrossPL"] = closedTradeRow->getGrossPL();
			//
			row["OpenOrderID"] = closedTradeRow->getOpenOrderID();
			row["OpenOrderParties"] = closedTradeRow->getOpenOrderParties();
			row["OpenOrderReqID"] = closedTradeRow->getOpenOrderReqID();
			row["OpenOrderRequestTXT"] = closedTradeRow->getOpenOrderRequestTXT();
			row["OpenQuoteID"] = closedTradeRow->getOpenQuoteID();
			row["OpenRate"] = closedTradeRow->getOpenRate();
			DATE dt = Tools::ConvertUTC2Local(this->pSession, closedTradeRow->getOpenTime());
			string temp = "";
			Tools::formatDate(dt, temp);
			row["OpenTime"] = temp;

			row["RolloverInterest"] = closedTradeRow->getRolloverInterest();
			row["TableType"] = Tools::getO2GTableTypeName(closedTradeRow->getTableType());

			row["TradeIDOrigin"] = closedTradeRow->getTradeIDOrigin();
			row["TradeIDRemain"] = closedTradeRow->getTradeIDRemain();

			row["ValueDate"] = closedTradeRow->getValueDate();

			//
			rows.append(row);
		}
		// 发送给事件处理函数
		this->onQryClosed_TradesTable(rows);
	}
	catch (const std::exception& ee)
	{
		PRINTLINE(ee.what());
		return;
	}
}

void HxcmApi::processLogin(Task task)
{
	//PRINTLINE("call processLogin")
	PyLock lock;
	try
	{
		//int loginState = any_cast<int>(task.task_data);
		IO2GSessionStatus::O2GSessionStatus loginStatus = any_cast<IO2GSessionStatus::O2GSessionStatus>(task.task_data);
		boost::python::dict row;
		row["login_status"] = (int)loginStatus; // 不能直接将O2GSessionStatus类型赋值给dict，可能是dict只支持简单数据类型		
		this->onLogin(row);
	}
	catch (const std::exception& ee)
	{
		PRINTLINE(ee.what());
		return;
	}
}

///-------------------------------------------------------------------------------------
///回调函数，用于处理异步消息
///-------------------------------------------------------------------------------------
void HxcmApi::Login(bool IsBlock )
{

	if (!pSessionStatusListener->isConnected() )
	{
		//std::cout << "HxcmApi::Login() " << __LINE__ <<std::endl;
		pSession->login(UserName.c_str(), 
						PWD.c_str(), 
						URL.c_str(), 
						CONN.c_str());
		//首次登陆时应该等待登陆完成在返回，阻塞模式
		if (IsBlock == true)
		{
			if (pSessionStatusListener->waitEvents() && pSessionStatusListener->isConnected())
			{
				// 加载TableManager
				O2G2Ptr<IO2GTableManager> tableManager = this->pSession->getTableManager();
				O2GTableManagerStatus managerStatus = tableManager->getStatus();
				while (managerStatus == TablesLoading)
				{
					Sleep(10);
					managerStatus = tableManager->getStatus();
				}

				if (managerStatus == TablesLoadFailed)
				{
					std::cout << "Cannot refresh all tables of table manager" << std::endl;
				}
				return;
			}
		}		
	}
	
}

void HxcmApi::Logout()
{
	//注销ResponseListener中的消息回调
	pSession->unsubscribeResponse(pResponseListener);
	pResponseListener->release();

	//注销SessionStatusListener中的消息回调
	pSession->unsubscribeSessionStatus(pSessionStatusListener);
	//重置被引用计数器
	pSessionStatusListener->reset();

	pSession->logout();

	//等待相应消息处理完
	//pSessionStatusListener->waitEvents();
	//pResponseListener->waitEvents();

}

//查询历史价格//
bool HxcmApi::qryHisPrices(string instrument, string stimeFrame, int maxBars,string sbeginDate,string sEndDate)
{
	std::cout << instrument << " " << stimeFrame << " " << sbeginDate << " " << sEndDate << "  " << __LINE__ << std::endl;

	// 处理fromDate，toDate为0的情况
	//如果beginDate和EndDate 等于0，即为当下
	//时间转换
	DATE beginDate = -1 ;
	DATE endDate = -1;
	try
	{
		beginDate = atof(sbeginDate.c_str());
		std::cout << beginDate <<" : "<< sbeginDate<<" " << __LINE__ << std::endl;
		if (beginDate != 0.0)
		{
			beginDate = double(Tools::String2OleDateTime(sbeginDate.c_str()));
			std::cout << "line no =" << __LINE__ << std::endl;
		}

		endDate = atof(sEndDate.c_str());
		//std::cout << endDate << " " << __LINE__ << std::endl;
		if (endDate != 0.0)
		{
			std::cout << sEndDate << " " << __LINE__ <<std::endl;
			endDate = double(Tools::String2OleDateTime(sEndDate.c_str()));
		}

	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << " " << __LINE__<< std::endl;
	}
	try
	{
		std::cout << "beginDate = " << beginDate << "  endDate = " << endDate << " LineNo = " << __LINE__ << std::endl;
		//时间转换：从local 转换为 UTC
		beginDate = Tools::ConvertLocal2UTC(this->pSession, beginDate);
		endDate = Tools::ConvertLocal2UTC(this->pSession, endDate);
		//PRINTLINE("");
		// 4. Create IO2GRequestFactory:
		O2G2Ptr<IO2GRequestFactory> factory = this->pSession->getRequestFactory();
		if (!factory)
		{
			std::cout << "Cannot create request factory" << std::endl;
			return false;
		}

		//5. Get IO2GTimeframeCollection:
		O2G2Ptr<IO2GTimeframeCollection> timeframeCollection = factory->getTimeFrameCollection();
		/*
		//观察有多少种TimeFrame
		for (int i = 0; i < timeframeCollection->size(); i++)
		{
			std::cout << "Timeframe = " << timeframeCollection->get(i)->getUnit() << " " << std::endl;
		}
		*/
		// 6. Get the IO2GTimeframe you need:
		O2G2Ptr<IO2GTimeframe> timeFrame = timeframeCollection->get(stimeFrame.c_str());
		
		if (!timeFrame)
		{
			std::cout << "Timeframe '" << stimeFrame << "' is incorrect!" << std::endl;
			return false;
		}

		// 7. Create a market data snapshot request (IO2GRequest) 
		//using the instrument, time frame, and maximum number of bars as arguments:

		O2G2Ptr<IO2GRequest> request = factory->createMarketDataSnapshotRequestInstrument(
			instrument.c_str(),
			timeFrame,
			maxBars);
		if (!request)
		{
			std::cout << "Cannot create request : " 
				<< instrument.c_str() << " "
				<< timeFrame << " "
				<< maxBars << " "
				<<__LINE__<< std::endl;
			return false;
		}

		// 记录该请求的相关信息到map中
		sFxcmRequestData reqData;
		reqData.instrument = instrument;
		reqData.stimeFrame = stimeFrame;
		reqData.beginDate = beginDate;
		reqData.endDate = endDate;
		reqData.getNums = 0;//已接受数据条数
		pResponseListener->mRequestDataSet[string(request->getRequestID())] = reqData;

		// 8.Fill the market data snapshot request 
		//using the request, date and time "from", and date and time "to" as arguments

		//非阻塞模式的实现，与ResponseListener配合，该模式下只需要发送请求就返回

		//填充request
		factory->fillMarketDataSnapshotRequestTime(request, beginDate, endDate, false);
		//记录request id，没有用处
		pResponseListener->setRequestID(request->getRequestID());
		pSession->sendRequest(request);
	}
	catch (const std::exception&e)
	{
		std::cout << e.what() << " " << __LINE__ << std::endl;
	}
	
}
// 查询最近的1笔历史数据
bool HxcmApi::qryLastHisPrice(string instrument, string stimeFrame)
{
	//开始时间设定为：当前时间 + 一天
	DATE beginDate = COleDateTime::GetCurrentTime();
	COleDateTimeSpan ds;
	ds.SetDateTimeSpan(1, 0, 0, 0);
	beginDate = beginDate + ds;//时间加上1天,确保大于当前时间
	//将本地时间转换为UTC时间,在qryHisPrices中统一转换
	//beginDate = Tools::ConvertLocal2UTC(this->pSession,beginDate);

	//std::cout << beginDate << " Line no = " << __LINE__ << std::endl;
	string sBeginDate;
	Tools::formatDate(beginDate, sBeginDate);
	//std::cout << "beginDate :" <<sBeginDate<< "   " << beginDate <<" Line no = "<< __LINE__ <<std::endl;
	return qryHisPrices(instrument, stimeFrame, 100, sBeginDate, "0");
}



//定义货币对的offer信息
void HxcmApi::Subscribe(string instrument,string status,bool isInstrumentName=true)
{
	//PRINTLINE("HxcmApi::Subscribe");
	if (!this->pSessionStatusListener->isConnected())
	{
		//"与交易服务器的链接已经断开，无法订阅货币对的交易信息。",不能是中文
		//this->sendMessage("unconnected with fxcm server ,can not subscribe instrument offers info");
		PRINTLINE("unconnected with fxcm server, can not subscribe instrument :  " + instrument);
		return;
 	}
	//PRINTLINE("HxcmApi::Subscribe " + instrument);
	//是否为货币对名称，即“USD/CNH”,如果是则要查询其OfferID
	if (isInstrumentName)
	{
		//true为打印货币对状态清单，用于调试用
		PrintOfferSubscribeStatus(pSession, instrument.c_str(),true );
		//instrument = offer->getOfferID();
	}
	O2G2Ptr<IO2GOfferRow> offer = Tools::GetOffer(this->pSession, instrument);
	if (offer)
	{
		instrument = offer->getOfferID();
	}
	else
	{
		PRINTLINE("can not get " + instrument + " offerID ");
		return;
	}
	
	//PRINTLINE( "instrument = " + instrument );
	//1. 生成订阅请求
	//获取RequestFactory
	O2G2Ptr<IO2GRequestFactory> requestFactory = this->pSession->getRequestFactory();
	//
	if (!requestFactory)
	{
		std::cout << "Cannot create request factory" << std::endl;
		return;
	}
	O2G2Ptr<IO2GValueMap> valuemap = requestFactory->createValueMap();
	valuemap->setString(Command, O2G2::Commands::SetSubscriptionStatus);
	valuemap->setString(SubscriptionStatus, status.c_str());
	valuemap->setString(OfferID, instrument.c_str());
	O2G2Ptr<IO2GRequest> request = requestFactory->createOrderRequest(valuemap);
	if (!request)
	{
		std::cout << requestFactory->getLastError() << std::endl;
		return ;
	}	

	//2. 发送订阅请求
	pResponseListener->setRequestID(request->getRequestID());
	pSession->sendRequest(request);
}

//启动OnTick事件
void HxcmApi::StartTick(string timeFrame)
{
	PRINTLINE("Call HxcmApi::StartTick");
	//如果timeFrame对应的线程不存在
	if (this->TickThread.count(timeFrame) == 0)
	{
		function1<void, string> f = boost::bind(&HxcmApi::qryTickData, this, timeFrame);
		thread t( f,timeFrame );
		this->TickThread[timeFrame] = &t;
	}
	else
	{
		PRINTLINE("need not call StartTick again")
		string s1 = "need not call StartTick: ";// " again";
		s1 += timeFrame;
		s1 += " again";
		sendMessage(s1);
	}
}

//线程函数,功能定时查询

void HxcmApi::qryTickData(string strTimeFrame)
{
	int times = 60;
	if (strTimeFrame == "m1")
	{
		times = 60;
	}
	else if(strTimeFrame == "m5")
	{
		times = 5 * 60;
	}
	else if (strTimeFrame == "m10" )
	{
		times = 10 * 60;
	}
	else if (strTimeFrame == "m15")
	{
		times = 15 * 60;
	}

	int maxBars = 1;

	// 4. Create IO2GRequestFactory:
	O2G2Ptr<IO2GRequestFactory> factory = pSession->getRequestFactory();
	if (!factory)
	{
		std::cout << "Cannot create request factory" << std::endl;
		return;
	}

	//5. Get IO2GTimeframeCollection:
	O2G2Ptr<IO2GTimeframeCollection> timeframeCollection = factory->getTimeFrameCollection();

	// 6. Get the IO2GTimeframe you need:
	O2G2Ptr<IO2GTimeframe> timeFrame = timeframeCollection->get("m1");

	// 7. Create a market data snapshot request (IO2GRequest) 
	O2G2Ptr<IO2GRequest> request = NULL;

	while (1)
	{		
		//时间间隔
		int sleeptime = 1 * times * 1000; // 
		boost::timer::cpu_timer cts;
		std::list<string>::iterator iter = Tick_instruments[strTimeFrame].begin();
		//对每个货币对进行tick数据查询
		while (iter != Tick_instruments[strTimeFrame].end())
		{
			string instrument = *iter;
			//获取当前时间
			DATE now = COleDateTime::GetCurrentTime();
			// beginDate 大于当前时间1天
			COleDateTimeSpan ds;
			ds.SetDateTimeSpan(1, 0, 0, 0);
			DATE beginDate = now + ds;
			// endDate 大于当前时间2天
			ds.SetDateTimeSpan(2, 0, 0, 0);
			DATE endDate = now + ds;

			//时间转换：从local 转换为 UTC
			beginDate = Tools::ConvertLocal2UTC(pSession, beginDate);
			endDate = Tools::ConvertLocal2UTC(pSession, endDate);
				
			// 构造查询
			request = factory->createMarketDataSnapshotRequestInstrument(instrument.c_str(),
																				timeFrame,
																				maxBars);

			// 记录该请求的相关信息到map中
			sFxcmRequestData reqData;
			reqData.instrument = instrument;
			reqData.stimeFrame = strTimeFrame;
			reqData.beginDate = beginDate;
			reqData.endDate = endDate;
			reqData.getNums = 0;//已接受数据条数
			pResponseListener->mRequestDataSet[string(request->getRequestID())] = reqData;

			// 8.填充 the market data snapshot request 
			factory->fillMarketDataSnapshotRequestTime(request, beginDate, endDate, false);

			pSession->sendRequest(request);
			//======================================================================================
			iter++;
		}

		boost::timer::cpu_times tt = cts.elapsed();//时间单位均为纳秒
		float ff = sleeptime - tt.wall / 1000000;
		Sleep(ff);
	}

}

void HxcmApi::regTick(boost::python::dict ticks)
{
	
	boost::python::list  keylist = ticks.keys();
	for (int i = 0; i < len(keylist); i++)
	{
		//提取tick周期,m1,h1等
		string tickPeriod = boost::python::extract<std::string>(keylist[i]);
		std::list<string> currencyPairs;

		//提取key对应的货币对list，['EUR/USD','USD/JPY']
		boost::python::list valueList = boost::python::extract<boost::python::list>(ticks.values()[i]);
		for (int i = 0; i < len(valueList); i++)
		{
			string currencyPair = boost::python::extract<std::string>(valueList[i]);
			currencyPairs.push_back(currencyPair);
		}
		Tick_instruments[tickPeriod] = currencyPairs;

	}
}
// 注意dict["xxx"]中要用双引号
//accountId:账户id，不是登陆名
dict HxcmApi::qryAccount(string accountId)
{

	dict result;
	Account * account = Account::Instance(this->pSession, accountId);
	if (account->update())
	{
		result["AccountID"] = account->AccountID;	
		result["AccountKind"] = account->AccountKind;
		result["AccountName"] = account->AccountName;
		result["AmountLimit"] = account->AmountLimit;
		result["ATPID"] = account->ATPID;
		result["Balance"] = account->Balance ;
		result["BaseUnitSize"] = account->BaseUnitSize;
		result["LastMarginCallDate"] = account->LastMarginCallDate;
		result["LeverageProfileID"] = account->LeverageProfileID;
		result["M2MEquity"] = account->M2MEquity;
		result["MaintenanceFlag"] = account->MaintenanceFlag;
		result["MaintenanceType"] = account->MaintenanceType;
		result["ManagerAccountID"] = account->ManagerAccountID;
		result["MarginCallFlag"] = account->MarginCallFlag;
		result["NonTradeEquity"] = account->NonTradeEquity;
	}
	return result;
}

// 查询货币对的基本信息
boost::python::list HxcmApi::qryInstrumentInfo()
{
	boost::python::list result;
	// 查询所有的货币对
	O2G2Ptr<IO2GLoginRules> loginRules = this->pSession->getLoginRules();
	if (loginRules)
	{
		O2G2Ptr<IO2GResponse> response = loginRules->getTableRefreshResponse(Offers);
		O2G2Ptr<IO2GTradingSettingsProvider> settingsProvider = loginRules->getTradingSettingsProvider();
		O2G2Ptr<IO2GAccountRow> account = Tools::GetAccount(this->pSession, this->AccountID);
		//PRINTLINE(this->AccountID);
		//PRINTLINE(account->getAccountName());
		if (response)
		{
			O2G2Ptr<IO2GResponseReaderFactory> readerFactory = this->pSession->getResponseReaderFactory();
			if (readerFactory)
			{
				O2G2Ptr<IO2GOffersTableResponseReader> reader = readerFactory->createOffersTableReader(response);
				// 对每一个货币对查询其基本信息
				for (int i = 0; i < reader->size(); ++i)
				{
					boost::python::dict row;					
					O2G2Ptr<IO2GOfferRow> offer = reader->getRow(i);

					row["Instrument"] = offer->getInstrument();
					row["OfferID"] = offer->getOfferID();
					row["TradingStatus"] = offer->getTradingStatus(); //交易状态，O 	Open， C  Close
					row["Digits"] = offer->getDigits();
					row["BuyInterest"] = offer->getBuyInterest();
					row["SellInterest"] = offer->getSellInterest();

					result["DividendBuy"] = offer->getDividendBuy();
					result["DividendSell"] = offer->getDividendSell();
					result["PointSize"] = offer->getPointSize();
					result["SubscriptionStatus"] = offer->getSubscriptionStatus();
					result["QuoteID"] = offer->getQuoteID();
					//

					row["BaseUnitSize"] = settingsProvider->getBaseUnitSize(offer->getInstrument(), account);
					row["CondDistEntryLimit"] = settingsProvider->getCondDistEntryLimit(offer->getInstrument());
					row["CondDistEntryStop"] = settingsProvider->getCondDistEntryStop(offer->getInstrument());
					row["CondDistLimitForTrade"] = settingsProvider ->getCondDistLimitForTrade(offer->getInstrument());
					row["CondDistStopForTrade"] = settingsProvider->getCondDistStopForTrade(offer->getInstrument());
					double mml;
					double eml;
					double lmr;
					if (settingsProvider->getMargins(offer->getInstrument(), account, mml, eml, lmr))
					{
						row["mml"] = mml;
						row["eml"] = eml;
						row["lmr"] = lmr;

					}
					O2GMarketStatus marketStatus = settingsProvider->getMarketStatus(offer->getInstrument());
					switch (marketStatus)
					{
					case O2GMarketStatus::MarketStatusClosed:
					{
						row["MarketStatus"] = "MarketStatusClosed";
						break;
					}						
					case O2GMarketStatus::MarketStatusOpen:
					{
						row["MarketStatus"] = "MarketStatusOpen";
						break;
					}			
					case O2GMarketStatus::MarketStatusUndefined:
					{
						row["MarketStatus"] = "MarketStatusUndefined";
						break;
					}
					default:
						row["MarketStatus"] = "MarketStatusUnKnown ";
						break;
					}
					row["MaxQuantity"] = settingsProvider->getMaxQuantity(offer->getInstrument(), account);
					row["MaxTrailingStep"] = settingsProvider->getMaxTrailingStep();
					row["MinQuantity"] = settingsProvider->getMinQuantity(offer->getInstrument(), account);
					row["MinTrailingStep"] = settingsProvider->getMinTrailingStep();
					row["MMR"] = settingsProvider->getMMR(offer->getInstrument(), account);
					result.append(row);
				}
			}
		}
	}
	return result;
}

boost::python::dict HxcmApi::qryInstrumentRealtimeInfo(string instrument)
{
	O2G2Ptr<IO2GOfferRow> offer = Tools::getOfferRow(this->pSession->getTableManager(), instrument);
	boost::python::dict result;
	if (offer)
	{
		result["OfferID"] = offer->getOfferID();
		result["Instrument"] = offer->getInstrument();
		result["Ask"] = offer->getAsk();
		result["Bid"] = offer->getBid();
		result["AskExpireDate"] = Tools::Date2String( offer->getAskExpireDate() );
		result["BidExpireDate"] = Tools::Date2String( offer->getBidExpireDate() );
		result["BidTradabl"] = offer->getBidTradable();
		result["AskTradable"] = offer->getAskTradable();
		result["Digits"] = offer->getDigits();
		result["BuyInterest"] = offer->getBuyInterest();
		result["SellInterest"] = offer->getSellInterest();
		result["High"] = offer->getHigh();
		result["Low"] = offer->getLow();
		result["DividendBuy"] = offer->getDividendBuy();
		result["DividendSell"] = offer->getDividendSell();
		result["Volume"] = offer->getVolume();
		result["PointSize"] = offer->getPointSize();
		result["Time"] = offer->getTime();
		result["TradingStatus"] = offer->getTradingStatus();
		result["ValueDate"] = offer->getValueDate();
		result["SubscriptionStatus"] = offer->getSubscriptionStatus();
		result["QuoteID"] = offer->getQuoteID();
	}
	return result;
}

// 查询货币对交易设置
boost::python::dict HxcmApi::qryTradingSettings(string instrument)
{
	boost::python::list result;
	// 查询所有的货币对
	O2G2Ptr<IO2GLoginRules> loginRules = this->pSession->getLoginRules();
	if (loginRules)
	{
		O2G2Ptr<IO2GTradingSettingsProvider> settingsProvider = loginRules->getTradingSettingsProvider();
		O2G2Ptr<IO2GAccountRow> account = Tools::GetAccount(this->pSession, this->AccountID);
		boost::python::dict row;
		double mml;
		double eml;
		double lmr;
		if (settingsProvider->getMargins(instrument.c_str(), account, mml, eml, lmr))
		{
			row["mml"] = mml;
			row["eml"] = eml;
			row["lmr"] = lmr;

		}
		O2GMarketStatus marketStatus = settingsProvider->getMarketStatus(instrument.c_str());
		switch (marketStatus)
		{
		case O2GMarketStatus::MarketStatusClosed:
		{
			row["MarketStatus"] = "MarketStatusClosed";
			break;
		}
		case O2GMarketStatus::MarketStatusOpen:
		{
			row["MarketStatus"] = "MarketStatusOpen";
			break;
		}
		case O2GMarketStatus::MarketStatusUndefined:
		{
			row["MarketStatus"] = "MarketStatusUndefined";
			break;
		}
		default:
			row["MarketStatus"] = "MarketStatusUnKnown ";
			break;
		}
		row["MaxQuantity"] = settingsProvider->getMaxQuantity(instrument.c_str(), account);
		row["MaxTrailingStep"] = settingsProvider->getMaxTrailingStep();
		row["MinQuantity"] = settingsProvider->getMinQuantity(instrument.c_str(), account);
		row["MinTrailingStep"] = settingsProvider->getMinTrailingStep();
		row["MMR"] = settingsProvider->getMMR(instrument.c_str(), account);
		return row;
	}
}

//instrument:货币对名称
void HxcmApi::qryPosition(string instrument)
{
	// 查询
	O2G2Ptr<IO2GRequestFactory> requestFactory = this->pSession->getRequestFactory();
	if (requestFactory)
	{
		sFxcmRequestData qryData;
		qryData.requestType = O2GResponseType::GetTrades;
		qryData.instrument = instrument;
		O2G2Ptr<IO2GRequest> request = requestFactory->createRefreshTableRequest(O2GTable::Trades);
		this->pResponseListener->mRequestDataSet[request->getRequestID()] = qryData;
		this->pSession->sendRequest(request);
	}


}

// 查询已关闭的交易
void HxcmApi::qryClosed_TradesTable()
{
	O2G2Ptr<IO2GRequestFactory> requestFactory = this->pSession->getRequestFactory();
	if (requestFactory)
	{
		O2G2Ptr<IO2GRequest> request = requestFactory->createRefreshTableRequest(O2GTable::ClosedTrades);
		this->pSession->sendRequest(request);
	}
}
///-------------------------------------------------------------------------------------
///Send 订单，关闭交易
///-------------------------------------------------------------------------------------

//发送OpenMarketOrder
int HxcmApi::SendOpenMarketOrder(string symbol,//货币对的名称，例如"EUR/USD"
								string accountID,	//账号ID
								string BuyOrSell,	// B 买，S 卖
								int	  Amount,
								//string TimeInForce,	// IOC,FOK
								double ClientRate,
								string CustomID
							)
{
	PRINTLINE("Begin call SendOpenMarketOrder. symbol = " + symbol
		+ " accountID = " + accountID
		+ " BuyOrSell = " + BuyOrSell);
	O2G2Ptr<IO2GRequestFactory> factory = this->pSession->getRequestFactory();
	if (!factory)
	{
		PRINTLINE("Cannot create request factory");
		return NULL;
	}

	IO2GTimeConverter *timeConverter = this->pSession->getTimeConverter();
	using namespace O2G2;
	// 填充命令
	O2G2Ptr<IO2GValueMap> valuemap = factory->createValueMap();
	valuemap->setString(O2GRequestParamsEnum::Command, Commands::CreateOrder);
	valuemap->setString(O2GRequestParamsEnum::OrderType, Orders::TrueMarketOpen);
	valuemap->setString(O2GRequestParamsEnum::AccountID, accountID.c_str());            // The identifier of the account the order should be placed for.
	valuemap->setString(O2GRequestParamsEnum::OfferID, Tools::GetOffer(this->pSession,symbol)->getOfferID());                // The identifier of the instrument the order should be placed for.
	valuemap->setString(O2GRequestParamsEnum::BuySell, BuyOrSell.c_str());                // The order direction: "S" for "Sell", "B" for "Buy".
	valuemap->setInt(O2GRequestParamsEnum::Amount, Amount);                    // The quantity of the instrument to be bought or sold.
	//valuemap->setDouble(O2GRequestParamsEnum::ClientRate, ClientRate);
	valuemap->setString(O2GRequestParamsEnum::CustomID, CustomID.c_str());    // The custom identifier of the order.
	//时间转换为UTC时间
	//DATE dtTimeInForce = timeConverter->convert(Tools::String2OleDateTime(TimeInForce.c_str()), IO2GTimeConverter::Local, IO2GTimeConverter::UTC);
	//valuemap->setDouble(O2GRequestParamsEnum::TimeInForce, dtTimeInForce);


	O2G2Ptr<IO2GRequest> request = factory->createOrderRequest(valuemap);
	if (!request)
	{
		PRINTLINE(factory->getLastError());
		return NULL;
	}
	try
	{
		PRINTLINE("will sendRequest ")
		this->pSession->sendRequest(request);
	}
	catch (const std::exception& ee)
	{
		PRINTLINE(ee.what());
		return -1;
	}		
	return 1;		

}
// 以市场价格关闭仓位
void HxcmApi::SendCloseMarketOrder(string tradeId)
{
	O2G2Ptr<IO2GRequestFactory> requestFactory = this->pSession->getRequestFactory();
	if (!requestFactory)
	{
		PRINTLINE("can not create request factory!");
		return;
	}
	O2G2Ptr<IO2GTradeRow> trade = Tools::getTradeRow(this->pSession->getTableManager(), tradeId.c_str());
	O2G2Ptr<IO2GLoginRules> loginRules = this->pSession->getLoginRules();
	O2G2Ptr<IO2GPermissionChecker> permissionChecker = loginRules->getPermissionChecker();
	// 准备构造请求的参数
	O2G2Ptr<IO2GValueMap> valuemap = requestFactory->createValueMap();
	valuemap->setString(Command, O2G2::Commands::CreateOrder);
	// 检查是否有权执行市场价CloseOrder
	string instrument = Tools::OfferID2OfferName(this->pSession, string(trade->getOfferID()));
	if (permissionChecker->canCreateMarketCloseOrder( instrument.c_str() ) != PermissionEnabled ) 
	{
		// 如果没有权限，则执行一个方向的开单
		valuemap->setString(OrderType, O2G2::Orders::TrueMarketOpen);
	}
	else
	{
		valuemap->setString(OrderType, O2G2::Orders::TrueMarketClose);
		valuemap->setString(TradeID, tradeId.c_str());
	}
	valuemap->setString(O2GRequestParamsEnum::AccountID, trade->getAccountID());
	valuemap->setString(O2GRequestParamsEnum::OfferID, trade->getOfferID());
	// 设置购买方向，和原单相反
	valuemap->setString(O2GRequestParamsEnum::BuySell, strcmp(trade->getBuySell(), O2G2::Buy) == 0 ? O2G2::Sell: O2G2::Buy);
	valuemap->setInt(O2GRequestParamsEnum::Amount, trade->getAmount());
	valuemap->setString(O2GRequestParamsEnum::CustomID, CustomID_smart);

	// 发送请求
	O2G2Ptr<IO2GRequest> request = requestFactory->createOrderRequest(valuemap);
	if (request)
	{
		this->pSession->sendRequest(request);
	}
}

// 关闭指定货币对的全部仓位
void HxcmApi::SendCloseAllPositionsByInstrument(string instrument)
{
	PRINTLINE("Call HxcmApi::CloseAllPositionsByInstrument");
	try
	{
		O2G2Ptr<IO2GRequestFactory> requestFactory = this->pSession->getRequestFactory();
		string offerid = Tools::GetOffer(this->pSession, instrument)->getOfferID();
		O2G2Ptr<IO2GAccountRow> account = Tools::GetAccount(this->pSession, this->AccountID);
		if (account && requestFactory)
		{
			O2G2Ptr<IO2GOfferRow> offer = Tools::GetOffer(this->pSession, instrument);
			if (offer)
			{
				CloseOrderData  closeOrderData;
				O2G2Ptr<IO2GTradesTable> tradesTable = (IO2GTradesTable *)this->pSession->getTableManager()->getTable(Trades);

				if (tradesTable)
				{
					// 遍历tradesTable，如果同时有Buy与Sell仓位，则side = Both
					bool bIsTradesFound = false;
					for (int i = 0; i < tradesTable->size(); ++i)
					{
						O2G2Ptr<IO2GTradeRow> trade = tradesTable->getRow(i);
						if (offerid != string(trade->getOfferID())
							|| this->AccountID != string(trade->getAccountID()))
						{
							continue;
						}
						bIsTradesFound = true;
						std::string sBuySell = trade->getBuySell();
						// Set opposite side
						OrderSide side = (sBuySell == O2G2::Buy) ? Sell : Buy;
						if (closeOrderData.offerID != offerid)
						{
							closeOrderData.offerID = offerid;
							closeOrderData.account = this->AccountID;
							closeOrderData.side = side;
						}
						else
						{
							OrderSide currentSide = closeOrderData.side;
							if (currentSide != Both && currentSide != side)
								closeOrderData.side = Both;
						}
					}

					// 如果有指定货币对的trade，则建立CloseMarketNettingOrderRequest
					if (bIsTradesFound)
					{
						
						O2G2Ptr<IO2GValueMap> batchValuemap = requestFactory->createValueMap();
						batchValuemap->setString(Command, O2G2::Commands::CreateOrder);

						std::string sOfferID = closeOrderData.offerID;
						std::string sAccountID = closeOrderData.account;
						OrderSide side = closeOrderData.side;

						std::string sOrderType = O2G2::Orders::TrueMarketClose;
						// 确定订单的方向
						switch (side)
						{
						case Buy:
						{
							O2G2Ptr<IO2GValueMap> childValuemap = requestFactory->createValueMap();
							childValuemap->setString(Command, O2G2::Commands::CreateOrder);
							childValuemap->setString(NetQuantity, "Y");
							childValuemap->setString(OrderType, sOrderType.c_str());
							childValuemap->setString(O2GRequestParamsEnum::AccountID, this->AccountID.c_str());
							childValuemap->setString(OfferID, sOfferID.c_str());
							childValuemap->setString(BuySell, O2G2::Buy);
							batchValuemap->appendChild(childValuemap);
						}
						break;
						case Sell:
						{
							O2G2Ptr<IO2GValueMap> childValuemap = requestFactory->createValueMap();
							childValuemap->setString(Command, O2G2::Commands::CreateOrder);
							childValuemap->setString(NetQuantity, "Y");
							childValuemap->setString(OrderType, sOrderType.c_str());
							childValuemap->setString(O2GRequestParamsEnum::AccountID, this->AccountID.c_str());
							childValuemap->setString(OfferID, sOfferID.c_str());
							childValuemap->setString(BuySell, O2G2::Sell);
							batchValuemap->appendChild(childValuemap);
						}
						break;
						case Both:
						{
							O2G2Ptr<IO2GValueMap> buyValuemap = requestFactory->createValueMap();
							buyValuemap->setString(Command, O2G2::Commands::CreateOrder);
							buyValuemap->setString(NetQuantity, "Y");
							buyValuemap->setString(OrderType, sOrderType.c_str());
							buyValuemap->setString(O2GRequestParamsEnum::AccountID, this->AccountID.c_str());
							buyValuemap->setString(OfferID, sOfferID.c_str());
							buyValuemap->setString(BuySell, O2G2::Buy);
							batchValuemap->appendChild(buyValuemap);

							O2G2Ptr<IO2GValueMap> sellValuemap = requestFactory->createValueMap();
							sellValuemap->setString(Command, O2G2::Commands::CreateOrder);
							sellValuemap->setString(NetQuantity, "Y");
							sellValuemap->setString(OrderType, sOrderType.c_str());
							sellValuemap->setString(O2GRequestParamsEnum::AccountID, this->AccountID.c_str());
							sellValuemap->setString(OfferID, sOfferID.c_str());
							sellValuemap->setString(BuySell, O2G2::Sell);
							batchValuemap->appendChild(sellValuemap);
						}
						break;
						}
						// 构造Request
						IO2GRequest *request = requestFactory->createOrderRequest(batchValuemap);
						if (request)
						{
							this->pSession->sendRequest(request);
						}
					}
				}

			}
		}
	}
	catch (const std::exception& ee)
	{
		PRINTLINE(ee.what());
		return;
	}
	
}


// 创建Open Range Order 
void HxcmApi::SendOpenRangeOrder(string instrument, string BuyOrSell, int amount, double rateMin, double rateMax,string customID)
{
	PRINTLINE("Call SendOpenRangeOrder");
	O2G2Ptr<IO2GRequestFactory> factory = this->pSession->getRequestFactory();
	O2G2Ptr<IO2GLoginRules> loginRules = this->pSession->getLoginRules();
	O2G2Ptr<IO2GPermissionChecker> permissionChecker = loginRules->getPermissionChecker();
	if (permissionChecker->canCreateMarketOpenOrder(instrument.c_str()) != PermissionEnabled)
	{
		PRINTLINE("can not CreateMarketOpenOrder");
	}

	if (factory)
	{
		O2G2Ptr<IO2GValueMap> valuemap = factory->createValueMap();
		valuemap->setString(Command, O2G2::Commands::CreateOrder);
		valuemap->setString(OrderType, O2G2::Orders::MarketOpenRange);
		valuemap->setString(O2GRequestParamsEnum::AccountID, this->AccountID.c_str());       // The identifier of the account the order should be placed for.
		valuemap->setString(OfferID, Tools::GetOffer(this->pSession,instrument)->getOfferID());           // The identifier of the instrument the order should be placed for.
		valuemap->setString(BuySell, BuyOrSell.c_str());           // The order direction ("B" for buy, "S" for sell).
		valuemap->setDouble(RateMin, rateMin);           // The minimum rate at which the order can be filled.
		valuemap->setDouble(RateMax, rateMax);           // The maximum rate at which the order can be filled.
		valuemap->setString(CustomID, customID.c_str()); // The custom identifier of the order.
		valuemap->setInt(Amount, amount);

		O2G2Ptr<IO2GRequest> request = factory->createOrderRequest(valuemap);
		PRINTLINE("create request");
		if (!request)
		{
			PRINTLINE("Order's request can not be created: " + string(factory->getLastError()));
			return;
		}
		else
		{
			this->pSession->sendRequest(request);
			PRINTLINE("sendRequest");
			return;
		}
	}
}

// 创建Close Range Order，指明价格范围
void HxcmApi::SendCloseRangeOrderFloat(string tradeid, double minRate, double maxRate,string customID)
{
	using namespace O2G2;
	O2G2Ptr<IO2GRequestFactory> factory = this->pSession->getRequestFactory();
	O2G2Ptr<IO2GTradeRow> trade = Tools::getTradeRow(this->pSession->getTableManager(), tradeid.c_str());
	if (factory && trade)
	{
		O2G2Ptr<IO2GValueMap> valuemap = factory->createValueMap();
		valuemap->setString(Command, Commands::CreateOrder);
		valuemap->setString(OrderType, Orders::MarketCloseRange);
		valuemap->setString(O2GRequestParamsEnum::AccountID, this->AccountID.c_str());
		valuemap->setString(OfferID, trade->getOfferID());                // The identifier of the instrument the order should be placed for.
		valuemap->setString(TradeID, tradeid.c_str());                // The identifier of the trade to be closed.
		valuemap->setString(BuySell, strcmp(trade->getBuySell(), O2G2::Buy) == 0 ? O2G2::Sell : O2G2::Buy);                // The order direction ("B" for Buy, "S" for Sell). Must be opposite to the direction of the trade.
		valuemap->setInt(Amount, trade->getAmount());                    // The quantity of the instrument to be bought or sold. Must be <= size of the position (Lot of the trade). Must be divisible by baseUnitSize.
		valuemap->setDouble(RateMin, minRate);                // The minimum rate at which the order can be filled.
		valuemap->setDouble(RateMax, maxRate);                // The maximum rate at which the order can be filled.
		valuemap->setString(CustomID, customID.c_str());    // The custom identifier of the order.
		O2G2Ptr<IO2GRequest> request = factory->createOrderRequest(valuemap);
		if (!request)
		{
			PRINTLINE("CloseRangeOrder's request can not be created: " + string(factory->getLastError()));
		}
		else
		{
			this->pSession->sendRequest(request);
		}
	}

}

// 创建Close Range Order，指明浮动pips
void HxcmApi::SendCloseRangeOrderInt(string tradeid, int minPoint, int maxPoint, string customID)
{
	using namespace O2G2;
	O2G2Ptr<IO2GTradeRow> trade = Tools::getTradeRow(this->pSession->getTableManager(), tradeid.c_str());
	
	if (trade)
	{
		O2G2Ptr<IO2GOfferRow> offer = Tools::GetOffer(this->pSession, 
			                                          Tools::OfferID2OfferName( this->pSession,
														                        trade->getOfferID()
													                           )
		                                              );
		const char * tradeBuySell = trade->getBuySell();
		// Close order is opposite to the trade
		bool buyOrder = false;
		if (strcmp(tradeBuySell, O2G2::Buy) == 0)
			buyOrder = true;

		double rate = buyOrder ? offer->getAsk() : offer->getBid();

		double pointSize = offer->getPointSize();
		double rateMin = rate - minPoint * pointSize;
		double rateMax = rate + maxPoint * pointSize;

		SendCloseRangeOrderFloat(tradeid, rateMin, rateMax, customID);

	}
}

// 创建Create Open Limit Order 
void HxcmApi::SendOpenLimitOrder(string instrument, int amount, double rate, string buySell, string customID)
{
	using namespace O2G2;
	O2G2Ptr<IO2GRequestFactory> factory = this->pSession->getRequestFactory();
	if (factory)
	{
		O2G2Ptr<IO2GValueMap> valuemap = factory->createValueMap();
		valuemap->setString(Command, Commands::CreateOrder);
		valuemap->setString(OrderType, Orders::OpenLimit);
		valuemap->setString(O2GRequestParamsEnum::AccountID, this->AccountID.c_str());                // The identifier of the account the order should be placed for.
		valuemap->setString(OfferID, Tools::OfferName2offerId(this->pSession,instrument).c_str());                    // The identifier of the instrument the order should be placed for.
		valuemap->setString(BuySell, buySell.c_str());                    // The order direction (use "B" for Buy, "S" for Sell)
		valuemap->setDouble(Rate, rate);                          // The rate at which the order must be filled.
		valuemap->setInt(Amount, amount);                         // The quantity of the instrument to be bought or sold.
		valuemap->setString(CustomID,customID.c_str());         // The custom identifier of the order.

		O2G2Ptr<IO2GRequest> request = factory->createOrderRequest(valuemap);
		if (request)
			this->pSession->sendRequest(request);
		else
			PRINTLINE( "Order's request can not be created: " + string( factory->getLastError() ) );

	}

}

void HxcmApi::SendCloseLimitOrder(string tradeid, double rate, string customID)
{
	using namespace O2G2;
	O2G2Ptr<IO2GRequestFactory> factory = this->pSession->getRequestFactory();
	O2G2Ptr<IO2GTradeRow> trade = Tools::getTradeRow(this->pSession->getTableManager(), tradeid.c_str());
	if (factory && trade)
	{
		O2G2Ptr<IO2GValueMap> valuemap = factory->createValueMap();
		valuemap->setString(Command, Commands::CreateOrder);
		valuemap->setString(OrderType, Orders::CloseLimit);
		valuemap->setString(O2GRequestParamsEnum::AccountID, this->AccountID.c_str());                // The identifier of the account the order should be placed for.
		valuemap->setString(OfferID, trade->getOfferID());                    // The identifier of the instrument the order should be placed for.
		valuemap->setString(TradeID, tradeid.c_str());                    // The identifier of the trade to be closed.
		valuemap->setString(BuySell, strcmp(trade->getBuySell(), O2G2::Buy) == 0 ? O2G2::Sell : O2G2::Buy);                    // The order direction ("B" - for Buy, "S" - for Sell). Must be opposite to the direction of the trade.
		valuemap->setInt(Amount, trade->getAmount());                         // The quantity of the instrument to be bought or sold.  Must <= to the size of the position (Lot column of the trade).
		valuemap->setDouble(Rate, rate);                          // The rate at which the order must be filled.
		valuemap->setString(CustomID, customID.c_str());        // The custom identifier of the order.
		O2G2Ptr<IO2GRequest> request = factory->createOrderRequest(valuemap);
		if (request)
			this->pSession->sendRequest(request);
		else
			PRINTLINE("Order's request can not be created: " + string(factory->getLastError()));
	}

}

// 创建Entry Limit Order 
void HxcmApi::SendEntryLimitOrder(string instument, int amount, double rate, string buySell, string customID)
{
	using namespace O2G2;
	O2G2Ptr<IO2GRequestFactory> factory = this->pSession->getRequestFactory();
	if (factory)
	{
		O2G2Ptr<IO2GValueMap> valuemap = factory->createValueMap();
		valuemap->setString(Command, Commands::CreateOrder);
		valuemap->setString(OrderType, Orders::LimitEntry);
		valuemap->setString(O2GRequestParamsEnum::AccountID, this->AccountID.c_str());          // The identifier of the account the order should be placed for.
		valuemap->setString(OfferID, Tools::OfferName2offerId(this->pSession,instument).c_str());              // The identifier of the instrument the order should be placed for.
		valuemap->setString(BuySell, buySell.c_str());              // The order direction ("B" for buy, "S" for sell)
		valuemap->setDouble(Rate, rate);                  // The rate at which the order must be filled (below current rate for Buy, above current rate for Sell)
		valuemap->setInt(Amount, amount);                  // The quantity of the instrument to be bought or sold.
		valuemap->setString(CustomID, customID.c_str()); // The custom identifier of the order.

		O2G2Ptr<IO2GRequest> request = factory->createOrderRequest(valuemap);
		if (request)
			this->pSession->sendRequest(request);
		else
		{
			PRINTLINE("Entry Limit Order's request can not be created: " + string(factory->getLastError()));
		}
	}

}

//创建Entry Stop Order 
void HxcmApi::SendEntryStopOrder(string instument, int amount, double rate, string buySell, string customID)
{
	using namespace O2G2;
	O2G2Ptr<IO2GRequestFactory> factory = this->pSession->getRequestFactory();
	if (factory)
	{
		O2G2Ptr<IO2GValueMap> valuemap = factory->createValueMap();
		valuemap->setString(Command, Commands::CreateOrder);
		valuemap->setString(OrderType, Orders::StopEntry);
		valuemap->setString(O2GRequestParamsEnum::AccountID, this->AccountID.c_str());         // The identifier of the account the order should be placed for.
		valuemap->setString(OfferID, Tools::OfferName2offerId(this->pSession, instument).c_str());             // The identifier of the instrument the order should be placed for.
		valuemap->setString(BuySell, buySell.c_str());             // The order direction ("B" for buy, "S" for sell)
		valuemap->setDouble(Rate, rate);                 // The rate at which the order must be filled (above current rate for Buy, bellow current rate for Sell)
		valuemap->setInt(Amount, amount);                 // The quantity of the instrument to be bought or sold.
		valuemap->setString(CustomID, customID.c_str()); // The custom identifier of the order.

		O2G2Ptr<IO2GRequest> request = factory->createOrderRequest(valuemap);
		if (request )
			this->pSession->sendRequest(request); 
		else
		{
			PRINTLINE("Entry Stop Order 's request can not be created: " + string(factory->getLastError()));
		}
	}

}
// 创建delete order命令
void HxcmApi::SendDeleteOrder(string orderid, string customID)
{
	using namespace O2G2;
	O2G2Ptr<IO2GRequestFactory> factory = this->pSession->getRequestFactory();
	if (factory)
	{
		O2G2Ptr<IO2GValueMap> valuemap = factory->createValueMap();
		valuemap->setString(Command, Commands::DeleteOrder);
		valuemap->setString(OrderID, orderid.c_str());
		valuemap->setString(O2GRequestParamsEnum::AccountID, this->AccountID.c_str());
		valuemap->setString(CustomID, customID.c_str());
		O2G2Ptr<IO2GRequest> request = factory->createOrderRequest(valuemap);
		if (request)
			this->pSession->sendRequest(request);
		else
		{
			PRINTLINE("Delete Order 's request can not be created: " + string(factory->getLastError()));
		}
	}

}

// 创建Edit Order 
void HxcmApi::SendEditOrder(string orderid, int amount, double rate,int trailStep,string customID)
{
	using namespace O2G2;
	O2G2Ptr<IO2GRequestFactory> factory = this->pSession->getRequestFactory();
	if (factory)
	{
		O2G2Ptr<IO2GValueMap> valuemapChangeOrder = factory->createValueMap();
		valuemapChangeOrder->setString(Command, Commands::EditOrder);
		valuemapChangeOrder->setString(OrderID, orderid.c_str());
		valuemapChangeOrder->setString(O2GRequestParamsEnum::AccountID, this->AccountID.c_str());
		valuemapChangeOrder->setDouble(Rate, rate);
		valuemapChangeOrder->setInt(Amount, amount);
		valuemapChangeOrder->setInt(TrailStep, trailStep);
		valuemapChangeOrder->setString(CustomID, customID.c_str());

		O2G2Ptr<IO2GRequest> request = factory->createOrderRequest(valuemapChangeOrder);
		if (request)
			this->pSession->sendRequest(request);
		else
		{
			PRINTLINE("Edit Order's request can not be created: " + string(factory->getLastError()));
		}
	}

}

void HxcmApi::processTradesTableUpdate(Task task)
{
	PyLock lock;
	try
	{
		O2G2Ptr<IO2GTradeRow> tradeRow = any_cast<O2G2Ptr<IO2GTradeRow>>(task.task_data);
		boost::python::list rows;
		boost::python::dict row;

		row["TradeID"] = tradeRow->getTradeID();
		row["AccountID"] = tradeRow->getAccountID();
		row["AccountName"] = tradeRow->getAccountName();	
		
		row["Amount"] = tradeRow->getAmount();
		row["BuySell"] = tradeRow->getBuySell();		
		row["Commission"] = tradeRow->getCommission();
		row["OfferID"] = tradeRow->getOfferID();
		row["Instrument"] = Tools::OfferID2OfferName(this->pSession, tradeRow->getOfferID());
		row["OpenOrderID"] = tradeRow->getOpenOrderID();
		row["OpenOrderReqID"] = tradeRow->getOpenOrderReqID();
		row["OpenOrderRequestTXT"] = tradeRow->getOpenOrderRequestTXT();
		row["OpenQuoteID"] = tradeRow->getOpenQuoteID();
		row["OpenRate"] = tradeRow->getOpenRate();
		DATE dt = Tools::ConvertUTC2Local(this->pSession, tradeRow->getOpenTime());
		string temp = "";
		Tools::formatDate(dt, temp);
		row["OpenTime"] = temp;
		row["Parties"] = tradeRow->getParties();

		row["RolloverInterest"] = tradeRow->getRolloverInterest();
		row["TableType"] = Tools::getO2GTableTypeName(tradeRow->getTableType());
		
		row["TradeIDOrigin"] = tradeRow->getTradeIDOrigin();
		row["UsedMargin"] = tradeRow->getUsedMargin();
		row["ValueDate"] = tradeRow->getValueDate();

		//
		rows.append(row);

		// 发送给事件处理函数
		this->onTradesTableUpdate(rows);
	}
	catch (const std::exception& ee)
	{
		PRINTLINE(ee.what());
		return;
	}
}

///-------------------------------------------------------------------------------------
///Boost.Python封装，给python调用的函数
///-------------------------------------------------------------------------------------

struct HxcmApiWrap : HxcmApi, wrapper<HxcmApi>
{
	HxcmApiWrap() : HxcmApi() {};
	HxcmApiWrap(string  userName, string   pwd, string  url, string connection,string accountId) 
		: HxcmApi(userName,pwd,url,connection, accountId)
	{
	};
	virtual void onResGetHistoryPrices(dict data, bool last)
	{
		//PRINTLINE("HxcmApiWrap::onResGetHistoryPrices");
		try
		{
			this->get_override("onResGetHistoryPrices")(data, last);
		}
		catch (error_already_set const &)
		{
			PyErr_Print();
		}
	};
	virtual void onSubscribeInstrument(boost::python::list data)
	{
		//PRINTLINE("HxcmApiWrap::onSubscribeInstrument");
		try
		{
			this->get_override("onSubscribeInstrument")(data);
		}
		catch (error_already_set const &)
		{
			PyErr_Print();
		}
	};
	virtual void onMessage(boost::python::dict data)
	{
		//PRINTLINE("HxcmApiWrap::onMessage");
		try
		{
			this->get_override("onMessage")(data);
		}
		catch (error_already_set const &)
		{
			PyErr_Print();
		}
	};
	virtual void onQryPosition(boost::python::list data)
	{
		//PRINTLINE("HxcmApiWrap::onQryPosition");
		try
		{
			this->get_override("onQryPosition")(data);
		}
		catch (error_already_set const &)
		{
			PyErr_Print();
		}
	};
	virtual void onSendOrderResult(boost::python::list data)
	{
		//PRINTLINE("HxcmApiWrap::onSendOrderResult");
		try
		{
			this->get_override("onSendOrderResult")(data);
		}
		catch (error_already_set const &)
		{
			PyErr_Print();
		}
	};
	virtual void onAccountsUpdate(boost::python::list data)
	{
		//PRINTLINE("HxcmApiWrap::onAccountsUpdate");
		try
		{
			this->get_override("onAccountsUpdate")(data);
		}
		catch (error_already_set const &)
		{
			PyErr_Print();
		}
	};

	virtual void onClosedTradeTableUpdate(boost::python::list data)
	{
		//PRINTLINE("HxcmApiWrap::onClosedTradeTableUpdate");
		try
		{
			this->get_override("onClosedTradeTableUpdate")(data);
		}
		catch (error_already_set const &)
		{
			PyErr_Print();
		}
	};
	virtual void onTradesTableUpdate(boost::python::list data)
	{
		//PRINTLINE("HxcmApiWrap::onTradesTableUpdate");
		try
		{
			this->get_override("onTradesTableUpdate")(data);
		}
		catch (error_already_set const &)
		{
			PyErr_Print();
		}
	};

	virtual void onQryClosed_TradesTable(boost::python::list data)
	{
		//PRINTLINE("HxcmApiWrap::onQryClosed_TradesTable ");
		try
		{
			this->get_override("onQryClosed_TradesTable")(data);
		}
		catch (error_already_set const &)
		{
			PyErr_Print();
		}
	};
	virtual void onLogin(boost::python::dict data)
	{
		//PRINTLINE("HxcmApiWrap::onLogin ");
		try
		{
			this->get_override("onLogin")(data);
		}
		catch (error_already_set const &)
		{
			PyErr_Print();
		}
	};

};
/**/
// 注意python中继承类的构造函数也要相应变化，否则调试脚本会出错
BOOST_PYTHON_MODULE(hxcmapi)
{
	PyEval_InitThreads();	//导入时运行，保证先创建GIL
	class_<HxcmApiWrap, boost::noncopyable>("HxcmApi",init<>())
		.def(init<string,string,string, string,string>())
		.def("Login", &HxcmApiWrap::Login)
		.def("Logout", &HxcmApiWrap::Logout)
		.def("qryHisPrices",&HxcmApiWrap::qryHisPrices)
		.def("qryLastHisPrice",&HxcmApiWrap::qryLastHisPrice)
		.def("qryAccount",&HxcmApiWrap::qryAccount)
		.def("qryPosition",&HxcmApiWrap::qryPosition)
		.def("qryInstrumentInfo",&HxcmApiWrap::qryInstrumentInfo)
		.def("qryInstrumentRealtimeInfo",&HxcmApiWrap::qryInstrumentRealtimeInfo)
		.def("qryTradingSettings",&HxcmApiWrap::qryTradingSettings)
		.def("qryClosed_TradesTable",&HxcmApiWrap::qryClosed_TradesTable)
		.def("Subscribe",&HxcmApiWrap::Subscribe)
		.def("StartTick", &HxcmApiWrap::StartTick)
		.def("regTick",&HxcmApiWrap::regTick)
		.def("SendOpenMarketOrder",&HxcmApiWrap::SendOpenMarketOrder)
		.def("SendCloseMarketOrder",&HxcmApiWrap::SendCloseMarketOrder)
		.def("SendCloseAllPositionsByInstrument",&HxcmApiWrap::SendCloseAllPositionsByInstrument)
		.def("SendOpenRangeOrder",&HxcmApiWrap::SendOpenRangeOrder)
		.def("SendCloseRangeOrderFloat",&HxcmApiWrap::SendCloseRangeOrderFloat)
		.def("SendOpenLimitOrder",&HxcmApiWrap::SendOpenLimitOrder)
		.def("SendCloseLimitOrder",&HxcmApiWrap::SendCloseLimitOrder)
		.def("SendEntryLimitOrder",&HxcmApiWrap::SendEntryLimitOrder)
		.def("SendEditOrder",&HxcmApiWrap::SendEditOrder)
		.def("SendEntryStopOrder",&HxcmApiWrap::SendEntryStopOrder)
		.def("SendDeleteOrder",&HxcmApiWrap::SendDeleteOrder)
		.def("onResGetHistoryPrices", pure_virtual(&HxcmApiWrap::onResGetHistoryPrices))
		.def("onSubscribeInstrument",pure_virtual(&HxcmApiWrap::onSubscribeInstrument))
		.def("onMessage", pure_virtual(&HxcmApiWrap::onMessage))
		.def("onQryPosition", pure_virtual(&HxcmApiWrap::onQryPosition))
		.def("onSendOrderResult", pure_virtual(&HxcmApiWrap::onSendOrderResult))
		.def("onAccountsUpdate", pure_virtual(&HxcmApiWrap::onAccountsUpdate))
		.def("onClosedTradeTableUpdate", pure_virtual(&HxcmApiWrap::onClosedTradeTableUpdate))
		.def("onTradesTableUpdate", pure_virtual(&HxcmApiWrap::onTradesTableUpdate))
		.def("onQryClosed_TradesTable", pure_virtual(&HxcmApiWrap::onQryClosed_TradesTable))
		.def("onLogin", pure_virtual(&HxcmApiWrap::onLogin))
		;
};


