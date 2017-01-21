#ifndef _IHARBOR_H_
#define _IHARBOR_H_
#include "slikernel.h"
#include "slimodule.h"
#include "slargs.h"
class INodeListener
{
public:
	virtual ~INodeListener() {}
	virtual void onOpen(sl::api::IKernel* pKernel, const int32 nodeType, const int32 nodeId, const char* ip, const int32 port) = 0;
	virtual void onClose(sl::api::IKernel* pKernel, const int32 nodeType, const int32 nodeId) = 0;
};

typedef void(*node_cb)(sl::api::IKernel* pKernel, const int32 nodeType, const int32 nodeId, const char* pContext, const int32 size);
typedef void(*node_args_cb)(sl::api::IKernel* pKernel, const int32 nodeType, const int32 nodeId, const OArgs& args);
class IHarbor : public sl::api::IModule
{
public:
	virtual ~IHarbor() {}
	virtual void addNodeListener(INodeListener* pNodeListener) = 0;
	virtual void prepareSend(int32 nodeType, int32 nodeId, int32 messageId, int32 size) = 0;
	virtual void send(int32 nodeType, int32 nodeId, const void* pContext, const int32 size) = 0;
	virtual void send(int32 nodeType, int32 nodeId, int32 messageId, const OArgs& args) = 0;
	virtual void rgsNodeMessageHandler(int32 messageId, node_args_cb handler) = 0;
	virtual void connect(const char* ip, const int32 port) = 0;
	virtual int32 getNodeType() const = 0;
	virtual int32 getNodeId() const = 0;
};
#endif