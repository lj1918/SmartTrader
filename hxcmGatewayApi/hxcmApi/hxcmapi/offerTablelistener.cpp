#include "stdafx.h"
#include "offerTablelistener.h"


OfferTableListener::OfferTableListener()
{
	mRef = 1;
}

/**/
OfferTableListener::OfferTableListener(	HxcmApi * api)
{
	mApi = api;
	mRef = 1;
}


OfferTableListener::~OfferTableListener()
{
	mRef = 0;
}

long OfferTableListener::addRef()
{
	return InterlockedIncrement(&mRef);
}

long OfferTableListener::release()
{
	InterlockedDecrement(&mRef);
	if (mRef == 0)
		delete this;
	return mRef;
}

void OfferTableListener::onStatusChanged(O2GTableStatus status)
{
	PRINTLINE("OfferTableListener::onStatusChanged");
}

void OfferTableListener::onAdded(const char *rowID, IO2GRow *row)
{
	PRINTLINE("OfferTableListener::onAdded");
	O2G2Ptr<IO2GOfferTableRow> offerTableRow = boost::any_cast<O2G2Ptr<IO2GOfferTableRow>>(row);
	if (offerTableRow != NULL)
	{
		// 构造Task,发送数据给python终端
		Task task = Task();
		task.task_name = OnGetOffer_smart;
		task.task_data = offerTableRow;
		//this->mApi->putTask(task);
		//PRINTLINE("OfferTableListener::onAdded : Instrument: %s, Bid = %f, Ask = %f\n", offerRow->getInstrument(), offerRow->getBid(), offerRow->getAsk());
	}
}

void OfferTableListener::onChanged(const char *rowID, IO2GRow *row)
{
	//PRINTLINE("OfferTableListener::onChanged : rowID = ",string(rowID));
	IO2GOfferTableRow* offerTableRow = (IO2GOfferTableRow*)(row);
	if (offerTableRow != NULL)
	{
		// 构造Task,发送数据给python终端
		Task task = Task();
		task.task_name = OnGetOffer_smart;
		task.task_data = offerTableRow;
		this->mApi->putTask(task);	
		//PRINTLINE("OfferTableListener::onChanged : ", offerRow->getInstrument()," ; ", offerRow->getBid(), " ; ", offerRow->getAsk());
	}
}

void OfferTableListener::onDeleted(const char *rowID, IO2GRow *row)
{
	PRINTLINE("OfferTableListener::onDeleted");
}
