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
	//std::cout << "requestId = " << requestId << std::endl;
	//是否还有再次查询
	bool needquestAgain = false;

	//如果是查询历史价格的响应信息,可以列出货币对订阅的情况，货币对实时交易信息在onTablesUpdates事件中处理
	//std::cout << "onRequestCompleted " << response->getType()  << "  "<<__LINE__ << std::endl;
	if (response && response->getType() == MarketDataSnapshot)
	{
		//获取该响应对应请求的相关信息：账户、密码等
		sFxcmRequestHisPrices reqData = mRequestDataSet[string(requestId)];
		//std::cout << reqData.instrument<< " " << reqData.beginDate << " " <<reqData.endDate << " " <<reqData.maxBars  << "l ine no =" << __LINE__ << std::endl;
		// 10. 获取 IO2GResponseReaderFactory:
		O2G2Ptr<IO2GResponseReaderFactory> readerFactory = mSession->getResponseReaderFactory();
		
		if (readerFactory)
		{			
			//std::cout << "if (readerFactory) " << __LINE__ << std::endl;
			// 11. 创建 IO2GMarketDataSnapshotResponseReader:
			O2G2Ptr<IO2GMarketDataSnapshotResponseReader> reader = readerFactory->createMarketDataSnapshotReader(response);
			//std::cout << "reader =  " << reader  << " "<< __LINE__ << std::endl;
			if (reader)
			{

			}
			if ( reader->size() > 0)
			{
				//std::cout << "  ( reader->size() > 0) " << __LINE__ << std::endl;
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
					//std::cout << message << __LINE__ << std::endl;
					//this->api->sendMessage(message);
				}
				catch (const std::exception&ee)
				{
					std::cout << ee.what() << __LINE__ << std::endl;					
				}
				// 构造Task,发送数据给python终端
				//std::cout << "  begin send task " << __LINE__ << std::endl;
				Task task = Task();
				task.task_name = OnGetHisPrices_smart;
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
						O2G2Ptr<IO2GTimeframe> timeFrame = timeframeCollection->get( this->mRequestDataSet[requestId].stimeFrame.c_str() );

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
						sFxcmRequestHisPrices reqData;
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
		return;
	}
	// command request的响应
	if (response && response->getType() == Command)
	{
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
