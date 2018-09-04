# 系统模块
from queue import Queue,Empty
from threading import Thread
from collections import defaultdict

# 第三方模块
from PyQt5.QtCore import QTimer
# 自己开发的模块
from SmartTraderFrameWork.event.eventType import *
from SmartTraderFrameWork.event import event
from SmartTraderFrameWork.log.logEngine import LogEngine
########################################################################
class EventEngine(object):
    '''
    方法说明
    __run: 私有方法，事件处理线程连续运行用
    __process: 私有方法，处理事件，调用注册在引擎中的监听函数
    __onTimer：私有方法，计时器固定事件间隔触发后，向事件队列中存入计时器事件
    start: 公共方法，启动引擎
    stop：公共方法，停止引擎
    register：公共方法，向引擎中注册监听函数
    unregister：公共方法，向引擎中注销监听函数
    put：公共方法，向事件队列中存入新的事件

    事件监听函数必须定义为输入参数仅为一个event对象，即：

    函数
    def func(event)
        ...

    对象方法
    def method(self, event)
        ...
    '''

    # ----------------------------------------------------------------------
    def __init__(self):
        '''初始化事件引擎'''
        # 事件引擎开关
        self.__active = False
        # 事件队列
        self.__queue = Queue()
        # 事件处理线程
        self.__thread = Thread( target =self.__run )
        # 计时器，用于触发计时器事件(实现系统的OnTime事件)
        self.__timer = QTimer()
        self.__timer.timeout.connect(self.__onTimer)

        # 这里的__handlers是一个字典，用来保存对应的事件调用关系
        # 其中每个键对应的值是一个列表，列表中保存了对该事件进行监听的函数功能
        self.__handlers = defaultdict(list) #defaultdict:当字典里的key不存在但被查找时，返回的不是keyError而是一个默认值

        # __generalHandlers是一个列表，用来保存通用回调函数（所有事件均调用）
        self.__generalHandlers = []
        return

    # ----------------------------------------------------------------------
    def __run(self):
        """引擎运行"""
        while self.__active == True:
            try:
                event = self.__queue.get( block=True, timeout = 1 )
                self.__process(event)
            except Empty:
                pass
        return

    # ----------------------------------------------------------------------
    def __process(self,event):
        """处理事件"""
        # 检查是否存在对该事件进行监听的处理函数
        if event.type_ in self.__handlers:
            for handler in self.__handlers[event.type_]:
                handler(event)
        # 调用通用处理函数进行处理
        if self.__generalHandlers:
            [handler(event) for handler in self.__generalHandlers]
        return
    # ----------------------------------------------------------------------
    def __onTimer(self):
        """向事件队列中存入计时器事件"""
        # 创建计时器事件
        event = event.Event()
        event.type_ = EVENT_TIMER
        # 向队列中存入计时器事件
        self.put(event)
        return
    # ----------------------------------------------------------------------
    def start(self,timer=True):
        """
        引擎启动
        timer：是否要启动计时器
        """
        # 将引擎设为启动
        self.__active = True
        # 启动事件处理线程
        self.__thread.start()
        # 启动计时器，计时器事件间隔默认设定为1秒
        if timer:
            self.__timer.start(1000)
        return
    # ----------------------------------------------------------------------
    def stop(self):
        """停止引擎"""
        # 将引擎设为停止
        self.__active = False

        # 停止计时器
        self.__timer.stop()

        # 等待事件处理线程退出
        self.__thread.join()
        return
    # ----------------------------------------------------------------------
    def register(self,type_,handler):
        # 尝试获取该事件类型对应的处理函数列表，若无defaultDict会自动创建新的list
        handlerList = self.__handlers[type_]
        # 若要注册的处理器不在该事件的处理器列表中，则注册该事件
        if handler not in handlerList:
            self.__handlers[type_].append(handler)
            # handlerList.append(handler) # 这样写也可以，python的值引用之类的鬼东东
        return
    # ----------------------------------------------------------------------
    def unregister(self,type_,handler):
        """注销事件处理函数监听"""
        # 尝试获取该事件类型对应的处理函数列表，若无则忽略该次注销请求
        handlerList = self.__handlers[type_]

        # 如果该函数存在于列表中，则移除
        if handler in handlerList:
            handlerList.remove(handler)

        # 如果函数列表为空，则从引擎中移除该事件类型
        if not handlerList:
            del self.__handlers[type_]
        return

    # ----------------------------------------------------------------------
    def registerGeneralHandler(self, handler):
        """注册通用事件处理函数监听"""
        if handler not in self.__generalHandlers:
            self.__generalHandlers.append(handler)

    # ----------------------------------------------------------------------
    def unregisterGeneralHandler(self, handler):
        """注销通用事件处理函数监听"""
        if handler in self.__generalHandlers:
            self.__generalHandlers.remove(handler)
    # ----------------------------------------------------------------------
    def put(self,event):
        self.__queue.put(event)
        return

# ----------------------------------------------------------------------
def test():
    """测试函数"""
    import sys
    from datetime import datetime
    from qtpy.QtCore import QCoreApplication
    import  threading
    print(threading.current_thread())
    f = open('z:\log1.txt', 'w', encoding='UTF-8')
    f.write('设施老客户士大夫了')
    def simpletest(event):
        print('call sample')
        f.write('处理每秒触发的计时器事件：{}\n'.format(str(datetime.now())))
        f.flush()

    app = QCoreApplication(sys.argv)

    ee = EventEngine()
    ee.register(EVENT_TIMER, simpletest)
    ee.registerGeneralHandler(simpletest)
    ee.start()

    app.exec_()
    f.flush()
# 直接运行脚本可以进行测试
if __name__ == '__main__':
    test()
