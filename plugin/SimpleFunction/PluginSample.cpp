// PluginSample.cpp: 定义 DLL 应用程序的导出函数。
//

#include "PluginSample.h"

Message FunctionInPlugin(ObjectMap &p) {
  using namespace kagami;
  Message result(kStrRedirect, kCodeSuccess, "Success.");
  return result;
}

vector<ActivityTemplate>* Attachment(void) {
  using namespace kagami;
  using T = ActivityTemplate;
  auto ptr = new vector<T>;
  ptr->emplace_back(T("FunctionInPlugin", FunctionInPlugin, kFlagNormalEntry, kCodeNormalParm, ""));
  return ptr;
}

int FreeMemory(void *ptr, int type) {
  using namespace kagami;
  using ObjTemp = map<string, ObjTemplate> *;
  using ActTemp = vector<ActivityTemplate> *;
  if (type == kPointerActTemp) {
    ActTemp origin = static_cast<ActTemp>(ptr);
    delete(origin);
  }
  if (type == kPointerObjTemp) {
    ObjTemp origin = static_cast<ObjTemp>(ptr);
    delete(origin);
  }
  return type;
}



