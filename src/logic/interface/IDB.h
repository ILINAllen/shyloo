#ifndef _SL_IDBMGR_H__
#define _SL_IDBMGR_H__
#include "slimodule.h"
#include <functional>
class IDBCallCondition{
public:
	enum DBConditionOpType{
		DBOP_EQ = 0, //  =
		DBOP_NE,	 // !=
		DBOP_GT,	 // >
		DBOP_GE,	 // >=
		DBOP_LS,	 // <
		DBOP_LE,	 // <=
	};

	virtual void AddCondition(const char* key, const DBConditionOpType opt, const int8 value) = 0;
	virtual void AddCondition(const char* key, const DBConditionOpType opt, const int16 value) = 0;
	virtual void AddCondition(const char* key, const DBConditionOpType opt, const int32 value) = 0;
	virtual void AddCondition(const char* key, const DBConditionOpType opt, const int64 value) = 0;
	virtual void AddCondition(const char* key, const DBConditionOpType opt, const char* value) = 0;
};

class IDBQueryParamAdder{

};

class IDBInsertParamAdder{

};

class IDBUpdateParamAdder{

};

typedef std::function<void(sl::api::IKernel* pKernel, IDBQueryParamAdder* adder, IDBCallCondition* condition)> DBQueryCommandFunc;
typedef std::function<void(sl::api::IKernel* pKernel, IDBInsertParamAdder* adder)> DBInsertCommandFunc;
typedef std::function<void(sl::api::IKernel* pKernel, IDBUpdateParamAdder* adder, IDBCallCondition* condition)> DBUpdateCommandFunc;
typedef std::function<void(sl::api::IKernel* pKernel, IDBCallCondition* condition)> DBDeleteCommandFunc;

class IDBCallSource{
public:
	virtual ~IDBCallSource() {}

	virtual const void* getContext(const int32 size = 0) = 0;
};

class IDBResult{
public:
	virtual ~IDBResult() {}

	virtual int32 rowCount() = 0;
	virtual int8 getDataInt8(const int32 i, const char* key) = 0;
	virtual int16 getDataInt16(const int32 i, const char* key) = 0;
	virtual int32 getDataInt32(const int32 i, const char* key) = 0;
	virtual int64 getDataInt64(const int32 i, const char* key) = 0;
	virtual const char* getDataString(const int32 i, const char* key) = 0;
};

typedef std::function<void(sl::api::IKernel* pKernel, const int64 id, const bool success, const int32 affectedRow, const IDBCallSource* source, const IDBResult* result)> DBCallBack;

class IDBCall{
public:
	virtual ~IDBCall() {}

	virtual void query(const char* tableName, const DBQueryCommandFunc& f, const DBCallBack& cb) = 0;
	virtual void insert(const char* tableName, const DBInsertCommandFunc& f, const DBCallBack& cb) = 0;
	virtual void update(const char* tableName, const DBUpdateCommandFunc& f, const DBCallBack& cb) = 0;
	virtual void del(const char* tableName, const DBDeleteCommandFunc& f, const DBCallBack& cb) = 0;
};

class IDB : public sl::api::IModule{
public:
	virtual ~IDB() {}

	virtual IDBCall* create(int64 threadId, const int64 id, const char* file, const int32 line, const void* context, const int32 size = 0) = 0;
};

#define CREATE_DB_CALL(_db, _task, _params, _cb) _db->execDBTask(_task, _params, std::bind(&_cb, this, std::placeholders::_1,std::placeholders::_2))
#define CREATE_DB_CALL_CONTEXT(_db, _task, _params) _db->execDBTask(_task, _params)
#endif