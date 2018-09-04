#pragma once
using namespace std;
class CSessionStatusListener : public IO2GSessionStatus
{
	// Í¨¹ý IO2GSessionStatus ¼Ì³Ð
	virtual long addRef() override;
	virtual long release() override;
	virtual void onSessionStatusChanged(O2GSessionStatus status) override;
	virtual void onLoginFailed(const char * error) override;
	//
private:
	volatile bool mConnected;
	volatile bool mDisconnected;
	volatile bool mError;
	string mSubSessionID;
	string mPin;
	IO2GSession *mSession;
	IO2GSessionStatus::O2GSessionStatus mStatus;

	long mRefCount;

public:
	CSessionStatusListener(IO2GSession *session, const char *subSessionID, const char *pin)
	{
		mSession = session;
		mSession->addRef();
		mSubSessionID = subSessionID;
		mPin = pin;

		mConnected = false;
		mDisconnected = false;
		mError = false;
		mStatus = IO2GSessionStatus::Disconnected;
		mRefCount = 1;
		//
		mSessionEvent = CreateEvent(0, FALSE, FALSE, 0);
	}
	~CSessionStatusListener()
	{
		mSession->release();
	}
	/** Wait for connection or error. */
	HANDLE mSessionEvent;
	bool waitEvents()
	{
		return WaitForSingleObject(mSessionEvent, _TIMEOUT) == 0;
	}


};

