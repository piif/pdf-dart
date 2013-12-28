/**
 * some includes and defines to ease definition of native API
 */

#ifndef dartNativeHelpers_h
#define dartNativeHelpers_h

#include <string.h>
#include "include/dart_api.h"
#include "include/dart_native_api.h"

#define INIT_LIB(name)												\
DART_EXPORT Dart_Handle name##_Init(Dart_Handle parent_library) {	\
  if (Dart_IsError(parent_library)) { return parent_library; }		\
  Dart_Handle result_code =											\
		 Dart_SetNativeResolver(parent_library, ResolveName);		\
  if (Dart_IsError(result_code)) return result_code;				\
  return Dart_Null();												\
}

#define BEGIN_ASYNC_FUNCTION(name) 								\
	void name##Handler(Dart_Port reply_port_id,					\
		Dart_CObject* message);									\
	void name(Dart_NativeArguments arguments) {					\
	  Dart_SetReturnValue(arguments, Dart_Null());				\
	  Dart_Port service_port =									\
		  Dart_NewNativePort(#name, name##Handler, true);		\
	  if (service_port != ILLEGAL_PORT) {						\
		Dart_Handle send_port = Dart_NewSendPort(service_port);	\
		Dart_SetReturnValue(arguments, send_port);				\
	  }															\
	}															\
	void name##Handler(Dart_Port send_port_id,					\
			Dart_CObject* message) {							\
		Dart_Port reply_port_id = message->value.as_array		\
			.values[0]->value.as_send_port;						\
		Dart_CObject result;									\
		int argc = message->value.as_array.length - 1;			\
		Dart_CObject** argv = message->value.as_array.values + 1;


#define END_ASYNC_FUNCTION(name) 					\
	endAsyncFunc:									\
		Dart_PostCObject(reply_port_id, &result);	\
	}

#define RETURN_ASYNC_FUNCTION goto endAsyncFunc

/***
#define THROW(message) \
	Dart_PropagateError(Dart_NewApiError(message))

#define BEGIN_THROW { Dart_Handle h = Dart_NewApiError
#define END_THROW   Dart_PropagateError(h); }
***/

#define BEGIN_NAMES \
resolveNameEntryHelper resolveNameEntries[] = {

#define ADD_NAME(n) \
	{ name: (char *)#n, func: n },

#define END_NAMES 				\
	{ name: NULL, func: NULL }	\
};

typedef struct _resolveNameEntryHelper {
	char *name;
	Dart_NativeFunction func;
} resolveNameEntryHelper;
extern resolveNameEntryHelper resolveNameEntries[];

Dart_Handle inner_error_handle;

Dart_NativeFunction ResolveName(Dart_Handle name, int argc) {
  if (!Dart_IsString(name)) return NULL;
  Dart_NativeFunction result = NULL;
  Dart_EnterScope();
  const char* cname;

  inner_error_handle = Dart_StringToCString(name, &cname);
  if (Dart_IsError(inner_error_handle)) {
	  Dart_PropagateError(inner_error_handle); // force exits function
  }

  int i = 0;
  while (resolveNameEntries[i].name) {
	  if (strcmp(resolveNameEntries[i].name, cname) == 0) {
		  result = resolveNameEntries[i].func;
	  }
	  i++;
  }

  Dart_ExitScope();
  return result;
}

#endif

