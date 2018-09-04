
from SmartTraderFrameWork.trader.language.chinese import text, constant

from SmartTraderFrameWork.trader.stGlobal import globalSetting
if globalSetting['language'] == 'english':
    from SmartTraderFrameWork.trader.language.english import text, constant
