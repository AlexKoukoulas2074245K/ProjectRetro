#pragma once

#define GL_SILENCE_DEPRECATION
#ifdef _WIN32

//---------------------------------------------------------------------------
//  On Win32, use our own GL headers and prepare a function table
//---------------------------------------------------------------------------
#include "khrplatform.h"
#include "gl2platform.h"
#include "gl2.h"
#include "gl2ext.h"
#include <assert.h>

struct GLFuncTable {
#define GL_FUNC(retVal, name, args) retVal (GL_APIENTRY *name)args;
#include "Funcs.h"
#undef GL_FUNC
    void initialize();
};
extern GLFuncTable glFuncTable;
#define GL_CHECK(call) do { glFuncTable.call; assert(glFuncTable.glGetError() == GL_NO_ERROR); } while (0)
#define GL_NO_CHECK(call) (glFuncTable.call)

#else // TURF_TARGET_WIN32

//---------------------------------------------------------------------------
//  Otherwise, use the system's GL headers
//---------------------------------------------------------------------------
#ifdef __APPLE__
    #include <OpenGL/gl3.h>
#else
    #include <GLES2/gl3.h>
#endif
#define GL_CHECK(call) do { call; assert(glGetError() == GL_NO_ERROR); } while (0)
#define GL_NO_CHECK(call) (call)

#endif // TURF_TARGET_WIN32
