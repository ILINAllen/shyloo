#ifndef __SL_DB_CONNECTION_H__
#define __SL_DB_CONNECTION_H__
#include "sldb_define.h"

namespace sl
{
namespace db
{
class SLDBConnection : public ISLDBConnection{
public:
	SLDBConnection(ISLDBConnectionPool* pConnPool);
	~SLDBConnection();
	virtual bool SLAPI open(const char* szHostName, const int32 port, const char* szName, const char* szPwd, const char* szDBName, const char* szCharSet);
	virtual bool SLAPI reOpen();
	virtual ISLDBResult* SLAPI execute(const char* commandSql);
	virtual int32 SLAPI getLastErrno(void);
	virtual const char* SLAPI getLastError(void);
	virtual void SLAPI release(void);

	bool connectDBSvr();
	bool isActive();

private:
	DBConnectionInfo		m_connInfo;
	MYSQL					m_mysqlHandler;
	ISLDBConnectionPool*	m_pConnPool;
};

}
}

#endif