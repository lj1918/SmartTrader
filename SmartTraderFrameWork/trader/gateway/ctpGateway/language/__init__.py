#   作者 :             "刘军"
#   创建日期：        2018/8/19

import json
import os
import traceback

# 默认设置
from .chinese import text

# 获取全局配置
from SmartTraderFrameWork.trader.stGlobal import globalSetting

# 打开配置文件，读取语言配置
if globalSetting['language'] == 'english':
    from .english import text