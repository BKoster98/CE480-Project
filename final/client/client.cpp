//Authors: Allison Hurley & Ben Kocik 
//Purpose: Final client dice roller Cpp program

#include <iostream>    
#include <thread> 
#include <mutex>
// Include our portable socket since it is written in C
#ifdef __cplusplus
extern "C"
{
#endif
#include "../portable_socket.h"
#ifdef __cplusplus
}
#endif