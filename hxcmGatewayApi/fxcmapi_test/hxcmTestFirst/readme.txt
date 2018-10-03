//��ӽӿڵĲ��裬��SubScribe����Ϊ�� 
1����hxcmapi.h�����
//1.1 �ӿں��������Ļ��ҶԵ�ʵʱ���ۣ�
void Subscribe(string instrument);
// 1.2 �����Ϣ������
void processGetSubScribeData(Task task)
// 1.3 ��Ӧ�Ļص�����
virtual void onSubscribeInstrument(dict data) {};

2����ResponseListener�е�void ResponseListener::onTablesUpdates(IO2GResponse * tablesUpdates)�¼���
��ӶԷ��ص���Ϣ�Ĵ����߼������ڷ���
void ResponseListener::onTablesUpdates(IO2GResponse * data)
{
	if ( data->getType() == TablesUpdates)
	{
		// ����Task
		Task task = Task();
		task.task_name = OnGetSubScribeData;
		task.task_data = data;
		//��response���ص�api������������ݽ��б�����������dict����
		//���������ﹹ�죬�޷����ݵ�python
		this->api->putTask(task);
	}
}

3����hxcmapi.cpp�����

// 3.0 Subscribe��ʵ�֣�����������python�����е���
void Subscribe(string instrument)
{
	��������
	//���͸��¼�������
	this->onSubscribeInstrument(rows);

}
// 3.1 ��processTask�д����������Ϣ���ַ���onSubscribeInstrument������

while (1)
	{
		//ȡ�������е�����
		Task task = this->task_queue.wait_and_pop();
		switch (task.task_name)
		{
		//��ʷ����
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

// 3.2 ��HxcmApiWrap ��onSubscribeInstrument������
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

// 3.3 ��BOOST_PYTHON_MODULE(hxcmapi)�����Ϊpython���õĺ���
BOOST_PYTHON_MODULE(hxcmapi)
{
	PyEval_InitThreads();	//����ʱ���У���֤�ȴ���GIL
	class_<HxcmApiWrap, boost::noncopyable>("HxcmApi",init<>())
		.def(init<string,string,string, string>())
		.def("Subscribe",&HxcmApiWrap::Subscribe)
		.def("onSubscribeInstrument",pure_virtual(&HxcmApiWrap::onSubscribeInstrument))
		;
};

4. python�Ĳ��Դ���
