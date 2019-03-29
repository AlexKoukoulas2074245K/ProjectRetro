//
//  ShaderResource.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 29/03/2019.
//


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef ShaderResource_h
#define ShaderResource_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "IResource.h"
#include "../common_utils/StringId.h"

#include <string>
#include <unordered_map>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

using GLuint = unsigned int;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class ShaderResource final: public IResource
{
public:
    ShaderResource() = default;
    ShaderResource(const GLuint programId, const std::unordered_map<StringId, GLuint, StringIdHasher> uniformNamesToLocations);
    ShaderResource& operator = (const ShaderResource&);
    ShaderResource(const ShaderResource&);
    
    GLuint GetProgramId() const;
    const std::unordered_map<StringId, GLuint, StringIdHasher>& GetUniformNamesToLocations() const;
    
private:
    void CopyConstruction(const ShaderResource&);
    
private:
    GLuint mProgramId;
    std::unordered_map<StringId, GLuint, StringIdHasher> mShaderUniformNamesToLocations;

};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* ShaderResource_h */