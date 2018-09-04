#   作者 :             "刘军"
#   创建日期：        2018/8/20
from datetime import datetime
from collections import OrderedDict

from SmartTraderFrameWork.engine.dataEngine import DataEngine
from SmartTraderFrameWork.trader.stEvent import *
########################################################################
from SmartTraderFrameWork.log.logEngine import LogEngine
from SmartTraderFrameWork.trader.stGlobal import globalSetting


class MainEngine(object):
    """主引擎"""
    # ----------------------------------------------------------------------
    def __init__(self, eventEngine):
        ''''''
        self.todayDate = datetime.now().strftime('%Y%m%d')
        # 首先创建事件引擎，它是整个系统相互调用的核心
        self.eventEngine = eventEngine
        self.eventEngine.start()

        # 日志引擎实例
        self.logEngine = None
        self.initLogEngine()

        self.dataEngine = DataEngine(self.eventEngine,self.logEngine)

        # MongoDB数据库相关
        self.dbClient = None  # MongoDB客户端对象

        # 接口实例
        self.gatewayDict = OrderedDict()
        self.gatewayDetailList = []

        # 应用模块实例
        self.appDict = OrderedDict()
        self.appDetailList = []

        # 风控引擎实例（特殊独立对象）
        self.rmEngine = None
        return
    # ----------------------------------------------------------------------
    def addGateway(self, gatewayModule):
        """添加底层接口"""
        gatewayName = gatewayModule.gatewayName

        # 创建接口实例
        self.gatewayDict[gatewayName] = gatewayModule.gatewayClass(self.eventEngine,
                                                                   gatewayName)

        # 设置接口轮询
        if gatewayModule.gatewayQryEnabled:
            self.gatewayDict[gatewayName].setQryEnabled(gatewayModule.gatewayQryEnabled)

        # 保存接口详细信息
        d = {
            'gatewayName': gatewayModule.gatewayName,
            'gatewayDisplayName': gatewayModule.gatewayDisplayName,
            'gatewayType': gatewayModule.gatewayType
        }
        self.gatewayDetailList.append(d)
        self.logEngine.info('添加Gateway成功，gatewayName = %s, DisplayName = %s ,Type = %s'%(gatewayName,
                                                                               gatewayModule.gatewayDisplayName,
                                                                               gatewayModule.gatewayType))

    # ----------------------------------------------------------------------
    def initLogEngine(self):
        """初始化日志引擎"""
        if not globalSetting["logActive"]:
            return

        # 创建引擎
        self.logEngine = LogEngine()

        # 设置日志级别
        levelDict = {
            "debug": LogEngine.LEVEL_DEBUG,
            "info": LogEngine.LEVEL_INFO,
            "warn": LogEngine.LEVEL_WARN,
            "error": LogEngine.LEVEL_ERROR,
            "critical": LogEngine.LEVEL_CRITICAL,
        }
        level = levelDict.get(globalSetting["logLevel"], LogEngine.LEVEL_CRITICAL)
        self.logEngine.setLogLevel(level)

        # 设置输出
        if globalSetting['logConsole']:
            self.logEngine.addConsoleHandler()

        if globalSetting['logFile']:
            self.logEngine.addFileHandler('z:\log.txt')

        # 注册事件监听
        self.registerLogEvent(EVENT_LOG)

    #----------------------------------------------------------------------
    def getGateway(self, gatewayName):
        """获取接口"""
        if gatewayName in self.gatewayDict:
            return self.gatewayDict[gatewayName]
        else:
            self.writeLog(text.GATEWAY_NOT_EXIST.format(gateway=gatewayName))
            return None

    #----------------------------------------------------------------------
    def registerLogEvent(self, eventType):
        """注册日志事件监听"""
        if self.logEngine:
            self.eventEngine.register(eventType, self.logEngine.processLogEvent)

    # ----------------------------------------------------------------------
    def qryAccount(self, gatewayName):
        """查询特定接口的账户"""
        gateway = self.getGateway(gatewayName)

        if gateway:
            gateway.qryAccount()