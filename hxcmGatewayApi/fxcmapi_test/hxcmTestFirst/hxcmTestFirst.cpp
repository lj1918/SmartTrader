// hxcmTestFirst.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "hxcmapi.h"

int main()
{
	HxcmApi *api = new HxcmApi("701037785", "4616", "http://www.fxcorporate.com/Hosts.jsp", "Demo", "1117090");
	api->Login(true);
	api->SendOpenMarketOrder("EUR/USD",
		"1117090",
		"B",
		1000,
		8.88,
		"lj721226");
	while (true)
	{
		;
	}
    return 0;
}

