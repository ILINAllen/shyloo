#define SL_DLL_EXPORT
#include "slshm_mgr.h"
namespace sl{
SL_SINGLETON_INIT(shm::SLShmMgr);
namespace shm{
ISLShmQueue* SLShmMgr::createShmQueue(bool bBackEnd, const char* pszShmKey, int iShmSize){
	return (ISLShmQueue*)CREATE_FROM_POOL(_shmQueues, bBackEnd, pszShmKey, iShmSize);
}

void SLShmMgr::recover(ISLShmQueue* queue){
	if (queue)
		_shmQueues.recover((SLShmQueue*)queue);
}

extern "C" SL_DLL_API ISLShmMgr* SLAPI newShmMgr(void){
	SLShmMgr* g_shmMgr = SLShmMgr::getSingletonPtr();
	if (g_shmMgr == NULL)
		g_shmMgr = NEW SLShmMgr();
	return SLShmMgr::getSingletonPtr();
}
}
}