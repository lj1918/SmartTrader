#include "stdafx.h"
#include "ResponseListener.h"
#include "hxcmapi.h"
#include "Tools.h"
#include <boost/format.hpp>

using namespace std;
ResponseListener::~ResponseListener()
{
	if (mResponse)
		mResponse->release();
	mSession->release();
	CloseHandle(mResponseEvent);
}

ResponseListener::ResponseListener(IO2GSession * session, HxcmApi *api/**/)
{
	mSession = session;
	this->api = api;
	//std::cout << this->api << std::endl;
	mSession->addRef();
	mRefCount = 1;
	mResponseEvent = CreateEvent(0, FALSE, FALSE, 0);
	mRequestID = "";
	mOrderID = "";
	mResponse = NULL;
	//mRequestDataSet =map<
}

long ResponseListener::addRef()
{
	return InterlockedIncrement(&mRefCount);
}

long ResponseListener::release()
{
	long rc = InterlockedDecrement(&mRefCount);
	if (rc == 0)
		delete this;
	return rc;
}
/** Request execution completed data handler. 回调函数*/
void ResponseListener::onRequestCompleted(const char * requestId, IO2GResponse * response = 0)
{
	PRINTLINE("response->getType() = " + Tools::GetResponseType(response->getType())  );
	//std::cout << "requestId = " << requestId << std::endl;
	//是否还有再次查询
	bool needquestAgain = false;

	//如果是查询历史价格的响应信息,可以列出货币对订阅的情况，货币对实时交易信息在onTablesUpdates事件中处理
	if (response && response->getType() == O2GResponseType::MarketDataSnapshot)
	{
		//获取该响应对应请求的相关信息：账户、密码等
		sFxcmRequestData reqData = mRequestDataSet[string(requestId)];

		// 10. 获取 IO2GResponseReaderFactory:
		O2G2Ptr<IO2GResponseReaderFactory> readerFactory = mSession->getResponseReaderFactory();
		
		if (readerFactory)
		{			
			// 11. 创建 IO2GMarketDataSnapshotResponseReader:
			O2G2Ptr<IO2GMarketDataSnapshotResponseReader> reader = readerFactory->createMarketDataSnapshotReader(response);
			if (reader)
			{
				if (reader->size() > 0)
				{
					//已收到数据更新
					mRequestDataSet[requestId].getNums += reader->size();

					//构造通知消息
					try
					{
						boost::format fmessage = boost::format("This time [%s]  received : %s records, total got %s records")
							% mRequestDataSet[requestId].instrument
							% reader->size()
							% mRequestDataSet[requestId].getNums; //"查询历史价格："
						std::string message = fmessage.str();
						this->api->sendMessage(message);
					}
					catch (const std::exception&ee)
					{
						std::cout << ee.what() << __LINE__ << std::endl;
					}
					// 构造Task,发送数据给python终端
					Task task = Task();
					task.task_name = OnGetHisPrices_smart;
					task.instrument = mRequestDataSet[requestId].instrument;
					task.task_data = reader;
					//将response返回到api，在那里对数据进行遍历，并构造dict数据
					//不能在这里构造boost::python下的数据类型，会出错
					this->api->putTask(task);
					//std::cout << "  end send task " << __LINE__ << std::endl;
					//判断是否返回了所有历史数据，如果endDate为节假日，可能会有问题
					if (fabs(mRequestDataSet[requestId].endDate - reader->getDate(0)) > 0.0001 //返回数据中最新记录的日期是否等于endDate
																							   /*|| mRequestDataSet[requestId].maxBars > mRequestDataSet[requestId].getNums*/ //maxBarsw为单次返回的历史数据的最大笔数
						)
					{
						mRequestDataSet[requestId].endDate = reader->getDate(0); // 返回数据的最新的时间
						needquestAgain = true;

						// 继续查询历史数据
						if (needquestAgain)
						{
							//填充request
							O2G2Ptr<IO2GRequestFactory> factory = mSession->getRequestFactory();

							//5. 获取 IO2GTimeframeCollection:
							O2G2Ptr<IO2GTimeframeCollection> timeframeCollection = factory->getTimeFrameCollection();
							// 6. 获取 the IO2GTimeframe 从 mRequestDataSet中获取:
							O2G2Ptr<IO2GTimeframe> timeFrame = timeframeCollection->get(this->mRequestDataSet[requestId].stimeFrame.c_str());

							// 通过工程类创建request
							O2G2Ptr<IO2GRequest> request = factory->createMarketDataSnapshotRequestInstrument(
								mRequestDataSet[requestId].instrument.c_str(),//货币对名称
								timeFrame,	//时间跨度，如m1，m5，h1等
								mRequestDataSet[requestId].maxBars);	//单次最大返回数据数量，如果fromtime 到 totime中的数据超过maxbars，则分多次返回

							factory->fillMarketDataSnapshotRequestTime(
								request,								//请求
								mRequestDataSet[requestId].beginDate,	//from时间
								mRequestDataSet[requestId].endDate, false);	// to时间

																			//保存本次查询的相关信息	
							sFxcmRequestData reqData;
							reqData.instrument = mRequestDataSet[requestId].instrument;
							reqData.stimeFrame = mRequestDataSet[requestId].stimeFrame;
							reqData.beginDate = mRequestDataSet[requestId].beginDate;
							reqData.endDate = mRequestDataSet[requestId].endDate;
							reqData.getNums = 0;
							this->mRequestDataSet[string(request->getRequestID())] = reqData;
							//再次查询	
							mSession->sendRequest(request);


							string sBeginDate;
							string sEndDate;
							//0.0 会被转换为string“1899/12/30 00:00:00”
							Tools::formatDate(mRequestDataSet[requestId].beginDate, sBeginDate);
							Tools::formatDate(mRequestDataSet[requestId].endDate, sEndDate);

							try
							{
								boost::format fmessage = boost::format("Qry Historal Prices : %s from %s to %s")
									% mRequestDataSet[requestId].instrument
									% sBeginDate % sEndDate; //"查询历史价格："
								std::string message = fmessage.str();
								//std::cout << "Request Market Instrument prices : " << message << __LINE__ << std::endl;
								//api->sendMessage("req offer price again! req offer price again! req offer price again! req offer price again! req offer price again! req offer price again! ");
								api->sendMessage(message);//不行，原因是什么？居然是混入了一个汉字的标点符号！！！
							}
							catch (const std::exception&ee)
							{
								std::cout << ee.what() << __LINE__ << std::endl;
							}
						}
					}
				}
			}			
		}
		return;
	}
	// command request的响应
	if (response && response->getType() == O2GResponseType::GetOffers)  ///CommandResponse  
	{
		PRINTLINE("response && response->getType() == GetOffers");
		//打印offer 信息
		IO2GOfferRow *resultOffer = NULL;
		O2G2Ptr<IO2GLoginRules> loginRules = mSession->getLoginRules();
		if (loginRules)
		{
			//O2G2Ptr<IO2GResponse> rsp = loginRules->getTableRefreshResponse(Offers);
			//if (response)
			//{
				O2G2Ptr<IO2GResponseReaderFactory> readerFactory = mSession->getResponseReaderFactory();
				if (readerFactory)
				{
					// 读取response中的数据
					O2G2Ptr<IO2GOffersTableResponseReader> reader = readerFactory->createOffersTableReader(response);
					for (int i = 0; i < reader->size(); ++i)
					{
						O2G2Ptr<IO2GOfferRow> offer = reader->getRow(i);
						if (offer)
						{

							if (strcmp(offer->getSubscriptionStatus(), O2G2::SubscriptionStatuses::ViewOnly) == 0)
								printf("%s : [V]iew only\n", offer->getInstrument());
							else if (strcmp(offer->getSubscriptionStatus(), O2G2::SubscriptionStatuses::Disable) == 0)
								printf("%s : [D]isabled\n", offer->getInstrument());
							else if (strcmp(offer->getSubscriptionStatus(), O2G2::SubscriptionStatuses::Tradable) == 0)
								printf("%s : Available for [T]rade\n", offer->getInstrument());
							else
								printf("%s : %s\n", offer->getInstrument(), offer->getSubscriptionStatus());
							string instrument = offer->getInstrument();							
						}
					}
				}
			//}
		}
		return;
	}

	// 查询position请求的响应
	if (response && response->getType() == O2GResponseType::GetTrades)
	{
		O2G2Ptr<IO2GResponseReaderFactory> readerFactory = mSession->getResponseReaderFactory();
		if (readerFactory)
		{
			O2G2Ptr<IO2GTradesTableResponseReader> reader = readerFactory->createTradesTableReader(response);
			/*
			for (int i = 0; i < reader->size(); i++)
			{
				O2G2Ptr<IO2GTradeRow> trade = reader->getRow(i);
				string opentime = "";
				Tools::formatDate( trade->getOpenTime(), opentime);

				std::cout << " This is a response to your request: \nTradeID = " << trade->getTradeID() <<
					" OfferID = " << trade->getOfferID() <<
					" OfferInstrument = " << Tools::OfferID2OfferName(this->mSession, trade->getOfferID()) <<
					" AccountName = " << trade->getAccountName() <<
					" OpenOrderID  = " << trade->getOpenOrderID() <<
					" OpenOrderReqID = " << trade->getOpenOrderReqID() <<
					" getOpenOrderRequestTXT = " << trade->getOpenOrderRequestTXT() <<
					" getOpenQuoteID = " << trade->getOpenQuoteID() <<
					" TradeIDOrigin = " << trade->getTradeIDOrigin() <<
					" Commission = " << trade->getCommission() <<
					" UsedMargin = " << trade->getUsedMargin() <<
					" OpenRate = " << trade->getOpenRate() << 
					" OpenTime = " << opentime <<
					" OpenTime = " << trade->getOpenTime() <<
					" ValueDate = " << trade->getValueDate() <<
					" Parties = " << trade->getParties() <<
					" BuySell = " << trade->getBuySell() <<
					" Amount = " << trade->getAmount() << std::endl;
			}
			*/
			// 构造Task,发送数据给python终端
			Task task = Task();
			task.task_name = OnQryPosition_smart;
			if (mRequestDataSet[requestId].requestType == O2GResponseType::GetTrades)
			{
				task.instrument = mRequestDataSet[requestId].instrument;
			}
			else
			{
				task.instrument = "Error Instrument";
			}
			
			task.task_data = reader.Detach();
			//插入task队列
			this->api->putTask(task);
		}
		//mRequestComplete = true;

	}
}

