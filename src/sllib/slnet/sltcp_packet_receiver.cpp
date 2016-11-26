#include "sltcp_packet_receiver.h"
#include "slchannel.h"
#include "sltcp_packet.h"
namespace sl
{
namespace network
{
static CObjectPool<TCPPacketReceiver> g_objPool("TCPPacketReceiver");
CObjectPool<TCPPacketReceiver>& TCPPacketReceiver::ObjPool()
{
	return g_objPool;
}
TCPPacketReceiver* TCPPacketReceiver::createPoolObject()
{
	return g_objPool.FetchObj();
}

void TCPPacketReceiver::reclaimPoolObject(TCPPacketReceiver* obj)
{
	return g_objPool.ReleaseObj(obj);
}

void TCPPacketReceiver::destroyObjPool()
{
	g_objPool.Destroy();
}

TCPPacketReceiver::SmartPoolObjectPtr TCPPacketReceiver::createSmartPoolObj()
{
	return SmartPoolObjectPtr(new SmartPoolObject<TCPPacketReceiver>(ObjPool().FetchObj(), g_objPool));
}

TCPPacketReceiver::TCPPacketReceiver(EndPoint& endpoint,
									 NetworkInterface& networkInterface)
									 :PacketReceiver(endpoint, networkInterface)
{}

TCPPacketReceiver::~TCPPacketReceiver(){}

bool TCPPacketReceiver::processRecv(bool expectingPacket)
{
	Channel* pChannel = getChannel();
	SL_ASSERT(pChannel != NULL);

	if(pChannel->isCondemn())
	{
		return false;
	}

	TCPPacket* pReceiveWindow = TCPPacket::createPoolObject();
	int len = pReceiveWindow->recvFromEndPoint(*m_pEndPoint);

	if(len < 0)
	{
		TCPPacket::reclaimPoolObject(pReceiveWindow);

		PacketReceiver::RecvState rstate = this->checkSocketErrors(len, expectingPacket);

		if(rstate == PacketReceiver::RECV_STATE_INTERRUPT)
		{
			onGetError(pChannel);
			return false;
		}
		return rstate = PacketReceiver::RECV_STATE_CONTINUE;
	}
	else if(len == 0)	///�����˳�
	{
		TCPPacket::reclaimPoolObject(pReceiveWindow);
		onGetError(pChannel);
		return false;
	}

	Reason ret = this->processPacket(pChannel, pReceiveWindow);
	if(ret != REASON_SUCCESS)
	{

	}
	return true;
}

void TCPPacketReceiver::onGetError(Channel* pChannel)
{
	pChannel->condemn();
}

Reason TCPPacketReceiver::processFilteredPacket(Channel* pChannel, Packet* pPacket)
{
	if(pPacket)
	{
		pChannel->addReceiveWindow(pPacket);
	}

	return REASON_SUCCESS;
}

PacketReceiver::RecvState TCPPacketReceiver::checkSocketErrors(int len, bool expectingPacket)
{
#ifdef SL_OS_WINDOWS
	DWORD wsaErr = WSAGetLastError();
#endif // SL_OS_WINDOWS

	if(
#ifdef SL_OS_WINDOWS
		wsaErr = WSAEWOULDBLOCK && !expectingPacket		///< Send��������ǻ��������ˣ� recv�����Ѿ������ݿɶ���
#else
		errno = EAGAIN && !expectingPacket
#endif
		)
	{
		return RECV_STATE_BREAK;
	}

#ifdef  SL_OS_LINUX
	if(errno == EAGAIN ||				///< �Ѿ������ݿɶ���
		errno == ECONNREFUSED ||		///< ���ӱ��������ܾ�
		errno == EHOSTUNREACH)			///< Ŀ�ĵ�ַ���ɵ���
	{
		return RECV_STATE_BREAK;
	}
#else

#endif //  SL_OS_LINUX

	return RECV_STATE_CONTINUE;


}


}
}