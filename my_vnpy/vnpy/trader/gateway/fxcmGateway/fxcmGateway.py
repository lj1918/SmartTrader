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
        # print("call HxcmGateway __init__")
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
            # print(user)
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
        # 调用api接口登陆
        self.api.Login(True)
        pass

    # ----------------------------------------------------------------------
    def subscribe(self, instrument,status,isInstrumentCode = False):
        """
        # 功能：订阅货币对的市场实时报价
        # instrument:货币对名称或代码
        # status: 订阅状态 "T" (enabled), "V" (hidden), and "D" (disabled).
        # isInstrumentCode: 是否为货币对代码
        # self.api.Subscribe('EUR/USD','D',False)
        """
        self.api.Subscribe(instrument, status, isInstrumentCode)
        return
    # ----------------------------------------------------------------------
    def qryInstrumentInfo(self):
        """
        功能： 查询可交易的货币对信息
        :return: 将insrument通过消息机制返回
        """
        InstrumentInfos = self.api.qryInstrumentInfo()
        for instrument in InstrumentInfos:
            instr = VtInstrumentsData()
            instr.symbol = instrument["Instrument"]
            instr.OfferID = instrument["OfferID"]

            instr.PointSize = instrument["PointSize"]  # 一个点的大小（the size of one pip）
            instr.Digits = instrument["Digits"]  # 报价小数点位数
            instr.BaseUnitSize = instrument["BaseUnitSize"]  # 最小交易单位

            instr.MaxQuantity = instrument["MaxQuantity"]  # 一单的最大交易数量
            instr.MinQuantity = instrument["MinQuantity"]  # 一单的最小交易数量
            instr.BuyInterest = instrument["BuyInterest"]  # Buy一个最小交易单位的利息
            instr.SellInterest = instrument["SellInterest"]  #
            instr.TradingStatus = instrument["TradingStatus"]  # 交易状态，O 可交易，C 不可交易
            instr.SubscriptionStatus = instrument["SubscriptionStatus"]  # 订阅状态

            instr.CondDistEntryLimit = instrument["CondDistEntryLimit"]  # 市场价格与entry limit订单价格的最小价差
            instr.CondDistEntryStop = instrument["CondDistEntryStop"]  # the minimal distance between the rates of the entry limit order and the current market rate
            instr.CondDistLimitForTrade = instrument["CondDistLimitForTrade"]  # the minimal distance between the rates of the limit order for the position and the current market rate.
            instr.CondDistStopForTrade = instrument["CondDistStopForTrade"]  # the minimal distance between the rates of the limit order for the position and the current market rate.

            # 合约基础信息推送
            self.gateway.onContract(instr)
        return

    #----------------------------------------------------------------------
    def qryOrders(self):
        """查询订单信息"""
        return

    # ----------------------------------------------------------------------
    def qryPosition(self):
        """查询仓位信息"""
        return

    # ----------------------------------------------------------------------
    def qryAccount(self,accountID):
        """查询账户信息"""
        accountInfo = self.api.qryAccount(accountID)
        account = VtAccountData()
        account.AccountID = accountInfo["AccountID"]
        account.AccountName = accountInfo["AccountName"]
        account.AccountKind = accountInfo["AccountKind"]
        account.AmountLimit = accountInfo["AmountLimit"]
        account.ATPID       = accountInfo["ATPID"]
        account.Balance     = accountInfo["Balance"]
        account.BaseUnitSize =  accountInfo["BaseUnitSize"]
        account.LastMarginCallDate = accountInfo["LastMarginCallDate"]
        account.LeverageProfileID  = accountInfo["LeverageProfileID"]
        account.M2MEquity = accountInfo["M2MEquity"]
        account.MaintenanceFlag = accountInfo["MaintenanceFlag"]
        account.MaintenanceType = accountInfo["MaintenanceType"]
        account.ManagerAccountID = accountInfo["ManagerAccountID"]
        account.MarginCallFlag = accountInfo["MarginCallFlag"]
        account.NonTradeEquity = accountInfo["NonTradeEquity"]
        account.rawData = accountInfo
        
        self.onAccount(account)
        return




class API(HxcmApi):
    '''FXCM的API实现'''
    def __init__(self, gateway,username, pwd, url, conn, accountid):
        super(API, self).__init__(username, pwd, url, conn, accountid)
        self.gateway = gateway
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
        self.offersDict = {}
        pass

    def writeLog(self,logContent):
        log = VtLogData()
        log.gatewayName = self.gatewayName
        log.logContent = logContent
        self.gateway.onlog(log)

    def onLogin(self,data):
        if (data['login_status'] == 3):
            print("登陆成功！")
        elif  (data['login_status'] == 0):
            print("登陆失败！")
        return

    def onResGetHistoryPrices(self, data, isLast):
        print("onResGetHistoryPrices : " + data['data'])
        return

    def onSubscribeInstrument(self, data):
        print("onSubscribeInstrument : " )
        # print(data)
        for row in data:
            offer = VtOfferData()
            offer.instrument = row[0]
            offer.Buy = row[1]
            offer.Sell = row[2]
            self.gateway.onOffer(offer)
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
    hxcmgateway.subscribe('EUR/USD','V',False)
    i = 1
    beginTime = datetime.now()
    # =====================

    while (i):
        time.sleep(1)
        i += 1
    pass

if __name__ == "__main__":
    main()