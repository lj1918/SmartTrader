# encoding: UTF-8

from vnpy.trader.language import text

# 将常量定义添加到vtText.py的局部字典中
d = locals()
for name in dir(text):
    if '__' not in name:
        # __getattribute__是属性访问拦截器
        d[name] = text.__getattribute__(name)
