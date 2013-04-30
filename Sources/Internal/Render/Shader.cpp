/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Vitaliy Borodovsky 
=====================================================================================*/
#include "Render/Shader.h"
#include "Render/RenderBase.h"
#include "Platform/Thread.h"
#include "Render/RenderManager.h"
#include "FileSystem/YamlParser.h"
#include "FileSystem/FileSystem.h"
#include "Platform/SystemTimer.h"
#include "Utils/Utils.h"
#include "Utils/StringFormat.h"

namespace DAVA 
{
#if defined(__DAVAENGINE_OPENGL__)
GLuint Shader::activeProgram = 0;

Shader::Shader()
    : RenderResource()
{
    DVASSERT(RenderManager::Instance()->GetRenderer() == Core::RENDERER_OPENGL_ES_2_0 || RenderManager::Instance()->GetRenderer() == Core::RENDERER_OPENGL);
    
    vertexShader = 0;
    fragmentShader = 0;
    program = 0;
    
    attributeNames = 0;
    activeAttributes = 0;
    activeUniforms = 0;

    uniforms = 0;
    
    for (int32 ki = 0; ki < VERTEX_FORMAT_STREAM_MAX_COUNT; ++ki)
         vertexFormatAttribIndeces[ki] = -1;

    vertexShaderData = 0;
    fragmentShaderData = 0;
    
//#if defined(__DAVAENGINE_ANDROID__) || defined (__DAVAENGINE_MACOS__)
//    relativeFileName = "";
//#endif //#if defined(__DAVAENGINE_ANDROID__) 
}

String VertexTypeStringFromEnum(GLenum type); // Fucking XCode 4 analyzer
String VertexTypeStringFromEnum(GLenum type)
{
    if (type == GL_FLOAT)return "GL_FLOAT";
    else if (type == GL_FLOAT_VEC2)return "GL_FLOAT_VEC2";
    else if (type == GL_FLOAT_VEC3)return "GL_FLOAT_VEC3";
    else if (type == GL_FLOAT_VEC4)return "GL_FLOAT_VEC4";
    else if (type == GL_FLOAT_MAT2)return "GL_FLOAT_MAT2";
    else if (type == GL_FLOAT_MAT3)return "GL_FLOAT_MAT3";
    else if (type == GL_FLOAT_MAT4)return "GL_FLOAT_MAT4";
    return "";
}
    
FastName uniformStrings[Shader::UNIFORM_COUNT] =
    {
        "none",
        "modelViewProjectionMatrix",
        "modelViewMatrix",
		"projectionMatrix",
        "normalMatrix",
        "flatColor",
        "globalTime",
    };

FastName attributeStrings[VERTEX_FORMAT_STREAM_MAX_COUNT] =
    {
        "inPosition",
        "inNormal",
        "inColor",
        "inTexCoord0",
        "inTexCoord1",
        "inTexCoord2",
        "inTexCoord3",
        "inTangent",
        "inBinormal",
        "inJointWeight"
    };
    
Shader::eUniform Shader::GetUniformByName(const FastName & name)
{
    for (int32 k = 0; k < UNIFORM_COUNT; ++k)
        if (name == uniformStrings[k])return (Shader::eUniform)k;
    return Shader::UNIFORM_NONE;
};
    
int32 Shader::GetUniformCount()
{
    return activeUniforms;
}
    
Shader::eUniformType Shader::GetUniformType(int32 index)
{
    return uniforms[index].type;
}
    
int32 Shader::GetUniformTypeSize(eUniformType type)
{
    switch(type)
    {
        case UT_FLOAT:
            return 4;
        case UT_FLOAT_VEC2:
            return 4 * 2;
        case UT_FLOAT_VEC3:
            return 4 * 3;
        case UT_FLOAT_VEC4:
            return 4 * 4;
        case UT_INT:
            return 4 * 1;
        case UT_INT_VEC2:
            return 4 * 2;
        case UT_INT_VEC3:
            return 4 * 3;
        case UT_INT_VEC4:
            return 4 * 4;
        case UT_BOOL:
            return 4 * 1;
        case UT_BOOL_VEC2:
            return 4 * 2;
        case UT_BOOL_VEC3:
            return 4 * 3;
        case UT_BOOL_VEC4:
            return 4 * 4;
        case UT_FLOAT_MAT2:
            return 4 * 2 * 2;
        case UT_FLOAT_MAT3:
            return 4 * 3 * 3;
        case UT_FLOAT_MAT4:
            return 4 * 4 * 4;
        case UT_SAMPLER_2D:
            return 4;
        case UT_SAMPLER_CUBE:
            return 4;

		default:
			break;
    };

	return 0;
}
    
    
const char * Shader::GetUniformTypeSLName(eUniformType type)
{
    switch(type)
    {
        case UT_FLOAT:
            return "float";
        case UT_FLOAT_VEC2:
            return "vec2";
        case UT_FLOAT_VEC3:
            return "vec3";
        case UT_FLOAT_VEC4:
            return "vec4";
        case UT_INT:
            return "int";
        case UT_INT_VEC2:
            return "ivec2";
        case UT_INT_VEC3:
            return "ivec3";
        case UT_INT_VEC4:
            return "ivec4";
        case UT_BOOL:
            return "bool";
        case UT_BOOL_VEC2:
            return "bvec2";
        case UT_BOOL_VEC3:
            return "bvec3";
        case UT_BOOL_VEC4:
            return "bvec4";
        case UT_FLOAT_MAT2:
            return "mat2";
        case UT_FLOAT_MAT3:
            return "mat3";
        case UT_FLOAT_MAT4:
            return "mat4";
        case UT_SAMPLER_2D:
            return "sampler2D";
        case UT_SAMPLER_CUBE:
            return "samplerCube";

		default:
			break;
    };

	return "";
}

    
    
const char * Shader::GetUniformName(int32 index)
{
    return uniforms[index].name.c_str();
}

int32 Shader::GetUniformLocation(int32 index)
{
    return uniforms[index].location;
}
    
int32 Shader::GetUniformArraySize(int32 index)
{
    return uniforms[index].size;
}

int32 Shader::FindUniformLocationByName(const FastName & name)
{
    for (int32 k = 0; k < activeUniforms; ++k)
    {
        if (uniforms[k].name == name)
        {
            return uniforms[k].location;
        }
    }
    return -1;
}

    
int32 Shader::GetAttributeIndexByName(const FastName & name)
{
    for (int32 k = 0; k < VERTEX_FORMAT_STREAM_MAX_COUNT; ++k)
        if (name == attributeStrings[k]) return k;
    return -1;
}
    
void Shader::SetDefines(const String & _defines)
{
    vertexShaderDefines = fragmentShaderDefines = _defines;
}

void Shader::SetVertexShaderDefines(const String & _defines)
{
    vertexShaderDefines = _defines;
}
    
void Shader::SetFragmentShaderDefines(const String & _defines)
{
    fragmentShaderDefines = _defines;
}
    
void Shader::SetDefineList(const String & enableDefinesList)
{
    Vector<String> defineNameList;
    String result;
    Split(enableDefinesList, ";", defineNameList, true);
    size_t size = defineNameList.size();
    for (size_t i = 0; i < size; ++i)
    {
        result += Format("#define %s\n", defineNameList[i].c_str());
    }
    SetDefines(result);
}
    
bool Shader::LoadFromYaml(const FilePath & pathname)
{
//#if defined(__DAVAENGINE_ANDROID__) || defined (__DAVAENGINE_MACOS__)
//    relativeFileName = pathname;
//#endif //#if defined(__DAVAENGINE_ANDROID__) 

    uint64 shaderLoadTime = SystemTimer::Instance()->AbsoluteMS();
    
    YamlParser * parser = YamlParser::Create(pathname);
    if (!parser)
        return false;
    
    YamlNode * rootNode = parser->GetRootNode();
    if (!rootNode)
    {
        SafeRelease(rootNode);
        return false;
    }
    
    YamlNode * vertexShaderNode = rootNode->Get("vertexShader");
    if (!vertexShaderNode)
    {
        SafeRelease(parser);
        return false;
    }

    YamlNode * glslVertexNode = vertexShaderNode->Get("glsl");
    if (!glslVertexNode)
    {
        SafeRelease(parser);
        return false;
    }
    
    YamlNode * fragmentShaderNode = rootNode->Get("fragmentShader");
    if (!fragmentShaderNode)
    {
        SafeRelease(parser);
        return false;
    }
    
    YamlNode * glslFragmentNode = fragmentShaderNode->Get("glsl");
    if (!glslFragmentNode)
    {
        SafeRelease(parser);
        return false;
    }

    FilePath pathOnly(pathname.GetDirectory());
    vertexShaderPath = pathOnly + glslVertexNode->AsString();
    fragmentShaderPath = pathOnly + glslFragmentNode->AsString();
    SafeRelease(parser);

    Load(vertexShaderPath, fragmentShaderPath);
    
    shaderLoadTime = SystemTimer::Instance()->AbsoluteMS() - shaderLoadTime;
    
//    Logger::Debug("shader loaded:%s load-time: %lld ms", pathname.c_str(), shaderLoadTime);
    return true;
}
    
bool Shader::Load(const FilePath & _vertexShaderPath, const FilePath & _fragmentShaderPath)
{
    vertexShaderPath = _vertexShaderPath;
    fragmentShaderPath = _fragmentShaderPath;
    uint32 vertexShaderSize = 0, fragmentShaderSize = 0;
        
    uint8 * vertexShaderBytes = FileSystem::Instance()->ReadFileContents(vertexShaderPath, vertexShaderSize);
    vertexShaderData = new Data(vertexShaderBytes, vertexShaderSize);
    
    uint8 * fragmentShaderBytes = FileSystem::Instance()->ReadFileContents(fragmentShaderPath, fragmentShaderSize);
    fragmentShaderData = new Data(fragmentShaderBytes, fragmentShaderSize);
    
    return true;
}
    
Shader::~Shader()
{
    SafeDeleteArray(attributeNames);
    SafeDeleteArray(uniforms);
    
    SafeRelease(vertexShaderData);
    SafeRelease(fragmentShaderData);
    
    DeleteShaders();
}
    
bool Shader::Recompile()
{
    DVASSERT((vertexShader == 0) && (fragmentShader == 0) && (program == 0));
    
    RenderManager::Instance()->LockNonMain();
    if (!CompileShader(&vertexShader, GL_VERTEX_SHADER, vertexShaderData->GetSize(), (GLchar*)vertexShaderData->GetPtr(), vertexShaderDefines))
    {
        Logger::Error("Failed to compile vertex shader: %s", vertexShaderPath.GetAbsolutePathname().c_str());
        return false;
    }
    
    if (!CompileShader(&fragmentShader, GL_FRAGMENT_SHADER, fragmentShaderData->GetSize(), (GLchar*)fragmentShaderData->GetPtr(), fragmentShaderDefines))
    {
        Logger::Error("Failed to compile fragment shader: %s", fragmentShaderPath.GetAbsolutePathname().c_str());
        return false;
    }
    
    program = glCreateProgram();
    RENDER_VERIFY(glAttachShader(program, vertexShader));
    RENDER_VERIFY(glAttachShader(program, fragmentShader));
    
    if (!LinkProgram(program))
    {
        Logger::Error("Failed to Link program for shader: %s", fragmentShaderPath.GetAbsolutePathname().c_str());

        DeleteShaders();
        return false;
    }
    
    
    RENDER_VERIFY(glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &activeAttributes));
    
