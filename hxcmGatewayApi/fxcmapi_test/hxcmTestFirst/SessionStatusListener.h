#pragma once
#include "stdafx.h"
#include "FxcmApiStruct.h"
#include <string>
using namespace std;

//ǰ���������ͷ�ļ��໥����������,����ֱ�����ͷ�ļ����㲻�����
//#include "hxcmapi.h"
//class HxcmApi;

class SessionStatusListener : public IO2GSessionStatus
{
private:
	long mRefCount;
	/** Subsession identifier. */
	std::string mSessionID;
	/** Pin code. */
	std::string mPin;
	/** Error flag. */
	bool mError;
	/** Flag indicating that connection is set. */
	bool mConnected;
	/** Flag indicating that connection was ended. */
	bool mDisconnected;
	/** Session object. */
	IO2GSession *mSession;
	/** Event handle. */
	HANDLE mSessionEvent;

	

public:
	//
	//HxcmApi * api = NULL;

	// ͨ�� IO2GSessionStatus �̳�
	virtual long addRef() override;
	virtual long release() override;
	virtual void onSessionStatusChanged(O2GSessionStatus status) override;
	virtual void onLoginFailed(const char * error) override;

	//
	SessionStatusListener(IO2GSession *session,  const char *sessionID, const char *pin)
	{
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

