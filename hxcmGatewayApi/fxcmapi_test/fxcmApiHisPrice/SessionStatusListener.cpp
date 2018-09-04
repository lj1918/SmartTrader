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
	switch (status)
	{
	case IO2GSessionStatus::Disconnected:
		std::cout << "status::disconnected" << std::endl;
		mConnected = false;
		mDisconnected = true;
		SetEvent(mSessionEvent);
		break;
	case IO2GSessionStatus::Connecting:
		std::cout << "status::connecting" << std::endl;
		break;
	case IO2GSessionStatus::TradingSessionRequested:
	{
		std::cout << "status::trading session requested" << std::endl;
		O2G2Ptr<IO2GSessionDescriptorCollection> descriptors = mSession->getTradingSessionDescriptors();
		bool found = false;
		if (descriptors)
		{
			if (mPrintSubsessions)
				std::cout << "descriptors available:" << std::endl;
			for (int i = 0; i < descriptors->size(); ++i)
			{
				O2G2Ptr<IO2GSessionDescriptor> descriptor = descriptors->get(i);
				if (mPrintSubsessions)
					std::cout << "  id:='" << descriptor->getID()
					<< "' name='" << descriptor->getName()
					<< "' description='" << descriptor->getDescription()
					<< "' " << (descriptor->requiresPin() ? "requires pin" : "") << std::endl;
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
		std::cout << "status::connected" << std::endl;
		mConnected = true;
		mDisconnected = false;
		SetEvent(mSessionEvent);
		break;
	case IO2GSessionStatus::Reconnecting:
		std::cout << "status::reconnecting" << std::endl;
		break;
	case IO2GSessionStatus::Disconnecting:
		std::cout << "status::disconnecting" << std::endl;
		break;
	case IO2GSessionStatus::SessionLost:
		std::cout << "status::session lost" << std::endl;
		break;
	}
}

void SessionStatusListener::onLoginFailed(const char * error)
{
	std::cout << "Login error: " << error << std::endl;
	mError = true;
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
