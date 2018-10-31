#   作者 :             "刘军"
#   创建日期：        2018/10/12

from SmartTraderFrameWork.trader import stConstant
from .fxcmGateway import FxcmGateway

gatewayClass = FxcmGateway
gatewayName = "FXCM"
gatewayDisplayName = "福汇"
gatewayType = stConstant.GATEWAYTYPE_INTERNATIONAL # 外汇
gatewayQryEnabled = False