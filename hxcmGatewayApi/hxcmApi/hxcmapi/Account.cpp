#include "stdafx.h"
#include "Account.h"

Account * Account::instance = NULL;
IO2GSession * Account::session = NULL;

Account::Account(IO2GSession *session)
{
	Account::session = session;
}


Account::~Account()
{
	delete instance;
	instance = NULL;
}

Account*  Account::Instance(IO2GSession *session)
{
	if (instance == NULL)
	{
		instance = new Account(session);
	}
	return instance;
}


bool Account::update()
{
	bool result = false;
	try
	{
		O2G2Ptr<IO2GLoginRules> loginRules = session->getLoginRules();
		O2G2Ptr<IO2GResponse> response = loginRules->getTableRefreshResponse(O2GTable::Accounts);
		O2G2Ptr<IO2GResponseReaderFactory> readerFactory = session->getResponseReaderFactory();
		O2G2Ptr<IO2GAccountsTableResponseReader>  accountsResponseReader = readerFactory->createAccountsTableReader(response);
		bool bWasError = false;
		// 只取第一个账户的信息
		O2G2Ptr<IO2GAccountRow> accountRow = accountsResponseReader->getRow(0);
		this->AccountID = accountRow->getAccountID();
		this->AccountKind = accountRow->getAccountKind();
		this->AccountName = accountRow->getAccountName();
		this->AmountLimit = accountRow->getAmountLimit();
		this->ATPID = accountRow->getARPID();
		this->Balance = accountRow->getBalance();
		this->BaseUnitSize = accountRow->getBaseUnitSize();
		this->LastMarginCallDate = accountRow->getLastMarginCallDate();
		this->LeverageProfileID = accountRow->getLeverageProfileID();
		this->M2MEquity = accountRow->getM2MEquity();
		this->MaintenanceFlag = accountRow->getMaintenanceFlag();
		this->MaintenanceType = accountRow->getMaintenanceType();
		this->ManagerAccountID = accountRow->getManagerAccountID();
		result = true;
	}
	catch (const std::exception& ee)
	{
		std::cout << ee.what() << std::endl;
		result = false;
		return result;
	}
	return result;
}