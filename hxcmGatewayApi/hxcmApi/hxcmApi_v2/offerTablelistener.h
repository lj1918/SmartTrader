#pragma once
#include "stdafx.h"
#include "hxcmApiStruct.h"
#include "hxcmApi.h"

class HxcmApi;

class OfferTableListener : public IO2GTableListener
{
private:
	HANDLE mEvent;
	long mRef;
	std::string mRequestID;
	HxcmApi* mApi;
	
public:
	OfferTableListener();
	OfferTableListener(HxcmApi* api);
	~OfferTableListener();
	long addRef(void);
	long release();
	virtual void onAdded(const char *rowID, IO2GRow *rowData) override;
	virtual void onChanged(const char *rowID, IO2GRow *rowData) override;
	void onDeleted(const char *rowID, IO2GRow *rowData) ;
	void onStatusChanged(O2GTableStatus status) ;

	void setRequest(std::string sRequestID)
	{
		mRequestID = sRequestID;
	}
	HANDLE getEvent();
};

