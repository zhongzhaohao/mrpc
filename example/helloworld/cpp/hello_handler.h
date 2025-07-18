#ifndef HELLO_HANDLER_H
#define HELLO_HANDLER_H

#include "helloworld.mrpc.h"

// SayHello的处理函数声明
void HandleSayHello(const char* key, const char* request_str, 
                   const char** response, mrpc_status* status);

#endif // HELLO_HANDLER_H 