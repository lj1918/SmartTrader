from SmartTraderFrameWork.event.event import Event
from SmartTraderFrameWork.trader.stEvent import *
from SmartTraderFrameWork.trader.stConstant import *
from SmartTraderFrameWork.trader.stModel import *

########################################################################
class  Gateway(object):
    """
    交易接口的父类
    功能：
    从外部数据源接口中获取数据，然后发送到事件引擎
    包含了大量的用于回调的函数：（self，event）
    """

    # ----------------------------------------------------------------------
    def __init__(self,eventEngine,gatewayName):
        self.eventEngine = eventEngine
        self.gatewayName = gatewayName

    # ----------------------------------------------------------------------
    def onTick(self, tick):
        """市场行情推送"""
        # 通用事件
        event1 = Event(type_=EVENT_TICK)
        event1.dict_['data'] = tick
        self.eventEngine.put(event1)
        pass
    # ----------------------------------------------------------------------
    def onTrade(self, trade):
        """成交信息推送"""
        # 通用事件
        event1 = Event(type_=EVENT_TRADE)
        event1.dict_['data'] = trade
        pass
    # ----------------------------------------------------------------------
    def onOrder(self, order):
        """订单变化推送"""
        # 通用事件
        event1 = Event(type_=EVENT_ORDER)
        event1.dict_['data'] = order
        self.eventEngine.put(event1)
        pass

    # ----------------------------------------------------------------------
    def onPosition(self, position):
        """持仓信息推送"""
        # 通用事件
        event1 = Event(type_=EVENT_POSITION)
        event1.dict_['data'] = position
        self.eventEngine.put(event1)
        pass

    #----------------------------------------------------------------------
    def onAccount(self, account):
        """账户信息推送"""
        # 通用事件
        event1 = Event(type_=EVENT_ACCOUNT)
        event1.dict_['data'] = account
        self.eventEngine.put(event1)

    #----------------------------------------------------------------------
    def onError(self, error):
        """错误信息推送"""
        # 通用事件
        event1 = Event(type_=EVENT_ERROR)
        event1.dict_['data'] = error
        self.eventEngine.put(event1)

    # ----------------------------------------------------------------------
    def onLog(self, log):
        """日志推送"""
        # 通用事件
        event1 = Event(type_=EVENT_LOG)
        event1.dict_['data'] = log
        self.eventEngine.put(event1)

    # ----------------------------------------------------------------------
    def onContract(self, contract):
        """合约基础信息推送"""
        # 通用事件
        event1 = Event(type_=EVENT_CONTRACT)
        event1.dict_['data'] = contract
        self.eventEngine.put(event1)

    # ----------------------------------------------------------------------
    def connect(self):
        """连接"""
        pass

    # ----------------------------------------------------------------------
    def subscribe(self, subscribeReq):
        """订阅行情"""
        pass

    # ----------------------------------------------------------------------
    def sendOrder(self, orderReq):
        """发单"""
        pass

    # ----------------------------------------------------------------------
    def cancelOrder(self, cancelOrderReq):
        """撤单"""
        pass

    # ----------------------------------------------------------------------
    def qryAccount(self):
        """查询账户资金"""
        pass

    # ----------------------------------------------------------------------
    def qryPosition(self):
        """查询持仓"""
        pass

    # ----------------------------------------------------------------------
    def close(self):
        """关闭"""
        pass

