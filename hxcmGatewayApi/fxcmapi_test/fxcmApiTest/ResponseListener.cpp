#include "stdafx.h"
#include "ResponseListener.h"

//#include <boost/format.hpp>

using namespace std;
ResponseListener::~ResponseListener()
{
	if (mResponse)
		mResponse->release();
	mSession->release();
	CloseHandle(mResponseEvent);
}

ResponseListener::ResponseListener(IO2GSession * session)
{
	mSession = session;
	//std::cout << this->api << std::endl;
	mSession->addRef();
	mRefCount = 1;
	mResponseEvent = CreateEvent(0, FALSE, FALSE, 0);
	mRequestID = "";
	mOrderID = "";
	mResponse = NULL;

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
			// 10. 获取 IO2GResponseReaderFactory:
		O2G2Ptr<IO2GResponseReaderFactory> readerFactory = mSession->getResponseReaderFactory();
		
		if (readerFactory)
		{			
			std::cout << "if (readerFactory) " << __LINE__ << std::endl;
			// 11. 创建 IO2GMarketDataSnapshotResponseReader:
			O2G2Ptr<IO2GMarketDataSnapshotResponseReader> reader = readerFactory->createMarketDataSnapshotReader(response);
			if (reader)
			{
				std::cout << "reader =  " << reader  << " "<< __LINE__ << std::endl;
			}

		}
		return;
	}
	// command request的响应

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
