#pragma once
#include "stdafx.h"
#include "FxcmApiStruct.h"
#include <iostream>
#include <map>
using namespace std;
//前置声明解决头文件相互包含的问题
//#include "hxcmapi.h"
class HxcmApi;

class ResponseListener : public IO2GResponseListener
{
private:
	long mRefCount;
	/** Session object. */
	IO2GSession *mSession;
	/** Request we are waiting for. */
	std::string mRequestID;
	/** Response Event handle. */
	HANDLE mResponseEvent;

	/** Order ID. */
	std::string mOrderID;

	/** State of last request. */
	IO2GResponse *mResponse;

	HxcmApi *api = NULL;

	//
	

protected:
	virtual ~ResponseListener();
public:
	//数据
	map<string, sFxcmRequestHisPrices> mRequestDataSet;

	//方法
	ResponseListener(IO2GSession *session, HxcmApi * api/**/);
	// 通过 IO2GResponseListener 继承
	virtual long addRef() override;
	virtual long release() override;
	virtual void onRequestCompleted(const char * requestId, IO2GResponse * response) override;
	virtual void onRequestFailed(const char * requestId, const char * error) override;
	virtual void onTablesUpdates(IO2GResponse * tablesUpdates) override;
	//
	/** Get response.*/
	IO2GResponse *getResponse();

	std::string getOrderID();
	/** Wait for request execution or error. */
	bool waitEvents();

	/** Set request ID. */
	void setRequestID(const char *sRequestID);
};

