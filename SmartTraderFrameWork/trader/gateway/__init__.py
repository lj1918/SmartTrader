#   作者 :             "刘军"
#   创建日期：        2018/8/19

from SmartTraderFrameWork.trader import stConstant
from SmartTraderFrameWork.trader.gateway.ctpGateway.ctpGateway import CtpGateway

gatewayClass = CtpGateway
gatewayName = 'CTP'
gatewayDisplayName = 'CTP'
gatewayType = stConstant.GATEWAYTYPE_FUTURES
gatewayQryEnabled = True