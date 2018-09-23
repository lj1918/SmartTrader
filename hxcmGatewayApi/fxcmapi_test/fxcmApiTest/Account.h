#pragma once

class Account
{
private:
	Account(IO2GSession *session);
	static IO2GSession *session;
	static Account * instance;
protected:
	
public:	

	~Account();
	static Account * Instance(IO2GSession *session);

	bool update();

	// �˺�ID
	const char *  AccountID;
	//  �˺����� : 32��Self-traded account,36 ��Funds manager acc, 38��Managed account
	const char *  AccountKind;
	// ��ʾ��fxcm����station�ϵ�����
	const char *  AccountName;
	//һ�������������
	int  AmountLimit;
	// �˺Ž��׸ſ���account trading profile ��ID
	const char *  ATPID;
	// �˺ŵ��ʽ���
	double  Balance;
	// һ�ֵĴ�С
	int  BaseUnitSize;
	// ���²��䱣֤��֪ͨ������
	DATE  LastMarginCallDate;
	// �˻��ܸ˸ſ���ID
	const char *  LeverageProfileID;
	// �ڽ��׿�ʼ�յ��˻�Ȩ�����
	double  M2MEquity;
	// �Ƿ���ά��״̬�� a rollover maintenance 
	bool  MaintenanceFlag;
	// �˻����Խ��н��׵����ͣ�
	//Y��ging is allowed.
	//N��Hedging is not allowed
	//0��Netting only
	//D��Day netting
	const char *  MaintenanceType;
	// Gets the unique identification number of the funds manager account.
	// �ʽ����˻�ID ???
	const char *  ManagerAccountID;
	// the limitation state of the account.
	// The returned value defines the operations that can be performed on the account.
	// ���䱣֤����
	const char *  MarginCallFlag;
	//  the amount of accounting transactions that is applied to the account during the current trading day
	// �ڵ�ǰ������Ӧ�����˻��Ļ�ƽ��׽��
	double  NonTradeEquity;

};

