import logging
from logging.handlers import TimedRotatingFileHandler
from datetime import datetime
from  SmartTraderFrameWork.event.event import Event
from SmartTraderFrameWork.trader.stSingleton import stSingleton
from SmartTraderFrameWork.trader.stFunction import getTempPath

class LogEngine(object,
                metaclass=stSingleton):
    # 日志级别
    LEVEL_DEBUG = logging.DEBUG
    LEVEL_INFO = logging.INFO
    LEVEL_WARN = logging.WARN
    LEVEL_ERROR = logging.ERROR
    LEVEL_CRITICAL = logging.CRITICAL

    # ----------------------------------------------------------------------
    def __init__(self):
        """Constructor"""
        self.logger = logging.getLogger()
        # self.formatter = logging.Formatter('%(asctime)s  %(levelname)s: %(message)s')
        self.formatter = logging.Formatter("%(levelname)s  [%(asctime)s.%(msecs)d][%(filename)s:%(lineno)d][%(process)d:%(threadName)s] %(message)s")
        self.level = self.LEVEL_CRITICAL


        self.consoleHandler = None
        self.fileHandler = None
        self.timedRotatingFileHandler = None

        # 添加NullHandler防止无handler的错误输出
        nullHandler = logging.NullHandler()
        self.logger.addHandler(nullHandler)

        # 日志级别函数映射
        self.levelFunctionDict = {
            self.LEVEL_DEBUG: self.debug,
            self.LEVEL_INFO: self.info,
            self.LEVEL_WARN: self.warn,
            self.LEVEL_ERROR: self.error,
            self.LEVEL_CRITICAL: self.critical,
        }

    # ----------------------------------------------------------------------
    def setLogLevel(self, level):
        """设置日志级别"""
        self.logger.setLevel(level)
        self.level = level

    # ----------------------------------------------------------------------
    def addConsoleHandler(self):
        """添加终端输出"""
        if not self.consoleHandler:
            self.consoleHandler = logging.StreamHandler()
            self.consoleHandler.setLevel(self.level)
            self.consoleHandler.setFormatter(self.formatter)
            self.logger.addHandler(self.consoleHandler)

    # ----------------------------------------------------------------------
    def addFileHandler(self, filename=''):
        """添加文件输出"""
        if not self.fileHandler:
            if not filename:
                filename = 'st_' + datetime.now().strftime('%Y%m%d') + '.log'
            filepath = getTempPath(filename)
            self.fileHandler = logging.FileHandler(filepath)
            self.fileHandler.setLevel(self.level)
            self.fileHandler.setFormatter(self.formatter)
            self.logger.addHandler(self.fileHandler)

    # ----------------------------------------------------------------------
    def addTimedRotatingFileHandler(self, filename='',when='H',interval = 1,backupCount=0):
        """添加文件输出"""
        if not self.fileHandler:
            if not filename:
                filename = 'stlog'
            filepath = getTempPath(filename)
            self.timedRotatingFileHandler = TimedRotatingFileHandler(filename=filepath,
                                                        when=when,
                                                        interval = interval,
                                                        backupCount=backupCount)
            self.timedRotatingFileHandler.setLevel(self.level)
            self.timedRotatingFileHandler.setFormatter(self.formatter)
            self.logger.addHandler(self.timedRotatingFileHandler)
    # ----------------------------------------------------------------------
    def debug(self, msg):
        """开发时用"""
        self.logger.debug(msg)

    # ----------------------------------------------------------------------
    def info(self, msg):
        """正常输出"""
        self.logger.info(msg)

    # ----------------------------------------------------------------------
    def warn(self, msg):
        """警告信息"""
        self.logger.warn(msg)

    # ----------------------------------------------------------------------
    def error(self, msg):
        """报错输出"""
        self.logger.error(msg)

    # ----------------------------------------------------------------------
    def exception(self, msg):
        """报错输出+记录异常信息"""
        self.logger.exception(msg)

    # ----------------------------------------------------------------------
    def critical(self, msg):
        """影响程序运行的严重错误"""
        self.logger.critical(msg)

    # ----------------------------------------------------------------------
    def processLogEvent(self, event):
        """处理日志事件"""
        log = event.dict_['data']
        function = self.levelFunctionDict[log.logLevel]  # 获取日志级别对应的处理函数
        msg = '\t'.join([log.gatewayName, log.logContent])
        function(msg)