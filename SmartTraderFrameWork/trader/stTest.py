#   作者 :             "刘军"
#   创建日期：        2018/8/18
import sys
from importlib import  reload
reload(sys)
import platform
system = platform.system()

from SmartTraderFrameWork.engine.mainEngine import MainEngine
from SmartTraderFrameWork.event.eventEngine import EventEngine
from SmartTraderFrameWork.log.logEngine import LogEngine
from SmartTraderFrameWork.trader.stText import *
from SmartTraderFrameWork.trader.stConstant import *
from SmartTraderFrameWork.trader.gateway import ctpGateway

def main():
    ee = EventEngine()
    me = MainEngine(ee)
    me.addGateway(ctpGateway)
    me.logEngine.info('启动成功')

    elog = LogEngine()
    elog.info('第二个时间log')

    print(ctpGateway.gatewayName)
    gw = ctpGateway.ctpGateway.CtpGateway(ee)
    gw.connect()
    gw.qryAccount()
    gw.subscribe('IC1706')






if __name__ == '__main__':
    main()
