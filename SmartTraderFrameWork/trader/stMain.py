import time
from SmartTraderFrameWork.event.event import Event
from SmartTraderFrameWork.event.eventEngine import EventEngine
from SmartTraderFrameWork.log.logEngine import LogEngine

if __name__ == '__main__':
    lg = LogEngine()
    lg.setLogLevel(lg.LEVEL_INFO)
    lg.addConsoleHandler()
    # lg.addFileHandler('z:\mylog.txt')
    lg.addTimedRotatingFileHandler(filename='z:\\mylog.txt',when='M',interval = 5)

    # 循环打印日志
    log_content = "test log"
    count = 0
    while count < 40:
        lg.error(log_content)
        time.sleep(1)
        count = count + 1


