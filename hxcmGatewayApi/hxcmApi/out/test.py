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
        def __init__(self,username,pwd,url,conn):
            super(test, self).__init__(username,pwd,url,conn)
            pass
        def onResGetHistoryPrices(self, data, isLast):
            # print(data['nums'])
            print(data['data'])
            # a = pd.DataFrame(data['data'])
            # a.to_csv('d:\\log.csv',mode='a',header=False)
            pass

        def onSubscribeInstrument(self,data):
        #     #print("call onSubscribeInstrument")
            print(data)
            pass
        def onMessage(self,data):
            # print(type(data))
            # print{'ljl'}
            # print("get:\n",data['data'])
            # print(data['data'])
            # msgDict[data['data']]
            pass

    api = test("701037785","4616", "http://www.fxcorporate.com/Hosts.jsp","demo")
    api.Login(True)
    result = api.qryAccount()
    print(result)
    # print(result['bb'])
    # aa = {'m5': ['EUR/USD', 'USD/JPY']}
    # api.regTick(aa)
    # api.qryHisPrices('EUR/USD','H1',300,'2018-08-16 01:00:00 ','2018-08-16 04:01:00')
    # api.qryHisPrices('EUR/USD', 'm1', 1, '2018-09-05 23:01:17', '2018-09-12 23:01:17')
    # api.StartTick("m1")
    # print("test test test test ")
    # api.StartTick("m5")
    # print("www www www www ")
    # api.qryLastHisPrice('EUR/USD', 'H1')
    # print(dir(api))
    # api.Subscribe('EUR/USD','D',True)
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