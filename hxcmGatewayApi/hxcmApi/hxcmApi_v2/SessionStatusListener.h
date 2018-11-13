#pragma once
#include "stdafx.h"
#include "FxcmApiStruct.h"
#include <string>
#include "hxcmApi.h"
using namespace std;

class SessionStatusListener: public IO2GSessionStatus
{
private:
	long mRefCount;
	string mSessionID;
	string mPin;
	bool mError;
	bool mConnected;
	bool mDisconnected;
	IO2GSession* mSession = NULL;
	HANDLE mSessionEvent;
	HxcmApi* mApi = NULL;


public:
	SessionStatusListener(HxcmApi * api, IO2GSession *session, const char *sessionID, const char *pin)
	{
		this->mApi = api;
		if (sessionID != 0)
			mSessionID = sessionID;
		else
			mSessionID = "";
		if (pin != 0)
			mPin = pin;
		else
			mPin = "";
		mSession = session;
		mSession->addRef();
		reset();
		mRefCount = 1;
		mSessionEvent = CreateEvent(0, FALSE, FALSE, 0);
	}
	~SessionStatusListener()
	{
	}
	//
	virtual long addRef() override;
	 virtual long release() override;
	 virtual void onSessionStatusChanged(O2GSessionStatus status) override;
	 virtual void onLoginFailed(const char * error) override;

	 void reset()
	 {
		 mConnected = false;
		 mDisconnected = false;
		 mError = false;
	 }
	 bool waitEvents();
	 bool isDisconnected() const;
	 bool isConnected() const;
};



