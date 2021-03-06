#include"stdafx.h"
#include "Tools.h"

using namespace std;
// 输出样本：2018-08-30 15:35:34 369，不支持"/"
// DATE 2 String
void Tools::formatDate(DATE d, std::string &buf)
{
	struct tm t;
	CO2GDateUtils::OleTimeToCTime(d, &t);
	std::stringstream sstream;
	sstream << std::setfill('0') << std::setw(4) << t.tm_year + 1900 << "-"
		<< std::setfill('0') << std::setw(2) << t.tm_mon + 1 << "-"
		<< std::setfill('0') << std::setw(2) << t.tm_mday << " "
		<< std::setfill('0') << std::setw(2) << t.tm_hour << ":"
		<< std::setfill('0') << std::setw(2) << t.tm_min << ":"
		<< std::setfill('0') << std::setw(2) << t.tm_sec;
	buf = sstream.str();
}
string Tools::Date2String(DATE d)
{
	string result;
	Tools::formatDate(d, result);
	return result;
}

// 输入时间：2018-08-30 15:35:34 369，不支持"/"
// COleDateTime转换为DATE ：double（COleDateTime）
 COleDateTime Tools::String2OleDateTime(char const *  timeStr)
{
	struct tm stTm;
	sscanf_s(timeStr, "%d-%d-%d %d:%d:%d",
		&(stTm.tm_year),
		&(stTm.tm_mon),
		&(stTm.tm_mday),
		&(stTm.tm_hour),
		&(stTm.tm_min),
		&(stTm.tm_sec));

	//stTm.tm_year -= 1900;
	//stTm.tm_mon--;
	//stTm.tm_isdst = -1;
	return COleDateTime(stTm.tm_year, stTm.tm_mon, stTm.tm_mday, stTm.tm_hour, stTm.tm_min, stTm.tm_sec);
}

 //将时间从本地时间转换为UTC时间
 DATE Tools::ConvertLocal2UTC(IO2GSession *pSession,DATE s)
 {
	 IO2GTimeConverter *timeConverter = pSession->getTimeConverter();
	 DATE d = timeConverter->convert(s, IO2GTimeConverter::Local, IO2GTimeConverter::UTC);
	 timeConverter->release();
	 return d;
 }
 //将UTC时间转换为本地时间
 DATE Tools::ConvertUTC2Local(IO2GSession *pSession,DATE s)
 {
	 IO2GTimeConverter *timeConverter = pSession->getTimeConverter();
	 DATE d = timeConverter->convert(s, IO2GTimeConverter::UTC, IO2GTimeConverter::Local);
	 timeConverter->release();
	 return d;
 }

 ///-------------------------------------------------------------------------------------
 ///从Python对象到C++类型转换用的函数
 ///-------------------------------------------------------------------------------------


 void Tools::getInt(boost::python::dict d, string key, int *value)
 {
	 if (d.has_key(key))		//检查字典中是否存在该键值
	 {
		 boost::python::api::object o = d[key];	//获取该键值
		 boost::python::extract<int> x(o);	//创建提取器
		 if (x.check())		//如果可以提取
		 {
			 *value = x();	//对目标整数指针赋值
		 }
	 }
 }

 void Tools::getDouble(boost::python::dict d, string key, double *value)
 {
	 if (d.has_key(key))
	 {
		 boost::python::api::object o = d[key];
		 boost::python::extract<double> x(o);
		 if (x.check())
		 {
			 *value = x();
		 }
	 }
 }
 void Tools::getStr(dict d, string key, char *value)
 {
	 if (d.has_key(key))
	 {
		 object o = d[key];
		 extract<string> x(o);
		 if (x.check())
		 {
			 string s = x();
			 const char *buffer = s.c_str();
			 //对字符串指针赋值必须使用strcpy_s, vs2013使用strcpy编译通不过
			 //+1应该是因为C++字符串的结尾符号？不是特别确定，不加这个1会出错
			 strcpy_s(value, strlen(buffer) + 1, buffer);

		 }
	 }
 }

 void Tools::getChar(dict d, string key, char *value)
 {
	 if (d.has_key(key))
	 {
		 object o = d[key];
		 extract<string> x(o);
		 if (x.check())
		 {
			 string s = x();
			 const char *buffer = s.c_str();
			 *value = *buffer;
		 }
	 }
 };

 IO2GAccountRow *  Tools::GetAccount(IO2GSession *session, string sAccountID)
 {
	 O2G2Ptr<IO2GLoginRules> loginRules = session->getLoginRules();
	 if (loginRules)
	 {
		 PRINTLINE("loginRules");
		 O2G2Ptr<IO2GResponse> response = loginRules->getTableRefreshResponse(Accounts);
		 if (response)
		 {
			 O2G2Ptr<IO2GResponseReaderFactory> readerFactory = session->getResponseReaderFactory();
			 if (readerFactory)
			 {
				 O2G2Ptr<IO2GAccountsTableResponseReader> reader = readerFactory->createAccountsTableReader(response);
				 PRINTLINE("reader->size() = " +reader->size());
				 for (int i = 0; i < reader->size(); ++i)
				 {
					 O2G2Ptr<IO2GAccountRow> account = reader->getRow(i);
					 if (account)
						 if (sAccountID != "" || account->getAccountID() == sAccountID)
						 {
							 string strMarginCallFlag = account->getMarginCallFlag();
							 string strAccountKind = account->getAccountKind();
							 // account->getAccountKind() == "N" 不能这样比较，会
							 if (strMarginCallFlag == "N" && (strAccountKind == "32"	 || strAccountKind == "36") )
							 {
								 return account.Detach();
							 }
						 }	 
				 }
			 }
		 }
	 }
	 PRINTLINE("Tools::GetAccount ERROR, return NULL");
	 return NULL;
 }

 IO2GOfferRow * Tools::GetOffer(IO2GSession *session, string sInstrument)
 {
	 if (!session || sInstrument == "")
		 return NULL;

	 IO2GOfferRow *resultOffer = NULL;
	 O2G2Ptr<IO2GLoginRules> loginRules = session->getLoginRules();
	 if (loginRules)
	 {
		 O2G2Ptr<IO2GResponse> response = loginRules->getTableRefreshResponse(Offers);
		 if (response)
		 {
			 O2G2Ptr<IO2GResponseReaderFactory> readerFactory = session->getResponseReaderFactory();
			 if (readerFactory)
			 {
				 O2G2Ptr<IO2GOffersTableResponseReader> reader = readerFactory->createOffersTableReader(response);
				 for (int i = 0; i < reader->size(); ++i)
				 {
					 O2G2Ptr<IO2GOfferRow> offer = reader->getRow(i);
					 if (offer)
					 {
						 // 输出货币对的订阅状态
						 if ( offer->getInstrument() == sInstrument )
						 {
							 resultOffer = offer.Detach();
						 }

					 }
				 }
			 }
		 }
	 }
	 return resultOffer;
 }

 IO2GOfferRow * Tools::getOfferRow(IO2GTableManager *tableManager, string intrument)
 {
	 O2G2Ptr<IO2GOffersTable> offersTable = (IO2GOffersTable *)tableManager->getTable(Offers);
	 for (int i = 0; i < offersTable->size(); ++i)
	 {
		 O2G2Ptr<IO2GOfferRow> offer = offersTable->getRow(i);
		 if (intrument == string(offer->getInstrument()))
		 {
			 return offer.Detach();
		 }
	 }
	 return NULL;
 }

 string Tools::GetResponseType(O2GResponseType type)
 {
	 string result = "unknow";
	 switch (type)
	 {
	 case O2GResponseType::CommandResponse:
	 {
		 result = "CommandResponse";
		 break;
	 }
	 case O2GResponseType::CreateOrderResponse:
	 {
		 result = "CreateOrderResponse";
		 break;
	 }
	 case O2GResponseType::GetAccounts:
	 {
		 result = "GetAccounts";
		 break;
	 }
		
	 case O2GResponseType::GetClosedTrades:
	 {
		 result = "GetClosedTrades";
		 break;
	 }
	 case O2GResponseType::GetLastOrderUpdate:
	 {
		 result = "GetLastOrderUpdate";
		 break;
	 }
	 case O2GResponseType::GetMessages:
	 {
		 result = "GetMessages";
		 break;
	 }
	 case O2GResponseType::GetOffers:
	 {
		 result = "GetOffers";
		 break;
	 }
	 case O2GResponseType::GetSystemProperties:
	 {
		 result = "GetSystemProperties";
		 break;
	 }
	 case O2GResponseType::GetTrades:
	 {
		 result = "GetTrades";
		 break;
	 }
	 case O2GResponseType::MarginRequirementsResponse:
	 {
		 result = "MarginRequirementsResponse ";
		 break;
	 }
	 case O2GResponseType::MarketDataSnapshot:
	 {
		 result = "MarketDataSnapshot  ";
		 break;
	 }
	 case O2GResponseType::TablesUpdates:
	 {
		 result = "TablesUpdates   ";
		 break;
	 }
	 case O2GResponseType::ResponseUnknown:
	 {
		 result = "ResponseUnknown    ";
		 break;
	 }
	 default:
		 break;
	 }
	 return result;
 }

 string Tools::OfferID2OfferName(IO2GSession *session,string offerid)
 {
	 string result = "";
	 if (!session )
		 return result;

	 IO2GOfferRow *resultOffer = NULL;
	 O2G2Ptr<IO2GLoginRules> loginRules = session->getLoginRules();
	 if (loginRules)
	 {
		 O2G2Ptr<IO2GResponse> response = loginRules->getTableRefreshResponse(Offers);
		 if (response)
		 {
			 O2G2Ptr<IO2GResponseReaderFactory> readerFactory = session->getResponseReaderFactory();
			 if (readerFactory)
			 {
				 O2G2Ptr<IO2GOffersTableResponseReader> reader = readerFactory->createOffersTableReader(response);
				 for (int i = 0; i < reader->size(); ++i)
				 {
					 O2G2Ptr<IO2GOfferRow> offer = reader->getRow(i);
					 if (offer)
					 {
						 // 输出货币对的订阅状态
						 if (offer->getOfferID() == offerid)
						 {
							 result = offer->getInstrument();
							 return result;
						 }

					 }
				 }
			 }
		 }
	 }
	 return result;
 }

 string Tools::OfferName2offerId(IO2GSession *session, string instrument)
 {
	 O2G2Ptr<IO2GOffersTable> offersTable = (IO2GOffersTable *)session->getTableManager()->getTable(Offers);
	 for (int i = 0; i < offersTable->size(); i++)
	 {
		 O2G2Ptr<IO2GOfferRow> offer = offersTable->getRow(i);
		 if (strcmp(instrument.c_str(), offer->getInstrument()) == 0)
			 return offer->getOfferID();
	 }
	 return "unknow instrment : " + instrument;
 }

 std::string Tools::getEntryOrderType(double dBid, double dAsk, double dRate, const char *sBuySell, double dPointSize, int iCondDistLimit, int iCondDistStop)
 {
	 double dAbsoluteDifference = 0.0;
	 if (strcmp(sBuySell, O2G2::Buy) == 0)
	 {
		 dAbsoluteDifference = dRate - dAsk;
	 }
	 else
	 {
		 dAbsoluteDifference = dBid - dRate;
	 }

	 int iDifferenceInPips = (int)(floor((dAbsoluteDifference / dPointSize) + 0.5));

	 if (iDifferenceInPips >= 0)
	 {
		 if (iDifferenceInPips <= iCondDistStop)
		 {
			 std::cout << "Price is too close to market." << std::endl;
			 return NULL;
		 }
		 return O2G2::Orders::StopEntry;
	 }
	 else
	 {
		 if (-iDifferenceInPips <= iCondDistLimit)
		 {
			 std::cout << "Price is too close to market." << std::endl;
			 return NULL;
		 }
		 return O2G2::Orders::LimitEntry;
	 }
 }
 std::string Tools::getO2GTableTypeName(O2GTable type)
 {
	 string result = "unknow";
	 switch (type)
	 {
	 case O2GTable::Accounts:
	 {
		result = "Accounts";
		break;
	 }
	 case O2GTable::ClosedTrades:
	 {
		 result = "ClosedTrades";
		 break;
	 }
	 case O2GTable::Messages:
	 {
		 result = "Messages";
		 break;
	 }

	 case O2GTable::Offers:
	 {
		 result = "Offers";
		 break;
	 }
	 case O2GTable::Orders:
	 {
		 result = "Orders";
		 break;
	 }
	 case O2GTable::Summary:
	 {
		 result = "Summary";
		 break;
	 }
	 case O2GTable::TableUnknown:
	 {
		 result = "TableUnknown";
		 break;
	 }
	 case O2GTable::Trades:
	 {
		 result = "Trades";
		 break;
	 }
	 default:
		 break;
	 }
	 return result;
 }


 IO2GTradeRow * Tools::getTradeRow(IO2GTableManager *tableManager, const char *tradeID)
 {
	 O2G2Ptr<IO2GTradesTable> tradesTable = (IO2GTradesTable *)tableManager->getTable(Trades);
	 for (int i = 0; i < tradesTable->size(); ++i)
	 {
		 O2G2Ptr<IO2GTradeRow> trade = tradesTable->getRow(i);
		 if (strcmp(tradeID, trade->getTradeID()) == 0 )
			 return trade.Detach();
	 }
	 return NULL;
 }


