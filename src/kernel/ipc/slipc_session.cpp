#include "slipc_session.h"
#include "slkernel.h"
namespace sl{
namespace core{
IPCSession::IPCSession(ITcpSession* pTcpSession, shm::ISLShmQueue* shmQueue, int64 localId, int64 remoteId)
	:m_pTcpSession(pTcpSession),
	m_pShmQueue(shmQueue),
	m_localId(localId),
	m_remoteId(remoteId)
{
	m_pTcpSession->m_pPipe = this;
	m_msgBuf.currSize = 0;
	m_msgBuf.msgSize = 0;
}

IPCSession::~IPCSession()
{
	//m_pTcpSession->close();
}

void IPCSession::release(){
	RELEASE_POOL_OBJECT(IPCSession, this);
}

void IPCSession::onRecv(const char* pBuf, uint32 dwLen){
	m_pTcpSession->onRecv(core::Kernel::getInstance(), pBuf, dwLen);
}

void IPCSession::onEstablish(){
	m_pTcpSession->onConnected(core::Kernel::getInstance());
}

void IPCSession::onTerminate(){
	m_pTcpSession->onDisconnect(core::Kernel::getInstance());
}

void IPCSession::send(const void* pContext, int dwLen){
	int32 nodeId = m_remoteId & 0xFFFFFFFF;
	int32 nodeType = (uint64)m_remoteId >> 32;
	KERNEL_LOG("-------------------------put data on[%d %d]", nodeType, nodeId);
	m_pShmQueue->putData(pContext, dwLen);
}

void IPCSession::close(){

}

const char* IPCSession::getRemoteIP(){
	return Kernel::getInstance()->getInternetIp();
}

int32 IPCSession::procRecv(){
	while (m_pShmQueue->hasCode()){
		int32 readLen = 0;
		m_pShmQueue->getData(m_msgBuf.buf + m_msgBuf.currSize, 4096 - m_msgBuf.currSize, readLen);
		if (m_msgBuf.msgSize == 0 && readLen > 0){
			SLASSERT(m_msgBuf.currSize == 0, "wtf");
			m_msgBuf.msgSize = *(int32*)(m_msgBuf.buf + sizeof(int32));
		}
		m_msgBuf.currSize += readLen;

		if (m_msgBuf.msgSize > 0 && m_msgBuf.currSize >= m_msgBuf.msgSize){
			onRecv(m_msgBuf.buf, m_msgBuf.currSize);
			m_msgBuf.msgSize = 0;
			m_msgBuf.currSize = 0;

			int32 nodeId = m_remoteId & 0xFFFFFFFF;
			int32 nodeType = (uint64)m_remoteId >> 32;
			KERNEL_LOG("+++++++++++++++++get  data from[%d %d]", nodeType, nodeId);
			break;
		}
	}
	return 0;
}

IPCSession* IPCSessionFactory::createSession(shm::ISLShmQueue* queue, int64 localId, int64 remoteId){
	if (NULL == m_pServer)
		return NULL;

	ITcpSession* pTcpSession = m_pServer->mallocTcpSession(core::Kernel::getInstance());
	if (NULL == pTcpSession){
		SLASSERT(false, "wtf");
		return NULL;
	}
	IPCSession* pIPCSession = CREATE_POOL_OBJECT(IPCSession, pTcpSession, queue, localId, remoteId);
	if (NULL == pIPCSession){
		SLASSERT(false, "wtf");
		return NULL;
	}

	return pIPCSession;
}


}
}