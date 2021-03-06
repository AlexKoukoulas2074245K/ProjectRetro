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

const glm::vec3& MeshResource::GetDimensions() const
{
    return mDimensions;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

MeshResource::MeshResource(const GLuint vertexArrayObject, const GLuint elementCount, const glm::vec3& meshDimensions)
    : mVertexArrayObject(vertexArrayObject)
    , mElementCount(elementCount)
    , mDimensions(meshDimensions)
{
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
