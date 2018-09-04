# 系统模块
# 自己开发的模块
from SmartTraderFrameWork.event.eventType import *

class Event(object):
    """事件对象"""

    #----------------------------------------------------------------------
    def __init__(self, type_=None):
        """构造函数"""
        self.type_ = type_      # 事件类型
        self.dict_ = {}         # 字典用于保存具体的事件数据

    def printself(self):
        print('type = ',self.type_,'dict = ',self.dict_)

#----------------------------------------------------------------------
def test():
    event = Event()
    event.type_ = EVENT_TIMER
    event.dict_['test'] = 'ljljljlj '

    event.printself()
    pass

if __name__ == '__main__':
    test()
