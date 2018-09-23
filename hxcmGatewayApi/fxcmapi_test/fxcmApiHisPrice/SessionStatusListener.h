#pragma once
using namespace std;
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
	/** Flag indicating whether sessions must be printed. */
	bool mPrintSubsessions;
	/** Session object. */
	IO2GSession *mSession;
	/** Event handle. */
	HANDLE mSessionEvent;
public:

	// Í¨¹ý IO2GSessionStatus ¼Ì³Ð
	virtual long addRef() override;
	virtual long release() override;
	virtual void onSessionStatusChanged(O2GSessionStatus status) override;
	virtual void onLoginFailed(const char * error) override;

	//
	SessionStatusListener(IO2GSession *session)
	{

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

