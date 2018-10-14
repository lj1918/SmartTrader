#   作者 :             "刘军"
#   创建日期：        2018/8/23

from vnpy.trader.gateway.fxcmGateway.fxcmGateway import FxcmGateway
from vnpy.event.eventEngine import EventEngine

if __name__ == '__main__':
    ee  = EventEngine()
    fm = FxcmGateway(ee)
    fm.connect()