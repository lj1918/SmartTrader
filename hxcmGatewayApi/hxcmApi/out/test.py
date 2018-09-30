from hxcmapi import  HxcmApi
from importlib import reload
from datetime import datetime
import pandas as pd
import time
import numpy as np

if __name__ == '__main__':
    LoginSuccessed  =    999
    msgDict = {}
    msgDict[LoginSuccessed] = "Login Successed"
    class test(HxcmApi):
        filename = 'd:\\record.csv'
        def __init__(self,username,pwd,url,conn,accountid):
            super(test, self).__init__(username,pwd,url,conn,accountid)
            pass
        def onResGetHistoryPrices(self, data, isLast):
            # print(data['nums'])
            # print(data['data'])
            # a = pd.DataFrame(data['data'])
            # a.to_csv('d:\\log.csv',mode='a',header=False)
            pass

        def onSubscribeInstrument(self,data):
        #     #print("call onSubscribeInstrument")
        #     print(data)
            pass
        def onMessage(self,data):
            # print(type(data))
            # print{'ljl'}
            # print("get:\n",data['data'])
            # print(data['data'])
            # msgDict[data['data']]
            pass
        def onQryPosition(self,data):
            print("*"*45)
            print(data)
            pass

    api = test("701037785","4616", "http://www.fxcorporate.com/Hosts.jsp","demo","1117090")
    api.Login(True)
    result = api.qryAccount("1117090")
    # print(result)
    result = api.qryInstrumentInfo()
    print(result)
    # print(result['bb'])
    api.qryPosition("EUR/USD")
    # aa = {'m5': ['EUR/USD', 'USD/JPY']}
    # api.regTick(aa)
    api.qryHisPrices('EUR/USD','m1',300,'2018-08-16 01:00:00 ','2018-08-16 04:01:00')
    # api.qryHisPrices('EUR/USD', 'm1', 1, '2018-09-05 23:01:17', '2018-09-12 23:01:17')
    # api.StartTick("m1")
    # print("test test test test ")
    # api.StartTick("m5")
    # print("www www www www ")
    # api.qryLastHisPrice('EUR/USD', 'H1')
    # print(dir(api))
    # api.Subscribe('EUR/USD','V',False)
    i = 1
    beginTime = datetime.now()
    #=====================

    while(i ):
        # api.qryHisPrices('EUR/USD', 'm1', 1, '2018-09-05 23:01:17', '2018-09-06 23:01:17')
        time.sleep(1)
        # print(datetime.now(),"=="*30)
        i+=1
    api.Logout()
    # api.Subscribe('USD/JPY', 'V', True)
    # while(i < 5):
    #     time.sleep(1)
    #     i+=1
    #     print(i)
#