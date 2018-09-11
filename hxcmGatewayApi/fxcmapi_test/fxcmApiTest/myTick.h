#pragma once
#include "stdafx.h"
#include "boost/thread/thread.hpp"
class myTick:boost::thread
{
public:
	myTick();
	~myTick();
};

