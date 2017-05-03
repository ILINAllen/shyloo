#ifndef __SL_FRAMEWORK_AI_MACHINE_H__
#define __SL_FRAMEWORK_AI_MACHINE_H__
#include "AINode.h"
class IObject;
class AIMachine{
public:
	AIMachine(AINode* root) :_root(root){}
	~AIMachine(){}

	void run(sl::api::IKernel* pKernel, IObject* object);

private:
	AINode*		_root;
};
#endif