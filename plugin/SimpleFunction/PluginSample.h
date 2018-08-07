/*
It's a sample of plugin for Kagami script processor.
It will tell you how to make your own plugin and export to script.
*/
#pragma once
#include "header.h"

using kagami::vector;
using kagami::ActivityTemplate;
using kagami::Message;
using kagami::ObjTemplate;
using kagami::Kit;
using kagami::ObjectMap;

#ifdef __cplusplus
extern "C" {
#endif
  __declspec(dllexport) vector<ActivityTemplate> *Attachment(void);
  __declspec(dllexport) int FreeMemory(void *ptr, int type);

#ifdef __cplusplus
}
#endif