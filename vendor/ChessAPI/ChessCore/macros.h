#pragma once

#include <iostream>

//-----------------------------

#define OUT_STREAM         std::cout

#define Error(method)      method << " [ERROR] "

#define WARNING(method)    method << " [Warning] "

#define SUCCESS(method)    method << " [Success] "

#define PRESS(method)      method << " [Press] "

#define WRITE(method)      method << " [Write] "

#define Debug(method)      method << " [Debug] "


//-----Errors-------

#define MESSAGE_ERROR(message) std::cout << "(message from writer: " << message << " )" << std::endl

#define ERROR_FILE_NOPEN(filepath, message) Error(std::cout) << "(Error002) The file: '" << filepath << "' was not able to open. Your file is empty or it does not exist :(" << std::endl; \
MESSAGE_ERROR(message)

//-----------------------------

#ifdef CC_DEBUG

#define PROGRAM_ASSERT(boolean, message)    if(boolean) {Error(std::cout) << message; return;}

#define DEBUG_BREAK(boolean)				if(boolean) { __debugbreak(); }

#else

#define PROGRAM_ASSERT(boolean, message)

#define DEBUG_BREAK(boolean)

#endif // DEBUG