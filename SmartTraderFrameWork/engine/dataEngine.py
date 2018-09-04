import shelve # shelve类似于一个存储持久化对象的持久化字典，即字典文件
from SmartTraderFrameWork.event.eventEngine import EventEngine
from SmartTraderFrameWork.event.event import Event
from SmartTraderFrameWork.trader.stEvent import *
from SmartTraderFrameWork.trader.stGlobal import globalSetting
from SmartTraderFrameWork.trader.stFunction import getTempPath
from SmartTraderFrameWork.trader.stConstant import *

class DataEngine(object):
    '''
    数据引擎：
    1、通过注册消息处理函数来从事件引擎接收数据
    2、将收到的数据保存到不同的list中
    '''
    contractFileName = 'ContractData.st'
    contractFilePath = getTempPath(contractFileName)

    FINISHED_STATUS = [STATUS_ALLTRADED, STATUS_REJECTED, STATUS_CANCELLED]
    # ----------------------------------------------------------------------
    def __init__(self,eventEngine,logEngine):
        '''构造函数'''
        # 事件引擎
        self.eventEngine = eventEngine
        # 保存数据的字段
        self.tickDict               = {}
        self.contractDict           = {}
        self.orderDict              = {}
        self.workingOrderDict       = {}    # 可撤销委托
        self.tradeDict              = {}
        self.accountDict            = {}
        self.positionDict           = {}
        self.logList                = []
        self.errorList              = []

        # 持仓细节信息
        self.detailDict             = {}
        self.tdPenaltyList          = globalSetting['tdPenalty'] # 平今手续费惩罚的产品代码列表

        # 读取保存在硬盘的合约数据
        self.loadContracts()

        # 注册事件监听
        self.registerEvent()

        #
        self.logEngine = logEngine

    # ----------------------------------------------------------------------
    def registerEvent(self):
        '''通过EventEngine机制，注册事件监听'''
        self.eventEngine.register(EVENT_TICK,self.processTickEvent)
        self.eventEngine.register(EVENT_CONTRACT,self.processContractEvent)
        self.eventEngine.register(EVENT_ORDER,self.processOrderEvent)
        self.eventEngine.register(EVENT_POSITION,self.processPositionEvent)
        self.eventEngine.register(EVENT_ACCOUNT,self.processAccountEvent)
        self.eventEngine.register(EVENT_TRADE,self.processTradeEvent)
        self.eventEngine.register(EVENT_LOG,self.processLogEvent)
        self.eventEngine.register(EVENT_ERROR,self.processErrorEvent)

    # ----------------------------------------------------------------------
    def processTickEvent(self,event):
        '''处理Tick事件'''
        tick = event.dict_['data']
        self.tickDict[tick.Symbol] = tick

    # ----------------------------------------------------------------------
    def processContractEvent(self,event):
        '''处理合约事件'''
        contract = event.dict_['data']
        self.contractDict[contract.Symbol] = contract

    # ----------------------------------------------------------------------
    def processOrderEvent(self, event):
        """处理委托事件"""
        order = event.dict_['data']
        self.orderDict[order.OrderID] = order

        # 如果订单的状态是全部成交或者撤销，则需要从workingOrderDict中移除
        if order.status in self.FINISHED_STATUS:
            if order.OrderID in self.workingOrderDict:
                del self.workingOrderDict[order.OrderID]
        # 否则则更新字典中的数据
        else:
            self.workingOrderDict[order.OrderID] = order

        # 更新到持仓细节中
        detail = self.getPositionDetail(order.Symbol)
        detail.updateOrder(order)

    # ----------------------------------------------------------------------
    def processTradeEvent(self, event):
        """处理成交事件"""
        trade = event.dict_['data']
        self.tradeDict[trade.TradeID] = trade

        # 更新到持仓细节中
        detail = self.getPositionDetail(trade.Symbol)
        detail.updateTrade(trade)

    # ----------------------------------------------------------------------
    def processPositionEvent(self, event):
        """处理持仓事件"""
        pos = event.dict_['data']
        self.positionDict[pos.PositionName] = pos

        # 更新到持仓细节中
        detail = self.getPositionDetail(pos.Symbol)
        detail.updatePosition(pos)

        # ----------------------------------------------------------------------
    def processAccountEvent(self, event):
        """处理账户事件"""
        account = event.dict_['data']
        self.accountDict[account.AccountID] = account
        self.logEngine.info('processAccountEvent ,%s'%(account))

    # ----------------------------------------------------------------------
    def processLogEvent(self, event):
        """处理日志事件"""
        log = event.dict_['data']
        self.logList.append(log)
        self.logEngine.info(log)

    # ----------------------------------------------------------------------
    def processErrorEvent(self, event):
        """处理错误事件"""
        error = event.dict_['data']
        self.errorList.append(error)

    # ----------------------------------------------------------------------
    def getTick(self, Symbol):
        """查询行情对象"""
        try:
            return self.tickDict[Symbol]
        except KeyError:
            return None

    #----------------------------------------------------------------------
    def getContract(self, Symbol):
        """查询合约对象"""
        try:
            return self.contractDict[Symbol]
        except KeyError:
            return None

    # ----------------------------------------------------------------------
    def getAllContracts(self):
        """查询所有合约对象（返回列表）"""
        return self.contractDict.values()

    # ----------------------------------------------------------------------
    def saveContracts(self):
        """保存所有合约对象到硬盘"""
        f = shelve.open(self.contractFilePath)
        f['data'] = self.contractDict
        f.close()
    #----------------------------------------------------------------------
    def loadContracts(self):
        """从硬盘读取合约对象"""
        f = shelve.open(self.contractFilePath)
        if 'data' in f:
            d = f['data']
            for key, value in d.items():
                self.contractDict[key] = value
        f.close()

    # ----------------------------------------------------------------------
    def getOrder(self, OrderID):
        """查询委托"""
        try:
            return self.orderDict[OrderID]
        except KeyError:
            return None

    # ----------------------------------------------------------------------
    def getAllWorkingOrders(self):
        """查询所有活动委托（返回列表）"""
        return self.workingOrderDict.values()

    # ----------------------------------------------------------------------
    def getAllOrders(self):
        """获取所有委托"""
        return self.orderDict.values()

    # ----------------------------------------------------------------------
    def getAllTrades(self):
        """获取所有成交"""
        return self.tradeDict.values()

    # ----------------------------------------------------------------------
    def getAllPositions(self):
        """获取所有持仓"""
        return self.positionDict.values()

    # ----------------------------------------------------------------------
    def getAllAccounts(self):
        """获取所有资金"""
        return self.accountDict.values()

    # ----------------------------------------------------------------------
    def getPositionDetail(self, Symbol):
        """查询持仓细节"""
        if Symbol in self.detailDict:
            detail = self.detailDict[Symbol]
        else:
            # 返回值为ERROR_FLOAT，表示查询错误
            detail = ERROR_FLOAT
        return detail

    # ----------------------------------------------------------------------
    def getAllPositionDetails(self):
        """查询所有本地持仓缓存细节"""
        return self.detailDict.values()

    # ----------------------------------------------------------------------
    def updateOrderReq(self, req, OrderID):
        """委托请求更新"""
        Symbol = req.Symbol

        detail = self.getPositionDetail(Symbol)
        detail.updateOrderReq(req, OrderID)

    # ----------------------------------------------------------------------
    def convertOrderReq(self, req):
        """根据规则转换委托请求"""
        detail = self.detailDict.get(req.Symbol, None)
        if not detail:
            return [req]
        else:
            return detail.convertOrderReq(req)

    # ----------------------------------------------------------------------
    def getLog(self):
        """获取日志"""
        return self.logList

    # ----------------------------------------------------------------------
    def getError(self):
        """获取错误"""
        return self.errorList



