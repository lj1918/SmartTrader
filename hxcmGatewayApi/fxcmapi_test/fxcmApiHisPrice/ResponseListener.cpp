#include "stdafx.h"
#include "ResponseListener.h"

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
	mSession->addRef();
	mRefCount = 1;
	mResponseEvent = CreateEvent(0, FALSE, FALSE, 0);
	mRequestID = "";
	mOrderID = "";
	mResponse = NULL;
	std::cout.precision(2);
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
/** Request execution completed data handler. »Øµ÷º¯Êý*/
void ResponseListener::onRequestCompleted(const char * requestId, IO2GResponse * response = 0)
{
	if (response && mRequestID == requestId)
	{
		mResponse = response;
		mResponse->addRef();
		if (response->getType() != CreateOrderResponse)
			SetEvent(mResponseEvent);
	}
}

void ResponseListener::onRequestFailed(const char * requestId, const char * error)
{
	if (mRequestID == requestId)
	{
		std::cout << "The request has been failed. ID: " << requestId << " : " << error << std::endl;
		SetEvent(mResponseEvent);
	}
}

/** Request update data received data handler. */
void ResponseListener::onTablesUpdates(IO2GResponse * tablesUpdates)
{
	std::cout << "call ResponseListener::onTablesUpdates "  << std::endl;
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
	std::cout << " ResponseListener::setRequestID" << std::endl;
	std::cout << sRequestID << std::endl;
	mRequestID = sRequestID;
	if (mResponse)
	{
		mResponse->release();
		mResponse = NULL;
	}
	ResetEvent(mResponseEvent);
}