    //Logger::Debug("shader loaded: %s attributeCount: %d", pathname.c_str(), activeAttributes);
    
    char attributeName[512];
    attributeNames = new FastName[activeAttributes];
    for (int32 k = 0; k < activeAttributes; ++k)
    {
        GLint size;
        GLenum type;
        RENDER_VERIFY(glGetActiveAttrib(program, k, 512, 0, &size, &type, attributeName));
        attributeNames[k] = attributeName;
        
        int32 flagIndex = GetAttributeIndexByName(attributeName);
        vertexFormatAttribIndeces[flagIndex] = glGetAttribLocation(program, attributeName);
//        Logger::Debug("shader attr: %s size: %d type: %s flagIndex: %d", attributeName, size, VertexTypeStringFromEnum(type).c_str(), flagIndex);
        //if (vertexFormatAttribIndeces[k] != -1)
        //    Logger::Debug("shader attr matched: 0x%08x", (1 << flagIndex));
    }
    
    RENDER_VERIFY(glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &activeUniforms));
    
    uniforms = new Uniform[activeUniforms];
    
    for (int32 k = 0; k < activeUniforms; ++k)
    {
        GLint size;
        GLenum type;
        RENDER_VERIFY(glGetActiveUniform(program, k, 512, 0, &size, &type, attributeName));
        eUniform uniform = GetUniformByName(attributeName);
        //Logger::Debug("shader uniform: %s size: %d type: %s", attributeName, size, VertexTypeStringFromEnum(type).c_str());
        uniforms[k].name = attributeName;
        uniforms[k].location = glGetUniformLocation(program, uniforms[k].name.c_str());
        uniforms[k].id = uniform;
        uniforms[k].type = (eUniformType)type;
        uniforms[k].size = size;
//        Logger::Debug("shader known uniform: %s(%d) size: %d type: %s", uniforms[k].name.c_str(), uniform, size, VertexTypeStringFromEnum(type).c_str());
    }
    
    
