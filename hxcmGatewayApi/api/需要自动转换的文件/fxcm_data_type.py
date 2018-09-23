

O2GTable = {}
O2GTable['TableUnknown'] 	= -1
O2GTable['Offers'] 			= 0
O2GTable['Accounts'] 		= 1
O2GTable['Orders'] 			= 2
O2GTable['Trades'] 			= 3
O2GTable['ClosedTrades'] 	= 4
O2GTable['Messages'] 		= 5
O2GTable['Summary'] 		= 6
enumDict['O2GTable'] = O2GTable

O2GResponseType = {}
O2GResponseType['ResponseUnknown'] = -1
O2GResponseType['TablesUpdates'] = 0
O2GResponseType['MarketDataSnapshot'] = 1
O2GResponseType['GetAccounts'] = 2
O2GResponseType['GetOffers'] = 3
O2GResponseType['GetOrders'] = 4
O2GResponseType['GetTrades'] = 5
O2GResponseType['GetClosedTrades'] = 6
O2GResponseType['GetMessages'] = 7
O2GResponseType['CreateOrderResponse'] = 8
O2GResponseType['GetSystemProperties'] = 9
O2GResponseType['CommandResponse'] = 10
O2GResponseType['MarginRequirementsResponse'] = 11
O2GResponseType['GetLastOrderUpdate'] = 12
O2GResponseType['MarketData'] = 13
O2GResponseType['Level2MarketData'] = 14
enumDict['O2GResponseType'] = O2GResponseType

O2GTableUpdateType = {}
O2GTableUpdateType['UpdateUnknown'] = - 1
O2GTableUpdateType['Insert'] = 0
O2GTableUpdateType['Update'] = 1
O2GTableUpdateType['Delete'] = 2
enumDict['O2GTableUpdateType'] = O2GTableUpdateType

O2GPermissionStatus = {}
O2GPermissionStatus['PermissionDisabled'] = 0
O2GPermissionStatus['PermissionEnabled'] = 1
O2GPermissionStatus['PermissionUnknown'] = 2
O2GPermissionStatus['PermissionHidden'] = -2
enumDict['O2GPermissionStatus'] = O2GPermissionStatus

O2GMarketStatus = {}
O2GPermissionStatus['MarketStatusOpen'] = 0
O2GPermissionStatus['MarketStatusClosed'] = 1
O2GPermissionStatus['MarketStatusUndefined'] = 2
enumDict['O2GMarketStatus'] = O2GMarketStatus

O2GPriceUpdateMode = {}
O2GPriceUpdateMode['Default'] = 0
O2GPriceUpdateMode['NoPrice'] = 1
enumDict['O2GPriceUpdateMode'] = O2GPriceUpdateMode

