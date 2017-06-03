#include "AOI.h"
#include "IHarbor.h"
#include "NodeDefine.h"
#include "NodeProtocol.h"
#include "IDCCenter.h"
#include "Attr.h"
#include "IShadowMgr.h"
#include "IPacketSender.h"
#include "Protocol.pb.h"
#include "ProtocolID.pb.h"
#include "EventID.h"
#include "IEventEngine.h"
#include <unordered_map>
#include <vector>

bool AOI::initialize(sl::api::IKernel * pKernel){
	_self = this;
	return true;
}

bool AOI::launched(sl::api::IKernel * pKernel){
	FIND_MODULE(_harbor, Harbor);
	if (_harbor->getNodeType() == NodeType::LOGIC){
		FIND_MODULE(_objectMgr, ObjectMgr);
		FIND_MODULE(_shadowMgr, ShadowMgr);
		FIND_MODULE(_packetSender, PacketSender);
		FIND_MODULE(_eventEngine, EventEngine);

		RGS_NODE_ARGS_HANDLER(_harbor, NodeProtocol::SCENE_MSG_ADD_INTERESTER, AOI::onSceneAddInterester);
		RGS_NODE_ARGS_HANDLER(_harbor, NodeProtocol::SCENE_MSG_ADD_WATCHER, AOI::onSceneAddWatcher);
		RGS_NODE_ARGS_HANDLER(_harbor, NodeProtocol::SCENE_MSG_REMOVE_INTERESTER, AOI::onSceneRemoveInterester);
		RGS_NODE_ARGS_HANDLER(_harbor, NodeProtocol::SCENE_MSG_REMOVE_WATCHER, AOI::onSceneRemoveWatcher);
	}

	if (_harbor->getNodeType() == NodeType::SCENE){

	}

	return true;
}

bool AOI::destory(sl::api::IKernel * pKernel){
	DEL this;
	return true;
}

void AOI::broadcast(IObject* object, int32 messageId, const sl::OBStream& args, bool self){
	std::unordered_map<int32, std::vector<int64>> sendWatchers;
	if (self && object->getPropInt32(attr_def::gate) != game::NODE_INVALID_ID){
		int32 gate = object->getPropInt32(attr_def::gate);
		sendWatchers[gate].push_back(object->getID());
	}

	ITableControl* watchers = object->findTable(OCTableMacro::AOI_WATCHERS::TABLE_NAME);
	SLASSERT(watchers, "wtf");
	for (int32 i = 0; i < watchers->rowCount(); i++){
		const IRow* row = watchers->getRow(i);
		int32 gate = row->getDataInt32(OCTableMacro::AOI_WATCHERS::GATE);
		if (gate != game::NODE_INVALID_ID)
			sendWatchers[gate].push_back(row->getDataInt64(OCTableMacro::AOI_WATCHERS::ID));
	}

	_packetSender->broadcast(sendWatchers, messageId, args);
}

void AOI::foreachNeighbor(IObject* object, const std::function<void(sl::api::IKernel* pKernel, IObject* object)>& f){
	ITableControl* interesters = object->findTable(OCTableMacro::AOI_INTERESTERS::TABLE_NAME);
	SLASSERT(interesters, "wtf");
	for (int32 i = 0; i < interesters->rowCount(); i++){
		const IRow* row = interesters->getRow(i);
		SLASSERT(row, "wtf");

		int64 id = row->getDataInt64(OCTableMacro::AOI_INTERESTERS::ID);
		IObject* other = _objectMgr->findObject(id);
		SLASSERT(other, "wtf");
		if (other)
			f(_kernel, other);
	}
}

bool AOI::isNeighbor(IObject* object, const int64 id){
	ITableControl* interesters = object->findTable(OCTableMacro::AOI_INTERESTERS::TABLE_NAME);
	SLASSERT(interesters, "wtf");

	return interesters->findRow(id) != nullptr;
}

void AOI::onSceneAddInterester(sl::api::IKernel* pKernel, int32 nodeType, int32 nodeId, const OArgs& args){
	int64 id = args.getInt64(0);
	int64 interesterId = args.getInt64(1);
	int32 type = args.getInt32(2);

	IObject* object = _objectMgr->findObject(id);
	SLASSERT(object, "wtf");

	if (object)
		addInterester(pKernel, object, interesterId, type);
}

void AOI::onSceneAddWatcher(sl::api::IKernel* pKernel, int32 nodeType, int32 nodeId, const OArgs& args){
	int64 id = args.getInt64(0);
	int64 watcherId = args.getInt64(1);
	int32 gate = args.getInt32(2);
	int32 logic = args.getInt32(3);

	IObject* object = _objectMgr->findObject(id);
	SLASSERT(object, "wtf");
	if (object)
		addWatcher(pKernel, object, watcherId, logic, gate);
}

void AOI::onSceneRemoveInterester(sl::api::IKernel* pKernel, int32 nodeType, int32 nodeId, const OArgs& args){
	int64 id = args.getInt64(0);
	int64 interesterId = args.getInt64(1);

	IObject* object = _objectMgr->findObject(id);
	SLASSERT(object, "wtf");

	if (object)
		removeInterester(pKernel, object, interesterId);
}

void AOI::onSceneRemoveWatcher(sl::api::IKernel* pKernel, int32 nodeType, int32 nodeId, const OArgs& args){
	int64 id = args.getInt64(0);
	int64 watcherId = args.getInt64(1);

	IObject* object = _objectMgr->findObject(id);
	SLASSERT(object, "wtf");
	
	if (object)
		removeWatcher(pKernel, object, watcherId);
}

