#pragma once
#include "stdafx.h"
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
protected:
	virtual ~ResponseListener();
public:
	ResponseListener(IO2GSession *session);
	// Í¨¹ý IO2GResponseListener ¼Ì³Ð
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

