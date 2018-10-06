#pragma once

/////////////////////////////////////////////////////////////////////////
///hxcmApi �ṹ
///201808  ����
/////////////////////////////////////////////////////////////////////////
using namespace std;
using namespace boost;

// ����
#define     CustomID_smart		"liujun_smartWorkShop"

#define		OnGetHisPrices_smart			1  //��ȡ��ʷ���ݱ��
#define		OnGetSubScribeData_smart		2 //��ȡ���ĵĻ��Ҷԣ�instrument�����г����ۣ�offer��
#define		OnMessage_smart					3 //���ظ��ͻ��˵���Ϣ������������Ϣ
#define     OnQryPosition_smart				4 //��ѯ���ҶԵĲ�λ��Ϣ
#define     OnTradesTableUpdate_smart		5 //OpenMarketOrder��result
#define     OnAccountsTableUpdate_smart					6 //Accounts������¼�
#define     OnClosedTradeTableUpdate_smart              7 //ClosedTradeTable������¼�
#define     OnQryClosed_TradesTable_smart				8 // ��ѯClosedTradeTable

struct SFxcmLoginField
{
	// �˺�
};
//��ʷ���ݵ�Request����
struct sFxcmRequestData
{
	string instrument;
	string stimeFrame;
	int maxBars;
	double beginDate;
	double endDate;
	int getNums;
	int requestType;
};

struct SFxcmHisPrice
{
	string instrument;
	double Ask;
	double  AskClose;
	double  AskHigh;
	double  AskLow;
	double  AskOpen;
	double  Bid;
	double  BidClose;
	double	BidHigh;
	double	BidLow;
	double	BidOpen;
	DATE	date;
	DATE	LastBarTime;
	int		LastBarVolume;
};
//����ṹ��
struct Task
{
	//Task() {};
	//Task(const char* msg) :task_message(msg) {};
	int task_name;		//�ص��������ƶ�Ӧ�ĳ���
	string instrument;	//���Ҷ�����
	any task_data;		//���ݽṹ��
	any task_error;		//����ṹ��
	int task_id;		//����id
	bool task_last;		//�Ƿ�Ϊ��󷵻�
	//char message[1024]  ;
	
};

enum eTimeFrames
{
	m1		=	1,
	m5		=	5,
	m15		=	15,
	m30		=	30,
	H1		=	60,
	H2		=	120,
	H4		=	240,
	H6		=	360,
	D1		=	24*60,
	W1		=	7*24*60
};

enum OrderSide
{
	Buy,
	Sell,
	Both
};

struct CloseOrderData
{
	std::string offerID;
	OrderSide side;
	std::string account;
};