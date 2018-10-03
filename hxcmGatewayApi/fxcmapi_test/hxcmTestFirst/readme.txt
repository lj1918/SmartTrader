//添加接口的步骤，以SubScribe（）为例 
1、在hxcmapi.h中添加
//1.1 接口函数：订阅货币对的实时报价，
void Subscribe(string instrument);
// 1.2 添加消息处理函数
void processGetSubScribeData(Task task)
// 1.3 对应的回调函数
virtual void onSubscribeInstrument(dict data) {};

2、在ResponseListener中的void ResponseListener::onTablesUpdates(IO2GResponse * tablesUpdates)事件中
添加对返回的信息的处理逻辑，用于返回
void ResponseListener::onTablesUpdates(IO2GResponse * data)
{
	if ( data->getType() == TablesUpdates)
	{
		// 构造Task
		Task task = Task();
		task.task_name = OnGetSubScribeData;
		task.task_data = data;
		//将response返回到api，在那里对数据进行遍历，并构造dict数据
		//不能在这里构造，无法传递到python
		this->api->putTask(task);
	}
}

3、在hxcmapi.cpp中添加

// 3.0 Subscribe的实现，主动函数由python程序中调用
void Subscribe(string instrument)
{
	。。。。
	//发送给事件处理函数
	this->onSubscribeInstrument(rows);

}
// 3.1 在processTask中处理队列中消息，分发到onSubscribeInstrument函数中

while (1)
	{
		//取出队列中的数据
		Task task = this->task_queue.wait_and_pop();
		switch (task.task_name)
		{
		//历史数据
		case ONGETHISPRICES:
			std::cout << "HxcmApi::processTask  ONGETHISPRICES" << __LINE__ << std::endl;
			this->processGetHisPrices(task);
		case OnGetSubScribeData:
			std::cout << "HxcmApi::processTask OnGetSubScribeData  " << __LINE__ << std::endl;
			this->processGetSubScribeData(task);
		default:
			break;
		}
	}

// 3.2 在HxcmApiWrap 中onSubscribeInstrument的重载
virtual void onSubscribeInstrument(boost::python::list data)
	{
		std::cout << "HxcmApiWrap::onSubscribeInstrument  " << __LINE__ << std::endl;
		try
		{
			this->get_override("onSubscribeInstrument")(data);
		}
		catch (error_already_set const &)
		{
			PyErr_Print();
		}
	};

// 3.3 在BOOST_PYTHON_MODULE(hxcmapi)中输出为python可用的函数
BOOST_PYTHON_MODULE(hxcmapi)
{
	PyEval_InitThreads();	//导入时运行，保证先创建GIL
	class_<HxcmApiWrap, boost::noncopyable>("HxcmApi",init<>())
		.def(init<string,string,string, string>())
		.def("Subscribe",&HxcmApiWrap::Subscribe)
		.def("onSubscribeInstrument",pure_virtual(&HxcmApiWrap::onSubscribeInstrument))
		;
};

4. python的测试代码