//    Logger::Debug("shader recompile success: %s", fragmentShaderPath.c_str());

    
    RenderManager::Instance()->UnlockNonMain();
    return true;
}
    
void Shader::SetUniformValue(int32 uniformLocation, int32 value)
{
    DVASSERT(uniformLocation >= 0);
    RENDER_VERIFY(glUniform1i(uniformLocation, value));
}
    
void Shader::SetUniformValue(int32 uniformLocation, float32 value)
{
    DVASSERT(uniformLocation >= 0);
    RENDER_VERIFY(glUniform1f(uniformLocation, value));
}

void Shader::SetUniformValue(int32 uniformLocation, const Vector2 & vector)
{
    DVASSERT(uniformLocation >= 0);
    RENDER_VERIFY(glUniform2fv(uniformLocation, 1, &vector.x));
}

void Shader::SetUniformValue(int32 uniformLocation, const Vector3 & vector)
{
    DVASSERT(uniformLocation >= 0);
    RENDER_VERIFY(glUniform3fv(uniformLocation, 1, &vector.x));
}

void Shader::SetUniformColor3(int32 uniformLocation, const Color & color)
{
    DVASSERT(uniformLocation >= 0);
    RENDER_VERIFY(glUniform3fv(uniformLocation, 1, &color.r));
}

