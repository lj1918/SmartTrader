#   作者 :             "刘军"
#   创建日期：        2018/8/19

from SmartTraderFrameWork.trader import stConstant
from .ctpGateway import CtpGateway

gatewayClass = CtpGateway
gatewayName = 'CTP'
gatewayDisplayName = 'CTP终端网关'
#GATEWAYTYPE_FUTURES = 'futures'  期货、期权、贵金属
gatewayType = stConstant.GATEWAYTYPE_FUTURES
gatewayQryEnabled = True