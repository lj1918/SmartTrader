#   作者 :             "刘军"
#   创建日期：        2018/10/12

from vnpy.trader import vtConstant
from .fxcmGateway import FxcmGateway

gatewayClass = FxcmGateway
gatewayName = "FXCM"
gatewayDisplayName = "福汇"
gatewayType = vtConstant.GATEWAYTYPE_INTERNATIONAL
gatewayQryEnabled = False