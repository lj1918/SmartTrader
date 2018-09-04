from SmartTraderFrameWork.trader.language import constant

# 将常量定义添加到vtConstant.py的局部字典中
# 有趣的机制，
d = locals()
for name in dir(constant):
    if '__' not in name:
        d[name] = constant.__getattribute__(name)
d = {}

def test_en():
    from SmartTraderFrameWork.trader.language.english import constant
    # 将常量定义添加到stConstant.py的局部字典中
    d = locals()
    for name in dir(constant):
        if '__' not in name:
            d[name] = constant.__getattribute__(name)
            print(constant.__getattribute__(name))
def test_cn():
    from SmartTraderFrameWork.trader.language.chinese import constant
    # 将常量定义添加到stConstant.py的局部字典中
    d = locals()
    for name in dir(constant):
        if '__' not in name:
            d[name] = constant.__getattribute__(name)
            print(constant.__getattribute__(name))
if __name__  == '__main__':
    test_en()