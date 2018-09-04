#   作者 :             "刘军"
#   创建日期：        2018/8/22

# from SmartTraderFrameWork.trader.gateway.ctpGateway.ctpGateway import CtpGateway
from SmartTraderFrameWork.trader.gateway import ctpGateway
from SmartTraderFrameWork.event.eventEngine2 import EventEngine2
from SmartTraderFrameWork.event.eventEngine import EventEngine

if __name__ == '__main__':
    print('开始测试ctpGateWay接口')
    eventEngine = EventEngine()
    gateway = ctpGateway.gatewayClass(eventEngine)
    gateway.connect()
    gateway.