void ResponseListener::onRequestFailed(const char * requestId, const char * error)
{
	if (mRequestID == requestId)
	{
		SetEvent(mResponseEvent);
	}
}

/** Request update data received data handler. */
// 
void ResponseListener::onTablesUpdates(IO2GResponse * data)
{
	
	//货币对实时报价的订阅消息处理
	if (data->getType() == TablesUpdates)
	{
		O2GResponseType repType = data->getType();
		O2G2Ptr<IO2GResponseReaderFactory> factory = mSession->getResponseReaderFactory();
		if (factory)
		{
			O2G2Ptr<IO2GTablesUpdatesReader> reader = factory->createTablesUpdatesReader(data);
			// 构造Task
			Task task = Task();
			task.task_name = OnGetSubScribeData_smart;
			task.task_data = reader;// 不能用reader.Detach()，后面进行类型cast时会出错
			//将reader返回到api，在那里对数据进行遍历，并构造list数据
			//不能在这里构造，无法传递到python
			this->api->putTask(task);
		}
	}
}

/** Gets response.*/
IO2GResponse *ResponseListener::getResponse()
{
	if (mResponse)
		mResponse->addRef();
	return mResponse;
}
//
std::string ResponseListener::getOrderID()
{
	return mOrderID;
}
bool ResponseListener::waitEvents()
{
	return WaitForSingleObject(mResponseEvent, _TIMEOUT) == 0;
}

/** Set request. */
void ResponseListener::setRequestID(const char * sRequestID)
{
	if (mResponse)
	{
		mResponse->release();
		mResponse = NULL;
	}
	ResetEvent(mResponseEvent);
}
