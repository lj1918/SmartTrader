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

	// �˺�ID
	std::string  AccountID;
	//  �˺����� : 32��Self-traded account,36 ��Funds manager acc, 38��Managed account
	std::string  AccountKind;
	// ��ʾ��fxcm����station�ϵ�����
	std::string  AccountName;
	//һ�������������
	int  AmountLimit;
	// �˺Ž��׸ſ���account trading profile ��ID
	std::string  ATPID;
	// �˺ŵ��ʽ���
	double  Balance;
	// һ�ֵĴ�С
	int  BaseUnitSize;
	// ���²��䱣֤��֪ͨ������
	DATE  LastMarginCallDate;
	// �˻��ܸ˸ſ���ID
	std::string  LeverageProfileID;
	// �ڽ��׿�ʼ�յ��˻�Ȩ�����
	double  M2MEquity;
	// �Ƿ���ά��״̬�� a rollover maintenance 
	bool  MaintenanceFlag;
	// �˻����Խ��н��׵����ͣ�
	//Y��ging is allowed.
	//N��Hedging is not allowed
	//0��Netting only
	//D��Day netting
	std::string  MaintenanceType;
	// Gets the unique identification number of the funds manager account.
	// �ʽ����˻�ID ???
	std::string  ManagerAccountID;
	// the limitation state of the account.
	// The returned value defines the operations that can be performed on the account.
	// ���䱣֤����
	std::string  MarginCallFlag;
	//  the amount of accounting transactions that is applied to the account during the current trading day
	// �ڵ�ǰ������Ӧ�����˻��Ļ�ƽ��׽��
	double  NonTradeEquity;

};