void Shader::SetUniformColor4(int32 uniformLocation, const Color & color)
{
    DVASSERT(uniformLocation >= 0);
    RENDER_VERIFY(glUniform4fv(uniformLocation, 1, &color.r));
}

void Shader::SetUniformValue(int32 uniformLocation, const Vector4 & vector)
{
    DVASSERT(uniformLocation >= 0);
    RENDER_VERIFY(glUniform4fv(uniformLocation, 1, &vector.x));
}

void Shader::SetUniformValue(int32 uniformLocation, const Matrix4 & matrix)
{
    DVASSERT(uniformLocation >= 0);
    RENDER_VERIFY(glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, matrix.data));
}
    
int32 Shader::GetAttributeCount()
{
    return activeAttributes;
}
    
int32 Shader::GetAttributeIndex(eVertexFormat vertexFormat)
{
    return vertexFormatAttribIndeces[CountLeadingZeros(vertexFormat)];
}

void Shader::DeleteShaders()
{
    RenderManager::Instance()->LockNonMain();
    DVASSERT(vertexShader != 0);  
    DVASSERT(fragmentShader != 0);
    DVASSERT(program != 0);
    
    
    RENDER_VERIFY(glDetachShader(program, vertexShader));
    RENDER_VERIFY(glDetachShader(program, fragmentShader));
    RENDER_VERIFY(glDeleteShader(vertexShader));
    RENDER_VERIFY(glDeleteShader(fragmentShader));
    RENDER_VERIFY(glDeleteProgram(program));
    vertexShader = 0;
    fragmentShader = 0;
    program = 0;

    RenderManager::Instance()->UnlockNonMain();
}

