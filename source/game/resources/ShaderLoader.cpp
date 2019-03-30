//
//  ShaderLoader.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 29/03/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "ShaderLoader.h"
#include "../resources/ShaderResource.h"
#include "../rendering/opengl/Context.h"
#include "../common_utils/MessageBox.h"
#include "../common_utils/Logging.h"
#include "../common_utils/StringUtils.h"

#include <fstream>   // ifstream
#include <streambuf> // istreambuf_iterator

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::string ShaderLoader::VERTEX_SHADER_FILE_EXTENSION = ".vs";
const std::string ShaderLoader::FRAGMENT_SHADER_FILE_EXTENSION = ".fs";

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void ShaderLoader::VInitialize()
{
}

std::unique_ptr<IResource> ShaderLoader::VCreateAndLoadResource(const std::string& resourcePathWithExtension) const
{
        // Since the shader loading is signalled by the .vs or .fs extension, we need to trim it here after
    // being added by the ResourceLoadingService prior to this call
    const auto resourcePath = resourcePathWithExtension.substr(0, resourcePathWithExtension.size() - 3);

    // Generate vertex shader id
    const auto vertexShaderId = GL_NO_CHECK(glCreateShader(GL_VERTEX_SHADER));
    
    // Read vertex shader source
    const auto vertexShaderFileContents = ReadFileContents(resourcePath + VERTEX_SHADER_FILE_EXTENSION);
    const char* vertexShaderFileContentsPtr = vertexShaderFileContents.c_str();

    // Compile vertex shader
    GL_CHECK(glShaderSource(vertexShaderId, 1, &vertexShaderFileContentsPtr, nullptr));
    GL_CHECK(glCompileShader(vertexShaderId));
    
    // Check vertex shader compilation
    std::string vertexShaderInfoLog;
    GLint vertexShaderInfoLogLength;
    GL_CHECK(glGetShaderiv(vertexShaderId, GL_INFO_LOG_LENGTH, &vertexShaderInfoLogLength));
    if (vertexShaderInfoLogLength > 0)
    {
        vertexShaderInfoLog.clear();
        vertexShaderInfoLog.reserve(vertexShaderInfoLogLength);
        GL_CHECK(glGetShaderInfoLog(vertexShaderId, vertexShaderInfoLogLength, nullptr, &vertexShaderInfoLog[0]));
        Log(LogType::INFO, "While compiling vertex shader:\n%s", vertexShaderInfoLog.c_str());
    }
    
    // Generate fragment shader id
    const auto fragmentShaderId = GL_NO_CHECK(glCreateShader(GL_FRAGMENT_SHADER));
    
    // Read vertex shader source
    const auto fragmentShaderFileContents = ReadFileContents(resourcePath + FRAGMENT_SHADER_FILE_EXTENSION);
    const char* fragmentShaderFileContentsPtr = fragmentShaderFileContents.c_str();
    
    GL_CHECK(glShaderSource(fragmentShaderId, 1, &fragmentShaderFileContentsPtr, nullptr));
    GL_CHECK(glCompileShader(fragmentShaderId));
    
    std::string fragmentShaderInfoLog;
    GLint fragmentShaderInfoLogLength;
    GL_CHECK(glGetShaderiv(fragmentShaderId, GL_INFO_LOG_LENGTH, &fragmentShaderInfoLogLength));
    if (fragmentShaderInfoLogLength > 0)
    {
        fragmentShaderInfoLog.clear();
        fragmentShaderInfoLog.reserve(fragmentShaderInfoLogLength);
        GL_CHECK(glGetShaderInfoLog(fragmentShaderId, fragmentShaderInfoLogLength, nullptr, &fragmentShaderInfoLog[0]));
        Log(LogType::INFO, "While compiling fragment shader:\n%s", fragmentShaderInfoLog.c_str());
    }

    // Link shader program
    const auto programId = GL_NO_CHECK(glCreateProgram());
    GL_CHECK(glAttachShader(programId, vertexShaderId));
    GL_CHECK(glAttachShader(programId, fragmentShaderId));
    GL_CHECK(glLinkProgram(programId));
    
#ifndef _WIN32
    std::string linkingInfoLog;
    GLint linkingInfoLogLength;
    
    glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &linkingInfoLogLength);
    if (linkingInfoLogLength > 0)
    {
        linkingInfoLog.clear();
        linkingInfoLog.reserve(linkingInfoLogLength);
        GL_CHECK(glGetProgramInfoLog(programId, linkingInfoLogLength, NULL, &linkingInfoLog[0]));
        Log(LogType::INFO, "While linking shader:\n%s", linkingInfoLog.c_str());
    }
