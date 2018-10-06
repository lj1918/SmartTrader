#include "stdafx.h"
#include "ResponseListener.h"
#include "hxcmapi.h"
#include "Tools.h"
#include <boost/format.hpp>

using namespace std;
ResponseListener::~ResponseListener()
{
	if (mResponse)
		mResponse->release();
	mSession->release();
	CloseHandle(mResponseEvent);
}

ResponseListener::ResponseListener(IO2GSession * session, HxcmApi *api/**/)
{
	mSession = session;
	this->api = api;
	//std::cout << this->api << std::endl;
	mSession->addRef();
	mRefCount = 1;
	mResponseEvent = CreateEvent(0, FALSE, FALSE, 0);
	mRequestID = "";
	mOrderID = "";
	mResponse = NULL;
	//mRequestDataSet =map<
}

long ResponseListener::addRef()
{
	return InterlockedIncrement(&mRefCount);
}

long ResponseListener::release()
{
	long rc = InterlockedDecrement(&mRefCount);
	if (rc == 0)
		delete this;
	return rc;
}
/** Request execution completed data handler. �ص�����*/
void ResponseListener::onRequestCompleted(const char * requestId, IO2GResponse * response = 0)
{
	PRINTLINE("response->getType() = " + Tools::GetResponseType(response->getType())  );
	//std::cout << "requestId = " << requestId << std::endl;
	//�Ƿ����ٴβ�ѯ
	bool needquestAgain = false;

	//����ǲ�ѯ��ʷ�۸����Ӧ��Ϣ,�����г����ҶԶ��ĵ���������Ҷ�ʵʱ������Ϣ��onTablesUpdates�¼��д���
	if (response && response->getType() == O2GResponseType::MarketDataSnapshot)
	{
		//��ȡ����Ӧ��Ӧ����������Ϣ���˻��������
		sFxcmRequestData reqData = mRequestDataSet[string(requestId)];

		// 10. ��ȡ IO2GResponseReaderFactory:
		O2G2Ptr<IO2GResponseReaderFactory> readerFactory = mSession->getResponseReaderFactory();
		
		if (readerFactory)
		{			
			// 11. ���� IO2GMarketDataSnapshotResponseReader:
			O2G2Ptr<IO2GMarketDataSnapshotResponseReader> reader = readerFactory->createMarketDataSnapshotReader(response);
			if (reader)
			{
				if (reader->size() > 0)
				{
					//���յ����ݸ���
					mRequestDataSet[requestId].getNums += reader->size();

					//����֪ͨ��Ϣ
					try
					{
						boost::format fmessage = boost::format("This time [%s]  received : %s records, total got %s records")
							% mRequestDataSet[requestId].instrument
							% reader->size()
							% mRequestDataSet[requestId].getNums; //"��ѯ��ʷ�۸�"
						std::string message = fmessage.str();
						this->api->sendMessage(message);
					}
					catch (const std::exception&ee)
					{
						std::cout << ee.what() << __LINE__ << std::endl;
					}
					// ����Task,�������ݸ�python�ն�
					Task task = Task();
					task.task_name = OnGetHisPrices_smart;
					task.instrument = mRequestDataSet[requestId].instrument;
					task.task_data = reader;
					//��response���ص�api������������ݽ��б�����������dict����
					//���������ﹹ��boost::python�µ��������ͣ������
					this->api->putTask(task);
					//std::cout << "  end send task " << __LINE__ << std::endl;
					//�ж��Ƿ񷵻���������ʷ���ݣ����endDateΪ�ڼ��գ����ܻ�������
					if (fabs(mRequestDataSet[requestId].endDate - reader->getDate(0)) > 0.0001 //�������������¼�¼�������Ƿ����endDate
																							   /*|| mRequestDataSet[requestId].maxBars > mRequestDataSet[requestId].getNums*/ //maxBarswΪ���η��ص���ʷ���ݵ�������
						)
					{
						mRequestDataSet[requestId].endDate = reader->getDate(0); // �������ݵ����µ�ʱ��
						needquestAgain = true;

						// ������ѯ��ʷ����
						if (needquestAgain)
						{
							//���request
							O2G2Ptr<IO2GRequestFactory> factory = mSession->getRequestFactory();

							//5. ��ȡ IO2GTimeframeCollection:
							O2G2Ptr<IO2GTimeframeCollection> timeframeCollection = factory->getTimeFrameCollection();
							// 6. ��ȡ the IO2GTimeframe �� mRequestDataSet�л�ȡ:
							O2G2Ptr<IO2GTimeframe> timeFrame = timeframeCollection->get(this->mRequestDataSet[requestId].stimeFrame.c_str());

							// ͨ�������ഴ��request
							O2G2Ptr<IO2GRequest> request = factory->createMarketDataSnapshotRequestInstrument(
								mRequestDataSet[requestId].instrument.c_str(),//���Ҷ�����
								timeFrame,	//ʱ���ȣ���m1��m5��h1��
								mRequestDataSet[requestId].maxBars);	//������󷵻��������������fromtime �� totime�е����ݳ���maxbars����ֶ�η���

							factory->fillMarketDataSnapshotRequestTime(
								request,								//����
								mRequestDataSet[requestId].beginDate,	//fromʱ��
								mRequestDataSet[requestId].endDate, false);	// toʱ��

																			//���汾�β�ѯ�������Ϣ	
							sFxcmRequestData reqData;
							reqData.instrument = mRequestDataSet[requestId].instrument;
							reqData.stimeFrame = mRequestDataSet[requestId].stimeFrame;
							reqData.beginDate = mRequestDataSet[requestId].beginDate;
							reqData.endDate = mRequestDataSet[requestId].endDate;
							reqData.getNums = 0;
							this->mRequestDataSet[string(request->getRequestID())] = reqData;
							//�ٴβ�ѯ	
							mSession->sendRequest(request);


							string sBeginDate;
							string sEndDate;
							//0.0 �ᱻת��Ϊstring��1899/12/30 00:00:00��
							Tools::formatDate(mRequestDataSet[requestId].beginDate, sBeginDate);
							Tools::formatDate(mRequestDataSet[requestId].endDate, sEndDate);

							try
							{
								boost::format fmessage = boost::format("Qry Historal Prices : %s from %s to %s")
									% mRequestDataSet[requestId].instrument
									% sBeginDate % sEndDate; //"��ѯ��ʷ�۸�"
								std::string message = fmessage.str();
								//std::cout << "Request Market Instrument prices : " << message << __LINE__ << std::endl;
								//api->sendMessage("req offer price again! req offer price again! req offer price again! req offer price again! req offer price again! req offer price again! ");
								api->sendMessage(message);//���У�ԭ����ʲô����Ȼ�ǻ�����һ�����ֵı����ţ�����
							}
							catch (const std::exception&ee)
							{
								std::cout << ee.what() << __LINE__ << std::endl;
							}
						}
					}
				}
			}			
		}
		return;
	}
	// command request����Ӧ
	if (response && response->getType() == O2GResponseType::GetOffers)  ///CommandResponse  
	{
		PRINTLINE("response && response->getType() == GetOffers");
		//��ӡoffer ��Ϣ
		IO2GOfferRow *resultOffer = NULL;
		O2G2Ptr<IO2GLoginRules> loginRules = mSession->getLoginRules();
		if (loginRules)
		{
			//O2G2Ptr<IO2GResponse> rsp = loginRules->getTableRefreshResponse(Offers);
			//if (response)
			//{
				O2G2Ptr<IO2GResponseReaderFactory> readerFactory = mSession->getResponseReaderFactory();
				if (readerFactory)
				{
					// ��ȡresponse�е�����
					O2G2Ptr<IO2GOffersTableResponseReader> reader = readerFactory->createOffersTableReader(response);
					for (int i = 0; i < reader->size(); ++i)
					{
						O2G2Ptr<IO2GOfferRow> offer = reader->getRow(i);
						if (offer)
						{

							if (strcmp(offer->getSubscriptionStatus(), O2G2::SubscriptionStatuses::ViewOnly) == 0)
								printf("%s : [V]iew only\n", offer->getInstrument());
							else if (strcmp(offer->getSubscriptionStatus(), O2G2::SubscriptionStatuses::Disable) == 0)
								printf("%s : [D]isabled\n", offer->getInstrument());
							else if (strcmp(offer->getSubscriptionStatus(), O2G2::SubscriptionStatuses::Tradable) == 0)
								printf("%s : Available for [T]rade\n", offer->getInstrument());
							else
								printf("%s : %s\n", offer->getInstrument(), offer->getSubscriptionStatus());
							string instrument = offer->getInstrument();							
						}
					}
				}
			//}
		}
		return;
	}

	// ��ѯposition�������Ӧ
	if (response && response->getType() == O2GResponseType::GetTrades)
	{
		O2G2Ptr<IO2GResponseReaderFactory> readerFactory = mSession->getResponseReaderFactory();
		if (readerFactory)
		{
			O2G2Ptr<IO2GTradesTableResponseReader> reader = readerFactory->createTradesTableReader(response);
			
			// ����Task,�������ݸ�python�ն�
			Task task = Task();
			task.task_name = OnQryPosition_smart;
			if (mRequestDataSet[requestId].requestType == O2GResponseType::GetTrades)
			{
				task.instrument = mRequestDataSet[requestId].instrument;
			}
			else
			{
				task.instrument = "Error Instrument";
			}
			
			task.task_data = reader;
			//����task����
			this->api->putTask(task);
		}
		//mRequestComplete = true;

	}
	// ����Order�������Ӧ,�յ�����Ӧ����ζ��Orderִ�гɹ���������ʾ�������յ�������
	if (response->getType() == O2GResponseType::CreateOrderResponse)
	{
		O2G2Ptr<IO2GResponseReaderFactory> readerFactory = mSession->getResponseReaderFactory();
		O2G2Ptr <IO2GOrderResponseReader> reader = readerFactory->createOrderResponseReader(response);
		if (reader)
		{
			string result = " fxcm server recieved the Request of Order : " + string(reader->getOrderID());
			Task task = Task();
			task.task_name = OnMessage_smart;
			task.task_data = result;
			this->api->putTask(task);
		}
	}

	// ��ѯClosedTradeTable����Ӧ
	if (response->getType() == O2GResponseType::GetClosedTrades)
	{
		O2G2Ptr<IO2GResponseReaderFactory> readerFactory = mSession->getResponseReaderFactory();
		O2G2Ptr <IO2GClosedTradesTableResponseReader > reader = readerFactory->createClosedTradesTableReader(response);
		if (reader)
		{
			Task task = Task();
			task.task_name = OnQryClosed_TradesTable_smart;
			task.task_data = reader;
			this->api->putTask(task);
		}
	}

}