/* Link a program with all currently attached shaders */
GLint Shader::LinkProgram(GLuint prog)
{
    RenderManager::Instance()->LockNonMain();

    GLint status;
    
    RENDER_VERIFY(glLinkProgram(prog));
    
    GLint logLength = 0;
    RENDER_VERIFY(glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength));
    if (logLength > 1)
    {
        GLchar *log = (GLchar *)malloc(logLength);
        RENDER_VERIFY(glGetProgramInfoLog(prog, logLength, &logLength, log));
        Logger::Debug("Program link log:\n%s", log);
        free(log);
    }
    
    RENDER_VERIFY(glGetProgramiv(prog, GL_LINK_STATUS, &status));
    if (status == GL_FALSE)
        Logger::Error("Failed to link program %d", prog);
    
    RenderManager::Instance()->UnlockNonMain();

    return status;
}
    
/* Create and compile a shader from the provided source(s) */
GLint Shader::CompileShader(GLuint *shader, GLenum type, GLint count, const GLchar * sources, const String & defines)
{
    RenderManager::Instance()->LockNonMain();

    GLint status;
    //const GLchar *sources;
        
    *shader = glCreateShader(type);				// create shader
    
    if (defines.length() == 0)
    {
        RENDER_VERIFY(glShaderSource(*shader, 1, &sources, &count));	// set source code in the shader
    }else
    {
        const GLchar * multipleSources[] = 
        {
            defines.c_str(),
            sources,
        };
        const GLint multipleCounts[] = 
        {
            (GLint)defines.length(),
            count,
        };
        RENDER_VERIFY(glShaderSource(*shader, 2, multipleSources, multipleCounts));	// set source code in the shader
    }
    
    RENDER_VERIFY(glCompileShader(*shader));					// compile shader
    
#ifdef __DAVAENGINE_DEBUG__
	{
		GLchar log[4096];
		GLsizei logLength = 0;
		RENDER_VERIFY(glGetShaderInfoLog(*shader, 4096, &logLength, log));
		if (logLength)
		{
			Logger::Debug("Shader compile log:\n%s", log);
		}
	}
#endif
    
    RENDER_VERIFY(glGetShaderiv(*shader, GL_COMPILE_STATUS, &status));
    if (status == GL_FALSE)
    {
        Logger::Error("Failed to compile shader: status == GL_FALSE\n");
    }
    RenderManager::Instance()->UnlockNonMain();

    return status;
}
    
void Shader::Unbind()
{
    if (activeProgram != 0)
    {
        RENDER_VERIFY(glUseProgram(0));
        activeProgram = 0;
    }
}
    
