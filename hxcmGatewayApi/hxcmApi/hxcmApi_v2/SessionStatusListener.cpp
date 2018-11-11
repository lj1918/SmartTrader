#include "stdafx.h"
#include "SessionStatusListener.h"


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
	char c[2];
	sprintf_s(c, "%d", status);
	string temp = "onSessionStatusChanged : status = " + string(c);
	Task task = Task();
	task.task_name = OnLogin_smart;
	switch (status)
	{
	case IO2GSessionStatus::Disconnected:
	{
		mConnected = false;
		mDisconnected = true;
		//��������,�Ժ���ʵ��
		//this->mSession->login();
		//���źţ�ֹͣ�ȴ�
		SetEvent(mSessionEvent);
		//����onLogin�¼�		
		task.task_data = status;
		this->mApi->putTask(task);
		break;
	}
	case IO2GSessionStatus::Connecting:
	{
		//����onLogin�¼�		
		task.task_data = status;
		this->mApi->putTask(task);
		break;
	}
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
	{
		//std::cout << "IO2GSessionStatus::Connected" << status << std::endl;
		//���źţ�ֹͣ�ȴ�
		mConnected = true;
		mDisconnected = false;
		SetEvent(mSessionEvent);
		//����onLogin�¼�		
		task.task_data = status;
		this->mApi->putTask(task);
		break;
	}

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
	//����LoginFailedԭ��
	Task task = Task();
	task.task_name = OnMessage_smart;
	task.task_data = string(error);
	this->mApi->putTask(task);
	//
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