void AOI::addWatcher(sl::api::IKernel* pKernel, IObject* object, int64 watcherId, int32 logic, int32 gate){
	ITableControl* watchers = object->findTable(OCTableMacro::AOI_WATCHERS::TABLE_NAME);
	SLASSERT(watchers, "wtf");
	if (watchers->findRow(watcherId))
		return;

	IRow* row = watchers->addRowKeyInt64(watcherId);
	row->setDataInt32(OCTableMacro::AOI_WATCHERS::LOGIC, logic);
	row->setDataInt32(OCTableMacro::AOI_WATCHERS::GATE, gate);
	
	_shadowMgr->createShadow(object, logic);

	notifyWatcherObjectAppear(pKernel, gate, watcherId, object);

	logic_event::VisionInfo vision;
	vision.object = object;
	vision.watcherId = watcherId;
	vision.gate = gate;
	_eventEngine->execEvent(logic_event::EVENT_LOGIC_ENTER_VISION, &vision, sizeof(vision));
}

void AOI::removeWatcher(sl::api::IKernel* pKernel, IObject* object, int64 watcherId){
	ITableControl* watchers = object->findTable(OCTableMacro::AOI_WATCHERS::TABLE_NAME);
	SLASSERT(watchers, "wtf");
	const IRow* row = watchers->findRow(watcherId);
	if (!row)
		return;

	int32 gate = row->getDataInt32(OCTableMacro::AOI_WATCHERS::GATE);
	int32 logic = row->getDataInt32(OCTableMacro::AOI_WATCHERS::LOGIC);

	_shadowMgr->removeShadow(object, logic);
		
	DEL_TABLE_ROW(watchers, row);

	notifyWatcherObjectDisappear(pKernel, gate, watcherId, object);

	logic_event::VisionInfo vision;
	vision.object = object;
	vision.watcherId = watcherId;
	vision.gate = gate;
	_eventEngine->execEvent(logic_event::EVENT_LOGIC_LEAVE_VISION, &vision, sizeof(vision));
}

void AOI::addInterester(sl::api::IKernel* pKernel, IObject* object, int64 interester, int32 type){
	ITableControl* interesters = object->findTable(OCTableMacro::AOI_INTERESTERS::TABLE_NAME);
	SLASSERT(interesters, "wtf");
	if (interesters->findRow(interester))
		return;

	const IRow* row = interesters->addRowKeyInt64(interester);
	row->setDataInt32(OCTableMacro::AOI_INTERESTERS::TYPE, type);

	logic_event::WatchInfo info;
	info.object = object;
	info.interester = interester;
	info.type = type;
	_eventEngine->execEvent(logic_event::EVENT_LOGIC_SEE_SOMEONE, &info, sizeof(info));
}

void AOI::removeInterester(sl::api::IKernel* pKernel, IObject* object, int64 interester){
	ITableControl* interesters = object->findTable(OCTableMacro::AOI_INTERESTERS::TABLE_NAME);
	SLASSERT(interesters, "wtf");
	const IRow* row = interesters->findRow(interester);
	if (!row)
		return;

	int32 type = row->getDataInt32(OCTableMacro::AOI_INTERESTERS::TYPE);

	DEL_TABLE_ROW(interesters, row);

	logic_event::WatchInfo info;
	info.object = object;
	info.interester = interester;
	info.type = type;
	_eventEngine->execEvent(logic_event::EVENT_LOGIC_MISS_SOMEONE, &info, sizeof(info));
}

void AOI::notifyWatcherObjectAppear(sl::api::IKernel* pKernel, int32 gate, int64 watcherId, IObject* object){
	sl::IBStream<game::MAX_PACKET_SIZE> args;
	args << object->getID() << object->getPropInt32(attr_def::type);

	int32 count = 0;
	for (auto prop : object->getObjProps()){
		if (prop->getSetting(object) & prop_def::share)
			count++;
	}
	args << count;

	for (auto prop : object->getObjProps()){
		int16 idxAndType = prop->getIndex(object);
		switch (prop->getType(object)){
		case DTYPE_INT8: idxAndType |= ((int16)protocol::AttribType::DTYPE_INT8) << 13; args << idxAndType << object->getPropInt8(prop); break;
		case DTYPE_INT16: idxAndType |= ((int16)protocol::AttribType::DTYPE_INT16) << 13; args << idxAndType << object->getPropInt16(prop); break;
		case DTYPE_INT32: idxAndType |= ((int16)protocol::AttribType::DTYPE_INT32) << 13; args << idxAndType << object->getPropInt32(prop); break;
		case DTYPE_INT64: idxAndType |= ((int16)protocol::AttribType::DTYPE_INT64) << 13; args << idxAndType << object->getPropInt64(prop); break;
		case DTYPE_STRING: idxAndType |= ((int16)protocol::AttribType::DTYPE_STRING) << 13; args << idxAndType << object->getPropString(prop); break;
		case DTYPE_FLOAT: idxAndType |= ((int16)protocol::AttribType::DTYPE_FLOAT) << 13; args << idxAndType << object->getPropFloat(prop); break;
		default: SLASSERT(false, "invaild type %d", prop->getType(object)); break;
		}
	}

	_packetSender->send(gate, watcherId, ServerMsgID::SERVER_MSG_NEW_ROLE_NOTIFY, args.out());
}

void AOI::notifyWatcherObjectDisappear(sl::api::IKernel* pKernel, int32 gate, int64 watcherId, IObject* object){
	sl::IBStream<64> args;
	args << object->getID();

	_packetSender->send(gate, watcherId, ServerMsgID::SERVER_MSG_REMOVE_ROLE_NOTIFY, args.out());
}