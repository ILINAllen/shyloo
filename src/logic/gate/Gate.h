#ifndef __SL_FRAMEWORK_GATE_H__
#define __SL_FRAMEWORK_GATE_H__
#include "slikernel.h"
#include "slimodule.h"
#include "IHarbor.h"
#include "IAgent.h"
#include "IGate.h"
#include "slbinary_stream.h"
#include <unordered_map>
#include <unordered_set>
using namespace sl;
class Gate :public IGate, public IAgentListener, public INodeListener{
	enum {
		GATE_STATE_NONE = 0,
		GATE_STATE_ROLELOADED,
		GATE_STATE_DISTRIBUTE,
		GATE_STATE_BINDING,
		GATE_STATE_ONLINE,
	};

	struct Role{
		int64 actorId;
	};

	struct Player{
		int64 agentId;
		int64 selectActorId;
		int64 accountId;
		int32 logic;
		int8 state;

	};

	typedef void(Gate::*agent_cb)(sl::api::IKernel* pKernel, const int64 id, const char* pContext, const int32 size);
	typedef void(Gate::*agent_args_cb)(sl::api::IKernel* pKernel, const int64 id, const OBStream& args);

public:
	virtual bool initialize(sl::api::IKernel * pKernel);
	virtual bool launched(sl::api::IKernel * pKernel);
	virtual bool destory(sl::api::IKernel * pKernel);

	virtual void onAgentOpen(sl::api::IKernel* pKernel, const int64 id);
	virtual int32 onAgentRecv(sl::api::IKernel* pKernel, const int64 id, const void* context, const int32 size);
	virtual void onAgentClose(sl::api::IKernel* pKernel, const int64 id);

	virtual void onOpen(sl::api::IKernel* pKernel, const int32 nodeType, const int32 nodeId, const char* ip, const int32 port);
	virtual void onClose(sl::api::IKernel* pKernel, const int32 nodeType, const int32 nodeId);

	void onClientLoginReq(sl::api::IKernel* pKernel, const int64 id, const OBStream& args);

	void rgsAgentMessageHandler(int32 messageId, agent_args_cb handler);
	void transMsgToLogic(sl::api::IKernel* pKernel, const int64 id, const void* pContext, const int32 size);

	
private:
	static Gate*		s_gate;
	static sl::api::IKernel* s_kernel;
	static IHarbor*		s_harbor;
	static IAgent*		s_agent;

	static std::unordered_map<int64, Player> s_players;
	static std::unordered_map<int32, std::unordered_set<int64>> s_logicPlayers;

	static std::unordered_map<int32, agent_args_cb> s_gateProtos;
};

#endif