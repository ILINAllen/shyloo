#define SL_DLL_EXPORT
#include "slnet_module.h"
#include "slnet.h"
namespace sl
{
SL_SINGLETON_INIT(network::CSLNetModule);
namespace network
{

extern "C" SL_DLL_API ISLNet* SLAPI getSLNetModule()
{
	CSLNetModule* g_netModulePtr = CSLNetModule::getSingletonPtr();
	if(g_netModulePtr == NULL)
		g_netModulePtr = NEW CSLNetModule();
	return CSLNetModule::getSingletonPtr();
}

CSLNetModule::CSLNetModule()
	:m_dispatcher(),
	 m_networkInterface(NULL),
	 m_listenerVec(),
	 m_connectorVec()
{
	m_networkInterface = new NetworkInterface(&m_dispatcher);
}

CSLNetModule::~CSLNetModule(){
	int32 size = m_connectorVec.size();
	for (int32 i = 0; i < size; i++){
		if (m_connectorVec[i])
			m_connectorVec[i]->release();
	}

	size = m_listenerVec.size();
	for (int32 i = 0; i < size; i++){
		if (m_listenerVec[i])
			m_listenerVec[i]->release();
	}

	if (nullptr != m_networkInterface)
		DEL m_networkInterface;
}

void CSLNetModule::release(){
	DEL this;
}

ISLListener* CSLNetModule::createListener()
{
	CSLListener* poListener = new CSLListener();
	if (nullptr != poListener)
		m_listenerVec.push_back(poListener);

	return poListener;
}

ISLConnector* CSLNetModule::createConnector()
{
	CSLConnector* poConnector = new CSLConnector();
	if (nullptr != poConnector)
		m_connectorVec.push_back(poConnector);

	return poConnector;
}

bool CSLNetModule::run(int64 overtime)
{
	int64 tick = sl::getTimeMilliSecond();
	m_networkInterface->checkDestroyChannel();
	
	int64 netOverTime = overtime - sl::getTimeMilliSecond() + tick;
	netOverTime = netOverTime >= 0 ? netOverTime : 0;
	m_dispatcher.processOnce(netOverTime);
	
	return true;
}

}
}