#pragma once

class Account
{
private:
	Account(IO2GSession *session,std::string accountid);
	static IO2GSession *session;
	static Account * instance;
	static std::string accountID;
protected:
	
public:	

	~Account();
	static Account * Instance(IO2GSession *session,std::string accountID);

	bool update();

	// 账号ID
	std::string  AccountID;
	//  账号类型 : 32：Self-traded account,36 ：Funds manager acc, 38：Managed account
	std::string  AccountKind;
	// 显示在fxcm交易station上的名称
	std::string  AccountName;
	//一个订单的最大金额
	int  AmountLimit;
	// 账号交易概况（account trading profile ）ID
	std::string  ATPID;
	// 账号的资金金额
	double  Balance;
	// 一手的大小
	int  BaseUnitSize;
	// 最新补充保证金通知的日期
	DATE  LastMarginCallDate;
	// 账户杠杆概况的ID
	std::string  LeverageProfileID;
	// 在交易开始日的账户权益余额
	double  M2MEquity;
	// 是否处于维护状态： a rollover maintenance 
	bool  MaintenanceFlag;
	// 账户可以进行交易的类型：
	//Y：ging is allowed.
	//N：Hedging is not allowed
	//0：Netting only
	//D：Day netting
	std::string  MaintenanceType;
	// Gets the unique identification number of the funds manager account.
	// 资金经理账户ID ???
	std::string  ManagerAccountID;
	// the limitation state of the account.
	// The returned value defines the operations that can be performed on the account.
	// 补充保证金标记
	std::string  MarginCallFlag;
	//  the amount of accounting transactions that is applied to the account during the current trading day
	// 在当前交易日应用于账户的会计交易金额
	double  NonTradeEquity;

};