void ResponseListener::onRequestFailed(const char * requestId, const char * error)
{
	if (mRequestID == requestId)
	{
		SetEvent(mResponseEvent);
	}
}

/** Request update data received data handler. */
// 
void ResponseListener::onTablesUpdates(IO2GResponse * data)
{
	if (data->getType() == O2GResponseType::TablesUpdates)
	{
		O2GResponseType repType = data->getType();
		PRINTLINE(repType);
		//���Ҷ�ʵʱ���۵Ķ�����Ϣ����
		if (repType == O2GResponseType::TablesUpdates)
		{
			O2G2Ptr<IO2GResponseReaderFactory> factory = mSession->getResponseReaderFactory();
			if (factory)
			{
				O2G2Ptr<IO2GTablesUpdatesReader> reader = factory->createTablesUpdatesReader(data);
				if (reader)
				{
					for (int i = 0; i < reader->size(); i++)
					{
						PRINTLINE(reader->getUpdateTable(i));
						switch (reader->getUpdateTable(i))
						{
							//����Offers��ı仯
							case O2GTable::Offers:
							{
								if (reader->getUpdateType(i) == O2GTableUpdateType::Update
									|| reader->getUpdateType(i) == O2GTableUpdateType::Insert)
								{
									// ����Task
									Task task = Task();
									task.task_name = OnGetSubScribeData_smart;
									task.task_data = reader;// ������reader.Detach()�������������castʱ����� ??
															//��reader���ص�api������������ݽ��б�����������list����
															//���������ﹹ�죬�޷����ݵ�python
									this->api->putTask(task);
								}	
							}
							break;
							case O2GTable::Orders:
							{
								O2G2Ptr<IO2GOrderRow> orderRow = reader->getOrderRow(i);
								
								if (!orderRow)
								{
									PRINTLINE(" orderRow is NULL  ");
									return;
								}
								if (reader->getUpdateType(i) == Insert)
								{
									PRINTLINE("The order has been added.OrderID = '" + string(orderRow->getOrderID()));
									// �˴������������������˸ö��������ǲ���ζ�Ŷ������г�����
									// �ʴ˽�����һ��֪ͨ��Ϣ����
									Task task = Task();
									task.task_name = OnMessage_smart;
									task.task_data = "The order has been added. OrderID = " + string(orderRow->getOrderID());
									this->api->putTask(task);
								}
								else if (reader->getUpdateType(i) == Delete)
								{
									PRINTLINE("The order has been deleted. OrderID = '" + string(orderRow->getOrderID()));
									//��һ���������г����ܺ󣬸ö����ᱻɾ��
									Task task = Task();
									task.task_name = OnMessage_smart;
									task.task_data = "The order has been deleted. OrderID = " + string(orderRow->getOrderID());
									this->api->putTask(task);
								}
								break;
							}
							break;
							case O2GTable::Trades: 
							{
								O2G2Ptr<IO2GTradeRow> tradeRow = reader->getTradeRow(i);
								if (reader->getUpdateType(i) == O2GTableUpdateType::Insert
									|| reader->getUpdateType(i) == O2GTableUpdateType::Update)
								{
									// �ж������г����ܣ���λҲ��仯									
									Task task = Task();
									task.task_name = OnTradesTableUpdate_smart;// Ӧ���޸�ΪOnTradesUpdate
									task.task_data = tradeRow;
									this->api->putTask(task);	

									Task taskmessage = Task();
									taskmessage.task_name = OnMessage_smart;// Ӧ���޸�ΪOnTradesUpdate
									string msg = "Added or Updated trade, TradeId = " + string(tradeRow->getTradeID());
									taskmessage.task_data = msg;
									this->api->putTask(taskmessage);
								}
								else if (reader->getUpdateType(i) == O2GTableUpdateType::Delete)
								{
									Task taskmessage = Task();
									taskmessage.task_name = OnMessage_smart;// Ӧ���޸�ΪOnTradesUpdate
									string msg = "deleted trade, TradeId = " + string(tradeRow->getTradeID());
									taskmessage.task_data = msg;
									this->api->putTask(taskmessage);
								}
								break;
							}
							case O2GTable::Accounts:
							{
								if (reader->getUpdateType(i) == O2GTableUpdateType::Update 
									|| reader->getUpdateType(i) == O2GTableUpdateType::Insert)
								{
									O2G2Ptr<IO2GAccountRow > tradeRow = reader->getAccountRow(i);
									// �˻������¼�
									Task task = Task();
									task.task_name = OnAccountsTableUpdate_smart;
									task.task_data = tradeRow;
									this->api->putTask(task);
								}
								break;
							}
							case O2GTable::ClosedTrades:
							{
								PRINTLINE("O2GTable::ClosedTrades");
								O2G2Ptr<IO2GClosedTradeRow > closedtradeRow = reader->getClosedTradeRow(i);
								if (reader->getUpdateType(i) == O2GTableUpdateType::Update
									|| reader->getUpdateType(i) == O2GTableUpdateType::Insert)
								{
									// �˻������¼�
									Task task = Task();
									task.task_name = OnClosedTradeTableUpdate_smart;
									task.task_data = closedtradeRow;
									this->api->putTask(task);
								}
								break;
							}
							case O2GTable::Messages:
							{
								O2G2Ptr<IO2GMessageRow> messageRow = reader->getMessageRow(i);
								if ( reader->getUpdateType(i) == O2GTableUpdateType::Insert  
									|| reader->getUpdateType(i) == O2GTableUpdateType::Update )
									{
										O2G2Ptr<IO2GMessageRow> messageRow = reader->getMessageRow(i);
										if (messageRow)
								{								
									PRINTLINE(messageRow->getText());
									Task task = Task();
									task.task_name = OnMessage_smart;
									if (reader->getUpdateType(i) == O2GTableUpdateType::Insert)
									{
										task.task_data = "new Message : " + string(messageRow->getText());
									}
									else
									{
										task.task_data = "Update Message : " + string(messageRow->getText());
									}
								
									task.task_last = true;
									this->api->putTask(task);
										}
									break;
								}
							}
							case O2GTable::Summary:
							{
								PRINTLINE("O2GTable::Summary");

								break;
							}
							default:
								break;
						}
					}
				}
				
			}
		}		

	}
}

/** Gets response.*/
IO2GResponse *ResponseListener::getResponse()
{
	if (mResponse)
		mResponse->addRef();
	return mResponse;
}
//
std::string ResponseListener::getOrderID()
{
	return mOrderID;
}
bool ResponseListener::waitEvents()
{
	return WaitForSingleObject(mResponseEvent, _TIMEOUT) == 0;
}

/** Set request. */
void ResponseListener::setRequestID(const char * sRequestID)
{
	if (mResponse)
	{
		mResponse->release();
		mResponse = NULL;
	}
	ResetEvent(mResponseEvent);
}
