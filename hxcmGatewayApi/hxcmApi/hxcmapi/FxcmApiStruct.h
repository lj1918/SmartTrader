#pragma once

/////////////////////////////////////////////////////////////////////////
///hxcmApi 结构
///201808  刘军
/////////////////////////////////////////////////////////////////////////
using namespace std;
using namespace boost;

// 常量
#define		OnGetHisPrices_smart			1  //获取历史数据标记
#define		OnGetSubScribeData_smart		2 //获取订阅的货币对（instrument）的市场报价（offer）
#define		OnMessage_smart				3 //返回给客户端的信息，包括出错信息

struct SFxcmLoginField
{
	// 账号
};
//历史数据的Request参数
struct sFxcmRequestHisPrices
{
	string instrument;
	string stimeFrame;
	int maxBars;
	double beginDate;
	double endDate;
	int getNums;
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
//任务结构体
struct Task
{
	//Task() {};
	//Task(const char* msg) :task_message(msg) {};
	int task_name;		//回调函数名称对应的常量
	any task_data;		//数据结构体
	any task_error;		//错误结构体
	int task_id;		//请求id
	bool task_last;		//是否为最后返回
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