void Shader::Bind()
{
    if (activeProgram != program)
    {
        RENDER_VERIFY(glUseProgram(program));
        activeProgram = program;
    }
    
    for (int32 k = 0; k < activeUniforms; ++k)
    {
        switch (uniforms[k].id)
        {
        case UNIFORM_MODEL_VIEW_PROJECTION_MATRIX:
            {    
                const Matrix4 & modelViewProj = RenderManager::Instance()->GetUniformMatrix(RenderManager::UNIFORM_MATRIX_MODELVIEWPROJECTION);
                RENDER_VERIFY(glUniformMatrix4fv(uniforms[k].location, 1, GL_FALSE, modelViewProj.data));
                break;
            }
        case UNIFORM_MODEL_VIEW_MATRIX:
            {    
                const Matrix4 & modelView = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW);
                RENDER_VERIFY(glUniformMatrix4fv(uniforms[k].location, 1, GL_FALSE, modelView.data));
                break;
            }
		case UNIFORM_PROJECTION_MATRIX:
			{
				const Matrix4 & proj = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_PROJECTION);
                RENDER_VERIFY(glUniformMatrix4fv(uniforms[k].location, 1, GL_FALSE, proj.data));
				break;
			}
        case UNIFORM_NORMAL_MATRIX:
            {
                const Matrix3 & normalMatrix = RenderManager::Instance()->GetNormalMatrix();
                RENDER_VERIFY(glUniformMatrix3fv(uniforms[k].location, 1, GL_FALSE, normalMatrix.data));
                break;
            }
        case UNIFORM_COLOR:
            {
                const Color & c = RenderManager::Instance()->GetColor();
                RENDER_VERIFY(glUniform4fv(uniforms[k].location, 1, &c.r));
                break;
            }
        case UNIFORM_GLOBAL_TIME:
            {
                float32 globalTime = SystemTimer::Instance()->GetGlobalTime();
                RENDER_VERIFY(glUniform1f(uniforms[k].location, globalTime));
            };
        default:
            
            break;
        }
    }
    
}
    
void Shader::Dump()
{
    Logger::Debug("Attributes: ");
    for (int32 k = 0; k < activeAttributes; ++k)
    {
        int32 flagIndex = GetAttributeIndexByName(attributeNames[k].c_str());
        Logger::Debug("Attribute: %s location: %d vertexFormatIndex:%x", attributeNames[k].c_str(), vertexFormatAttribIndeces[flagIndex], flagIndex);
    }
    
    RENDER_VERIFY(glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &activeUniforms));
    
    
    Logger::Debug("Uniforms: ");
    for (int32 k = 0; k < activeUniforms; ++k)
    {
        //Logger::Debug("shader uniform: %s size: %d type: %s", attributeName, size, VertexTypeStringFromEnum(type).c_str());
//        uniformNames[k] = attributeName;
//        uniformLocations[k] = glGetUniformLocation(program, uniformNames[k].c_str());
//        uniformIDs[k] = uniform;
//        uniformTypes[k] = (eUniformType)type;
        eUniform uniform = GetUniformByName(uniforms[k].name.c_str());
        Logger::Debug("uniform: %s(%d) type: %s", uniforms[k].name.c_str(), uniform, VertexTypeStringFromEnum(uniforms[k].type).c_str());
    }

    
}
    
Shader * Shader::RecompileNewInstance(const String & combination)
{
    Shader * shader = new Shader();
    shader->vertexShaderData = SafeRetain(vertexShaderData);
    shader->fragmentShaderData = SafeRetain(fragmentShaderData);
    shader->SetDefineList(combination);
    if (!shader->Recompile())
    {
        SafeRelease(shader);
        return 0;
    }
    return shader;
}

    
//#if defined(__DAVAENGINE_ANDROID__) || defined (__DAVAENGINE_MACOS__)
//void Shader::SaveToSystemMemory()
//{
//    RenderResource::Lost();
//}
//
//void Shader::Lost()
//{
////    Logger::Debug("[Shader::Lost]");
//    DeleteShaders();
//    RenderResource::Lost();
//}
//void Shader::Invalidate()
//{
////    Logger::Debug("[Shader::Invalidate]");
////    LoadFromYaml(relativeFileName);
//    Recompile();
//    
//    RenderResource::Invalidate();
//}
//#endif //#if defined(__DAVAENGINE_ANDROID__) 


#endif 



}



