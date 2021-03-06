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

ShaderResource::ShaderResource
(
    const std::unordered_map<StringId, GLuint, StringIdHasher> uniformNamesToLocations,
    const GLuint programId
)
    : mShaderUniformNamesToLocations(uniformNamesToLocations) 
    , mProgramId(programId)
{
    
}

ShaderResource& ShaderResource::operator = (const ShaderResource& rhs)
{
    CopyConstruction(rhs);
    return *this;
}

ShaderResource::ShaderResource(const ShaderResource& rhs)
{
    CopyConstruction(rhs);
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

void ShaderResource::CopyConstruction(const ShaderResource& rhs)
{
    mProgramId = rhs.GetProgramId();
    for (const auto& uniformEntry: rhs.GetUniformNamesToLocations())
    {
        mShaderUniformNamesToLocations[uniformEntry.first] = uniformEntry.second;
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
