#   作者 :             "刘军"
#   创建日期：        2018/10/14

import os
import json
from copy import copy
from datetime import datetime,timedelta

from vnpy.api.fxcm.hxcmapi import HxcmApi
from vnpy.trader.vtGateway import *
from vnpy.trader.vtText import *
from vnpy.trader.vtFunction import getJsonPath

# 方向类型映射
directionMap = {}
directionMap[DIRECTION_LONG] = 'B'
directionMap[DIRECTION_SHORT] = 'S'
directionMapReverse = {v: k for k, v in directionMap.items()}

class HxcmGateway(VtGateway):
    #----------------------------------------------------------------------
    def __init__(self,eventEngine, gatewayName="Hxcm"):
        print("call HxcmGateway __init__")
        # 调用父类的构造函数
        super(HxcmGateway, self).__init__(eventEngine,gatewayName)
        self.fileName = self.gatewayName + "_connect.json"
        self.filePath = getJsonPath(self.fileName,__file__)

        # 载入json文件，读取Gateway配置内容
        try:
            f = open(self.filePath,encoding="utf-8")
        except IOError:
            log = VtLogData()
            log.gatewayName = self.gatewayName
            log.logContent = READ_CONFIGFILE_FAILED
            return
        # 读取json文件
        setting = json.load(f)
        try:
            user = str(setting['user'])
            pwd = str(setting['pwd'])
            url = str(setting['url'])
            connection = str(setting['connection'])
            accountid = str(setting['accountid'])
            print(user)
        except KeyError:
            log = VtLogData()
            log.gatewayName = self.gatewayName
            log.logContent = u'连接配置缺少字段，请检查'
            self.onLog(log)
            return
        # 初始化api接口
        self.api = API(self,user,pwd,url,connection,accountid)
        pass

    # ----------------------------------------------------------------------
    def connect(self):
        # api接口登陆
        self.api.Login(True)
        pass

    # ----------------------------------------------------------------------

class API(HxcmApi):
    '''FXCM的API实现'''
    def __init__(self, gateway,username, pwd, url, conn, accountid):
        super(API, self).__init__(username, pwd, url, conn, accountid)
        self.username = username
        self.pwd = pwd
        self.url = url
        self.conn = conn
        self.accountid = accountid

        # 系统数据的缓存
        self.orderDict = {}
        self.tradeDict = {}
        self.accoutDict = {}
        self.positionDict = {}
        pass

    def writeLog(self,logContent):
        log = VtLogData()
        log.gatewayName = self.gatewayName
        log.logContent = logContent
        self.gateway.onlog(log)

    def onResGetHistoryPrices(self, data, isLast):
        print("onResGetHistoryPrices : " + data['data'])
        pass

    def onSubscribeInstrument(self, data):
        print("onSubscribeInstrument : " + data['data'])
        pass

    def onMessage(self, data):
        print("onMessage : " + data['data'])
        pass

    def onQryPosition(self, data):
        print("onQryPosition : ", data)
        pass

    def onSendOrderResult(self, data):
        print("onSendOrderResult :", data)
        pass

    def onAccountsUpdate(self, data):
        print("onAccountsUpdate : ", data)
        pass

    def onClosedTradeTableUpdate(self, data):
        print("onClosedTradeTableUpdate : ", data)
        pass

    def onTradesTableUpdate(self, data):
        print("onTradesTableUpdate : ", data)
        pass

    def onQryClosed_TradesTable(self, data):
        print("onQryClosed_TradesTable : ", data)
        pass

def main():
    # api = API("701037785", "4616", "http://www.fxcorporate.com/Hosts.jsp", "demo", "1117090")
    # api.Login(True)
    from vnpy.event.eventEngine import EventEngine
    ee = EventEngine()
    hxcmgateway = HxcmGateway(ee)
    hxcmgateway.connect()
    pass

if __name__ == "__main__":
    main()