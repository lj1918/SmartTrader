#include "stdafx.h"
#include "SessionStatusListener.h"

/** Constructor. */


long CSessionStatusListener::addRef()
{
	return InterlockedIncrement(&mRefCount);

}

long CSessionStatusListener::release()
{
	long rc = InterlockedDecrement(&mRefCount);
	if (rc == 0)
		delete this;
	return rc;

}

void CSessionStatusListener::onSessionStatusChanged(O2GSessionStatus status)
{
	mStatus = status;
	switch (mStatus)
	{
	case    IO2GSessionStatus::Disconnected:
	{
		std::cout << "Status::disconnected" << std::endl;
		mConnected = false;
		mDisconnected = true;
		SetEvent(mSessionEvent);
		break;
	}

	case    IO2GSessionStatus::Connecting:
		std::cout << "Status::connecting" << std::endl;
		break;
	case    IO2GSessionStatus::TradingSessionRequested:
		std::cout << "Status::trading session requested" << std::endl;
		break;
	case    IO2GSessionStatus::Connected:
	{
		std::cout << "Status::connected" << std::endl; 
		DATE serverDate = mSession->getServerTime();
		std::cout << "serverDate = " << serverDate << std::endl;
		O2GUserKind userKind = mSession->getUserKind();
		std::cout << "userKind = " << userKind;
		IO2GSessionDescriptorCollection * descriptors = mSession->getTradingSessionDescriptors();
		mConnected = true;
		mDisconnected = false;
		SetEvent(mSessionEvent);
		break;
	}
	case    IO2GSessionStatus::Reconnecting:
		std::cout << "Status::reconnecting" << std::endl;
		break;
	case    IO2GSessionStatus::Disconnecting:
		std::cout << "Status::disconnecting" << std::endl;
		break;
	case    IO2GSessionStatus::SessionLost:
		std::cout << "Status::session lost" << std::endl;
		break;
	}

	if (mStatus == IO2GSessionStatus::TradingSessionRequested)
	{
		IO2GSessionDescriptorCollection *descriptors = mSession->getTradingSessionDescriptors();
		bool found = false;
		if (descriptors)
		{
			std::cout << "Available descriptors:" << std::endl;
			for (int i = 0; i < descriptors->size(); i++)
			{
				IO2GSessionDescriptor *descriptor = descriptors->get(i);
				string temp = "";
				if (descriptor->requiresPin()  == true)
					temp = "requires pin";
				else
					temp = "not requires pin";
				std::cout << "  id='" << descriptor->getID() << "' " <<
					"name='" << descriptor->getName() << "' " <<
					"description='" << descriptor->getDescription() << "' " << temp  << std::endl;

				if (mSubSessionID == descriptor->getID())
					found = true;

				descriptor->release();
			}
			descriptors->release();
		}
		if (!found)
			onLoginFailed("The specified sub session identifier is not found");
		else
			mSession->setTradingSession(mSubSessionID.c_str(), mPin.c_str());
	}

}

void CSessionStatusListener::onLoginFailed(const char * error)
{
	std::cout << "Login error: " << error << std::endl;
	mError = true;

}