#endif
    
    GL_CHECK(glValidateProgram(programId));
    
#ifndef _WIN32
    GLint status;
    std::string validateInfoLog;
    GLint validateInfoLogLength;
    
    GL_CHECK(glGetProgramiv(programId, GL_VALIDATE_STATUS, &status));
    glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &validateInfoLogLength);
    if (validateInfoLogLength > 0)
    {
        validateInfoLog.clear();
        validateInfoLog.reserve(validateInfoLogLength);
        GL_CHECK(glGetProgramInfoLog(programId, validateInfoLogLength, NULL, &validateInfoLog[0]));
        Log(LogType::INFO, "While validating shader:\n%s", validateInfoLog.c_str());
    }
#endif
    
    // Destroy intermediate compiled shaders
    GL_CHECK(glDetachShader(programId, vertexShaderId));
    GL_CHECK(glDetachShader(programId, fragmentShaderId));
    GL_CHECK(glDeleteShader(vertexShaderId));
    GL_CHECK(glDeleteShader(fragmentShaderId));
    
    const auto uniformNamesToLocations = GetUniformNamesToLocationsMap(programId, vertexShaderFileContents, fragmentShaderFileContents);
    return std::make_unique<ShaderResource>(programId, uniformNamesToLocations);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

std::string ShaderLoader::ReadFileContents(const std::string& filePath) const
{
    std::ifstream file(filePath);
    
    if (!file.good())
    {
        ShowMessageBox(MessageBoxType::ERROR, "File could not be found", filePath.c_str());
        return nullptr;
    }
    
    std::string contents;
    
    file.seekg(0, std::ios::end);
    contents.reserve(static_cast<size_t>(file.tellg()));
    file.seekg(0, std::ios::beg);
    
    contents.assign((std::istreambuf_iterator<char>(file)),
               std::istreambuf_iterator<char>());
    
    return contents;
}

std::unordered_map<StringId, GLuint, StringIdHasher> 
ShaderLoader::GetUniformNamesToLocationsMap(const GLuint programId, const std::string& vertexShaderFileContents, const std::string& fragmentShaderFileContents) const
{
    std::unordered_map<StringId, GLuint, StringIdHasher> uniformNamesToLocationsMap;
    
    const auto vertexShaderContentSplitByNewline = StringSplit(vertexShaderFileContents, '\n');
    for (const auto& vertexShaderLine: vertexShaderContentSplitByNewline)
    {
        if (StringStartsWith(vertexShaderLine, "uniform"))
        {
            const auto uniformLineSplitBySpace = StringSplit(vertexShaderLine, ' ');
            
            // Uniform names will always be the third components in the line
            // e.g. uniform bool foo. The semicolumn at the end also needs to be trimmed
            const auto uniformName = uniformLineSplitBySpace[2].substr(0, uniformLineSplitBySpace[2].size() - 1);
            const auto uniformLocation = GL_NO_CHECK(glGetUniformLocation(programId, uniformName.c_str()));
            
            uniformNamesToLocationsMap[StringId(uniformName)] = uniformLocation;
        }
    }
    
    const auto fragmentShaderContentSplitByNewline = StringSplit(fragmentShaderFileContents, '\n');
    for (const auto& fragmentShaderLine: fragmentShaderContentSplitByNewline)
    {
        if (StringStartsWith(fragmentShaderLine, "uniform"))
        {
            const auto uniformLineSplitBySpace = StringSplit(fragmentShaderLine, ' ');
            
            // Uniform names will always be the third components in the line
            // e.g. uniform bool foo
            const auto uniformName = uniformLineSplitBySpace[2];
            const auto uniformLocation = GL_NO_CHECK(glGetUniformLocation(programId, uniformName.c_str()));
            
            uniformNamesToLocationsMap[StringId(uniformName)] = uniformLocation;
        }
    }
    
    return uniformNamesToLocationsMap;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
