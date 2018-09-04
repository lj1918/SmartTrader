class stSingleton(type):
    """
    单例，应用方式:静态变量 class  xxx(object,metaclass = Singleton)
    """
    _instances = {}
    # ----------------------------------------------------------------------
    def __call__(cls, *args, **kwargs):
        """调用"""
        if cls not in cls._instances:
            cls._instances[cls] = super(stSingleton, cls).__call__(*args, **kwargs)
        return cls._instances[cls]
