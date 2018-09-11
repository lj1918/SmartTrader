#include "stdafx.h"
#include "SessionStatusListener.h"
using namespace std;


long SessionStatusListener::addRef()
{
	return InterlockedIncrement(&mRefCount);
}

long SessionStatusListener::release()
{
	long rc = InterlockedDecrement(&mRefCount);
	if (rc == 0)
		delete this;
	return rc;
}

void SessionStatusListener::onSessionStatusChanged(O2GSessionStatus status)
{
	std::cout <<"status = " << status << std::endl;
	switch (status)
	{
	case IO2GSessionStatus::Disconnected:
		std::cout << "status = IO2GSessionStatus::Disconnected " << status << std::endl;
		mConnected = false;
		mDisconnected = true;

		//发信号，停止等待
		SetEvent(mSessionEvent);
		break;
	case IO2GSessionStatus::Connecting:
		break;
	case IO2GSessionStatus::TradingSessionRequested:
	{
		O2G2Ptr<IO2GSessionDescriptorCollection> descriptors = mSession->getTradingSessionDescriptors();
		bool found = false;
		if (descriptors)
		{
			for (int i = 0; i < descriptors->size(); ++i)
			{
				O2G2Ptr<IO2GSessionDescriptor> descriptor = descriptors->get(i);
				if (mSessionID == descriptor->getID())
				{
					found = true;
					break;
				}
			}
		}
		if (!found)
		{
			onLoginFailed("The specified sub session identifier is not found");
		}
		else
		{
			mSession->setTradingSession(mSessionID.c_str(), mPin.c_str());
		}
	}
	break;
	case IO2GSessionStatus::Connected:
		std::cout << "IO2GSessionStatus::Connected" << status << std::endl;
		//发信号，停止等待
		//api->sendMessage("Login successed!!!");
		mConnected = true;
		mDisconnected = false;
		SetEvent(mSessionEvent);
		break;
	case IO2GSessionStatus::Reconnecting:
		break;
	case IO2GSessionStatus::Disconnecting:
		break;
	case IO2GSessionStatus::SessionLost:
		break;
	}
}

void SessionStatusListener::onLoginFailed(const char * error)
{
	mError = true;
	std::cout << error << std::endl;
}
/** Wait for connection or error. */
bool SessionStatusListener::waitEvents()
{
	return WaitForSingleObject(mSessionEvent, _TIMEOUT) == 0;
}
/** Check whether session is disconnected */
bool SessionStatusListener::isDisconnected() const
{
	return mDisconnected;
}
bool SessionStatusListener::isConnected() const
{
	return mConnected;
}
