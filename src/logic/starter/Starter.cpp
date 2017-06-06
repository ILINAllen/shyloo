#include "Starter.h"
#include "IHarbor.h"
#include "slxml_reader.h"
#include "slobjectpool.h"
#include "StartNodeTimer.h"
#include "slargs.h"
#include "NodeDefine.h"
#include "NodeProtocol.h"
using namespace sl;

bool Starter::initialize(sl::api::IKernel * pKernel){
	_self = this;
	_kernel = pKernel;
	return true;
}

bool Starter::launched(sl::api::IKernel * pKernel){
	FIND_MODULE(_harbor, Harbor);

	sl::XmlReader server_conf;
	if (!server_conf.loadXml(pKernel->getCoreFile())){
		SLASSERT(false, "load core file %s failed", pKernel->getCoreFile());
		return false;
	}
	_harbor->addNodeListener(this);

	const sl::xml::ISLXmlNode& starter = server_conf.root()["starter"][0];
	_checkInterval = starter.getAttributeInt64("check");
	_deadTime = starter.getAttributeInt64("dead");
	const sl::xml::ISLXmlNode& nodes = server_conf.root()["starter"][0]["node"];
	for (int32 i = 0; i < nodes.count(); i++){
		Execute info;
		info.type = nodes[i].getAttributeInt32("type");
		info.min = nodes[i].getAttributeInt32("min");
		info.max = nodes[i].getAttributeInt32("max");
		info.delay = nodes[i].getAttributeInt32("delay");
		info.timer = StartNodeTimer::create(info.type);
		_executes[info.type] = info;
		SLASSERT(info.timer, "wtf");
		START_TIMER(info.timer, info.delay, TIMER_BEAT_FOREVER, _checkInterval);
	}
	return true;
}

bool Starter::destory(sl::api::IKernel * pKernel){
	DEL this;
	return true;
}

void Starter::onNodeTimerStart(sl::api::IKernel * pKernel, int32 type, int64 tick){
	auto itor = _executes.find(type);
	if (itor == _executes.end()){
		SLASSERT(false, "wtf");
		return;
	}

	for (int32 i = 1; i <= itor->second.min; i++){
		_self->startNode(pKernel, type, i);
	}
	_nodes[type].max = itor->second.min;
}

void Starter::onNodeTimer(sl::api::IKernel * pKernel, int32 type, int64 tick){

}

void Starter::onNodeTimerEnd(sl::api::IKernel * pKernel, int32 type, int64 tick){
	if (_executes.find(type) == _executes.end())
		return;

	if (_executes[type].timer == nullptr){
		SLASSERT(false, "execute no timer");
		return;
	}
	_executes[type].timer->release();
	_executes[type].timer = nullptr;
}

void Starter::onOpen(sl::api::IKernel* pKernel, const int32 nodeType, const int32 nodeId, const char* ip, const int32 port){
	if (nodeType == NodeType::SLAVE){
		return;
	}

	if (_executes.find(nodeType) == _executes.end()){
		return;
	}

	_nodes[nodeType].nodes[nodeId].online = true;
	_nodes[nodeType].nodes[nodeId].closeTick = 0;
}

void Starter::onClose(sl::api::IKernel* pKernel, const int32 nodeType, const int32 nodeId){
	if (nodeType == NodeType::SLAVE){
		return;
	}

	_nodes[nodeType].nodes[nodeId].online = false;
	_nodes[nodeType].nodes[nodeId].closeTick = sl::getTimeMilliSecond();
}

void Starter::startNode(sl::api::IKernel* pKernel, int32 nodeType, int32 nodeId){
	IArgs<2, 128> args;
	args << nodeType << nodeId;
	args.fix();
	_harbor->send(NodeType::SLAVE, 1, NodeProtocol::MASTER_MSG_START_NODE, args.out());
	TRACE_LOG("start new Node %d %d", nodeType, nodeId);
}