//
//  ShaderResource.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 29/03/2019.
//


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "ShaderResource.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

ShaderResource::ShaderResource(const GLuint programId, const std::unordered_map<StringId, GLuint, StringIdHasher> uniformNamesToLocations)
    : mProgramId(programId)
    , mShaderUniformNamesToLocations(uniformNamesToLocations)
{
    
}

GLuint ShaderResource::GetProgramId() const
{
    return mProgramId;
}

const std::unordered_map<StringId, GLuint, StringIdHasher>& ShaderResource::GetUniformNamesToLocations() const
{
    return mShaderUniformNamesToLocations;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////