//
//  MeshResource.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 30/03/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "MeshResource.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

GLuint MeshResource::GetVertexArrayObject() const
{
    return mVertexArrayObject;
}

GLuint MeshResource::GetElementCount() const
{
    return mElementCount;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

MeshResource::MeshResource(const GLuint vertexArrayObject, const GLuint elementCount)
    : mVertexArrayObject(vertexArrayObject)
    , mElementCount(elementCount)
{
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////