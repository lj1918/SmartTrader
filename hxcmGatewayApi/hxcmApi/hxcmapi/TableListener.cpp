#include "stdafx.h"
#include "TableListener.h"


TableListener::TableListener(void)
{
	mEvent = CreateEventA(0, FALSE, FALSE, 0);
	mRef = 1;
}

TableListener::~TableListener(void)
{
	CloseHandle(mEvent);
}

long TableListener::addRef()
{
	return InterlockedIncrement(&mRef);
}

long TableListener::release()
{
	InterlockedDecrement(&mRef);
	if (mRef == 0)
		delete this;
	return mRef;
}

HANDLE TableListener::getEvent()
{
	return mEvent;
}

void TableListener::onStatusChanged(O2GTableStatus status)
{
}

void TableListener::onAdded(const char *rowID, IO2GRow *row)
{
	IO2GOfferTableRow *offerTableRow = static_cast<IO2GOfferTableRow *>(row);
	//...
	SetEvent(mEvent);
}

void TableListener::onChanged(const char *rowID, IO2GRow *row)
{
}

void TableListener::onDeleted(const char *rowID, IO2GRow *row)
{
}

void TableListener::onEachRow(const char *, IO2GRow *)
{
}
