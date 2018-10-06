#pragma once
#include "stdafx.h"
#include <string>
#include <ATLComTime.h>
using namespace std;
using namespace boost::python;
/*
static修饰的类成为静态类，静态类中只能包含静态成员（被static修饰的字段、属性、方法），
不能被实例化，不能被继承；非静态中可以包含静态成员
*/
 static class Tools
{
	//静态类中不能存在构造函数  
public:
	//static void formatDate(DATE date, char *buf);
	static void formatDate(DATE d, std::string &buf);
	static string Date2String(DATE d);
	//static COleDateTime myString2Date(string  timeStr);
	static COleDateTime String2OleDateTime(char const * timeStr);

	///-------------------------------------------------------------------------------------
	///从Python对象到C++类型转换用的函数
	///-------------------------------------------------------------------------------------
	//从字典中获取某个建值对应的整数，并赋值到请求结构体对象的值上
	static void getInt(boost::python::dict d, string key, int *value);
	//从字典中获取某个建值对应的浮点数，并赋值到请求结构体对象的值上
	static void getDouble(boost::python::dict d, string key, double *value);
	//从字典中获取某个建值对应的字符，并赋值到请求结构体对象的值上
	static  void getStr(dict d, string key, char *value);
	//从字典中获取某个建值对应的字符串，并赋值到请求结构体对象的值上
	static  void getChar(dict d, string key, char *value);
	///-------------------------------------------------------------------------------------
	///便利函数
	///-------------------------------------------------------------------------------------
	//将时间从本地时间转换为UTC时间
	static DATE ConvertLocal2UTC(IO2GSession *pSession, DATE s);
	//将UTC时间转换为本地时间
	static DATE ConvertUTC2Local(IO2GSession *pSession, DATE s);
	// 获取账户
	static IO2GAccountRow * GetAccount(IO2GSession *session, string sAccountID);
	// 获取货币对对象
	static IO2GOfferRow * GetOffer(IO2GSession *session, string sInstrument);
	// 将O2GResponseType转换为字符串
	static string GetResponseType(O2GResponseType type);
	//
	static string OfferID2OfferName(IO2GSession *session,string offerid);
	static string OfferName2offerId(IO2GSession *session, string instrument);
	// Determine order type based on parameters: current market price of a trading instrument, desired order rate, order direction
	static  std::string getEntryOrderType(double dBid, double dAsk, double dRate, const char *sBuySell, double dPointSize, int iCondDistLimit, int iCondDistStop);
	// 返回枚举类型O2GTable对应的字符串
	static std::string getO2GTableTypeName(O2GTable table);
	// 根据tradeid获取trade对象
	static IO2GTradeRow * getTradeRow(IO2GTableManager *tableManager, const char *tradeID);
	// 根据货币对名称获取Offer对象
	static IO2GOfferRow * getOfferRow(IO2GTableManager *tableManager, string intrument);
};

