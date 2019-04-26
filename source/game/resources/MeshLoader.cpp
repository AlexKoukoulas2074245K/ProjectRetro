//
//  MeshLoader.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 30/03/2019,
//  Adapted from https://github.com/opengl-tutorials/ogl/blob/master/common/objloader.cpp
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

// Disable CRT_SECURE warnings for fopen, fscanf etc..
#ifdef _WIN32
#pragma warning(disable: 4996)
#endif

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "MeshLoader.h"
#include "MeshResource.h"
#include "../common/utils/StringUtils.h"
#include "../common/utils/MathUtils.h"
#include "../rendering/opengl/Context.h"

#include <cassert>
#include <cstdio>
#include <vector>


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void MeshLoader::VInitialize()
{
}

std::unique_ptr<IResource> MeshLoader::VCreateAndLoadResource(const std::string& path) const
{
    auto trimmedPath = path;
    const auto injectedTexCoordsString = ExtractAndRemoveInjectedTexCoordsIfAny(trimmedPath);
    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;
    
    std::vector<glm::vec3> final_vertices;
    std::vector<glm::vec2> final_uvs;
    std::vector<glm::vec3> final_normals;
    
    std::vector<unsigned short> final_indices;
    
    float minX = 100.0f, maxX = -100.0f, minY = 100.0f, maxY = -100.0f, minZ = 100.0f, maxZ = -100.0f;

    FILE * file = std::fopen(trimmedPath.c_str(), "r");
    assert(file != nullptr && "Model file not found");
    
    while(1)
    {
        char lineHeader[128];
        // read the first word of the line
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break; // EOF = End Of File. Quit the loop.
        
        if (strcmp(lineHeader, "v") == 0)
        {
            glm::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
            temp_vertices.push_back(vertex);

            if (vertex.x < minX) minX = vertex.x;
            if (vertex.x > maxX) maxX = vertex.x;
            if (vertex.y < minY) minY = vertex.y;
            if (vertex.y > maxY) maxY = vertex.y;
            if (vertex.z < minZ) minZ = vertex.z;
            if (vertex.z > maxZ) maxZ = vertex.z;
        }
        else if (strcmp(lineHeader, "vt") == 0)
        {
            glm::vec2 uv;
            fscanf(file, "%f %f\n", &uv.x, &uv.y );            
            temp_uvs.push_back(uv);
        }
        else if (strcmp(lineHeader, "vn") == 0)
        {
            glm::vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
            temp_normals.push_back(normal);
        }
        else if (strcmp(lineHeader, "f") == 0)
        {
            std::string vertex1, vertex2, vertex3;
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
            if (matches != 9)
            {
                assert(false && "File can't be read by this simple parser");
                fclose(file);
            }
            
            vertexIndices.push_back(vertexIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);
            
            uvIndices.push_back(uvIndex[0]);
            uvIndices.push_back(uvIndex[1]);
            uvIndices.push_back(uvIndex[2]);
            
            normalIndices.push_back(normalIndex[0]);
            normalIndices.push_back(normalIndex[1]);
            normalIndices.push_back(normalIndex[2]);
        }
        else
        {
            // Probably a comment, eat up the rest of the line
            char stupidBuffer[1000];
            fgets(stupidBuffer, 1000, file);
        }
    }
    
    if (injectedTexCoordsString.length() > 0)
    {
        temp_uvs.clear();
        const auto injectedCoordPairs = StringSplit(injectedTexCoordsString, '-');
        for (const auto injectedCoordPairString: injectedCoordPairs)
        {
            const auto injectedCoordPairSplitByComma = StringSplit(injectedCoordPairString, ',');
            temp_uvs.push_back(glm::vec2(std::stof(injectedCoordPairSplitByComma[0]), std::stof(injectedCoordPairSplitByComma[1])));
        }
    }
    
    // For each vertex of each triangle
    for(unsigned int i=0; i<vertexIndices.size(); i++)
    {
        // Get the indices of its attributes
        unsigned int vertexIndex = vertexIndices[i];
        unsigned int uvIndex = uvIndices[i];
        unsigned int normalIndex = normalIndices[i];
        
        // Get the attributes thanks to the index
        glm::vec3 vertex = temp_vertices[ vertexIndex-1 ];
        glm::vec2 uv = temp_uvs[ uvIndex-1 ];
        glm::vec3 normal = temp_normals[ normalIndex-1 ];
        
        // Put the attributes in buffers
        final_vertices.push_back(vertex);
        final_uvs.push_back(uv);
        final_normals.push_back(normal);
        final_indices.push_back(static_cast<unsigned short>(i));
    }
    
    std::fclose(file);
    
    GLuint vertexArrayObject;
    GLuint vertexBufferObject;
    GLuint uvCoordsBufferObject;
    GLuint indexBufferObject;
    
    // Create Buffers
    GL_CHECK(glGenVertexArrays(1, &vertexArrayObject));
    GL_CHECK(glGenBuffers(1, &vertexBufferObject));
    GL_CHECK(glGenBuffers(1, &uvCoordsBufferObject));
    GL_CHECK(glGenBuffers(1, &indexBufferObject));
    
    // Prepare VAO to record buffer state
    GL_CHECK(glBindVertexArray(vertexArrayObject));
    
    // Bind and Buffer VBO
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, final_vertices.size() * sizeof(glm::vec3), &final_vertices[0], GL_STATIC_DRAW));
    
    // 1st attribute buffer : vertices
    GL_CHECK(glEnableVertexAttribArray(0));
    GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0));
    
    // Bind and buffer TBO
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, uvCoordsBufferObject));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, final_uvs.size() * sizeof(glm::vec2), &final_uvs[0], GL_STATIC_DRAW));
    
    // 2nd attribute buffer: tex coords
    GL_CHECK(glEnableVertexAttribArray(1));
    GL_CHECK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0));
    
    // Bind and Buffer IBO
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject));
    GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, final_indices.size() * sizeof(unsigned short), &final_indices[0], GL_STATIC_DRAW));
    
    GL_CHECK(glBindVertexArray(0));
    
    // Calculate dimensinos
    glm::vec3 meshDimensions(math::Abs(minX - maxX), math::Abs(minY - maxY), math::Abs(minZ - maxZ));
    return std::unique_ptr<MeshResource>(new MeshResource(vertexArrayObject, final_indices.size(), meshDimensions));
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

std::string MeshLoader::ExtractAndRemoveInjectedTexCoordsIfAny(std::string& path) const
{
    std::string injectedTexCoordString;
    std::string cleanPath;
    
    auto recordingInjectedTexCoords = false;
    
    for (auto i = 0U; i < path.length(); ++i)
    {
        const auto c = path[i];
        if (c == '[' || c == ']')
        {
            recordingInjectedTexCoords = !recordingInjectedTexCoords;
        }
        else if (recordingInjectedTexCoords)
        {
            injectedTexCoordString += c;
        }
        else
        {
            cleanPath += c;
        }
    }
    
    path = cleanPath;
    return injectedTexCoordString;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
