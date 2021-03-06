// fxcmApiTest.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "SessionStatusListener.h"
#include "ResponseListener.h"
#include "Account.h"


string AccountId = "701037785";
string Password = "4616";
string url = "http://www.fxcorporate.com/Hosts.jsp";
//The name of the connection, for example "Demo" or "Real".
string connection = "Demo";
bool login(IO2GSession *session, SessionStatusListener *statusListener, string account, string pwd, string url, string conn);
int main()
{
	string accountName = "";

	IO2GSession *session = CO2GTransport::createSession();
	SessionStatusListener *sessionListener = new SessionStatusListener(session);
	session->subscribeSessionStatus(sessionListener);
	bool bConnected = login(session, sessionListener, AccountId, Password, url, connection);

	if (bConnected)
	{
		if( 1 == 1)
		{
			O2G2Ptr<IO2GLoginRules> loginRules = session->getLoginRules();
			O2G2Ptr<IO2GResponse> response = loginRules->getTableRefreshResponse(O2GTable::Accounts);
			O2G2Ptr<IO2GResponseReaderFactory> readerFactory = session->getResponseReaderFactory();
			O2G2Ptr<IO2GAccountsTableResponseReader>  accountsResponseReader = readerFactory->createAccountsTableReader(response);
			bool bWasError = false;
			for (int i = 0; i < accountsResponseReader->size(); i++)
			{
				O2G2Ptr<IO2GAccountRow> accountRow = accountsResponseReader->getRow(i);
				std::cout << "AccountID: " << accountRow->getAccountID() << ", "
					<< "Balance: " << std::fixed << accountRow->getBalance() << ", "
					<< "AccountName: " << std::fixed << accountRow->getAccountName() << ", "
					<< "AccountID: " << std::fixed << accountRow->getAccountID() << ", "
					// 32: Self-traded account, funds manager account (only LAMM), managed account (only LAMM
					// 36: Funds manager account(only PAMM).
					// 38; Managed account (only PAMM).
					<< "AccountKind: " << std::fixed << accountRow->getAccountKind() << ", "
					// the maximum amount of an order that is allowed on the account
					<< "AmountLimit: " << std::fixed << accountRow->getAmountLimit() << ", "
					// the unique identification number of an account trading profile which defines the commission requirements
					<< "getATPID : " << std::fixed << accountRow->getATPID() << ", "
					//  the size of one lot
					<< "BaseUnitSize: " << accountRow->getBaseUnitSize() << std::endl;
			}
		}
		
		Account * account = Account::Instance(session);
		if (account->update())
		{
			string name = account->AccountName;
		}

	}

	string temp;
	std::cin >> temp ;
    return 0;
}

bool login(IO2GSession *session, SessionStatusListener *statusListener,string account,string pwd,string url,string conn)
{
	statusListener->reset();
	session->login(account.c_str(), pwd.c_str(), url.c_str(), conn.c_str());
	return statusListener->waitEvents() && statusListener->isConnected();
}