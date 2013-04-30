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
#include "FileSystem/FilePath.h"
#include "FileSystem/FileSystem.h"
#include "Utils/Utils.h"

namespace DAVA
{

String FilePath::projectPathname = String();

void FilePath::SetProjectPathname(const String &pathname)
{
    projectPathname = NormalizePathname(MakeDirectory(pathname));
}

const String & FilePath::GetProjectPathname()
{
    return projectPathname;
}
    
FilePath::FilePath()
{
    absolutePathname = String();
}

FilePath::FilePath(const FilePath &path)
{
    absolutePathname = path.absolutePathname;
}
    
FilePath::FilePath(const char * sourcePath)
{
	if(sourcePath)
        Initialize(String(sourcePath));
	else
		absolutePathname = String();
}

FilePath::FilePath(const String &pathname)
{
	if(pathname.empty())
		absolutePathname = String();
	else
        Initialize(String(pathname));
}

    
FilePath::FilePath(const String &directory, const String &filename)
{
	FilePath directoryPath(directory);
	directoryPath.MakeDirectoryPathname();

	absolutePathname = AddPath(directoryPath, filename);
}
    
void FilePath::Initialize(const String &pathname)
{
    if(IsAbsolutePathname(pathname))
    {
        absolutePathname = GetSystemPathname(pathname);
    }
    else
    {
        if(     (pathname.find("FBO ") == 0)
            ||  (pathname.find("memoryfile_0x") == 0)
            ||  (pathname.find("Text ") == 0))
        {
            absolutePathname = pathname;
        }
        else
        {
            FilePath path = FileSystem::Instance()->GetCurrentWorkingDirectory() + pathname;
            absolutePathname = path.GetAbsolutePathname();
        }
    }
}

    
    
FilePath::~FilePath()
{
    
}

FilePath& FilePath::operator=(const FilePath &path)
{
    this->absolutePathname = path.absolutePathname;
    return *this;
}
    
FilePath FilePath::operator+(const FilePath &path) const
{
    FilePath pathname(AddPath(*this, path));
    return pathname;
}

FilePath FilePath::operator+(const String &path) const
{
    FilePath pathname(AddPath(*this, path));
    return pathname;
}

FilePath FilePath::operator+(const char * path) const
{
    FilePath pathname(AddPath(*this, String(path)));
    return pathname;
}

FilePath& FilePath::operator+=(const FilePath & path)
{
    absolutePathname = AddPath(*this, path);
    return (*this);
}
    
FilePath& FilePath::operator+=(const String & path)
{
    absolutePathname = AddPath(*this, path);
    return (*this);
}

FilePath& FilePath::operator+=(const char * path)
{
    absolutePathname = AddPath(*this, String(path));
    return (*this);
}
    
bool FilePath::operator==(const FilePath &path) const
{
    return absolutePathname == path.absolutePathname;
}

bool FilePath::operator!=(const FilePath &path) const
{
    return absolutePathname != path.absolutePathname;
}

    
bool FilePath::IsDirectoryPathname() const
{
    if(IsEmpty())
    {
        return false;
    }

    const int32 lastPosition = absolutePathname.length() - 1;
    return (absolutePathname.at(lastPosition) == '/');
}



String FilePath::GetFilename() const
{
    return GetFilename(absolutePathname);
}
    
String FilePath::GetFilename(const String &pathname)
{
    String::size_type dotpos = pathname.rfind(String("/"));
    if (dotpos == String::npos)
        return pathname;
    
    return pathname.substr(dotpos+1);
}



String FilePath::GetBasename() const
{
    const String filename = GetFilename();
    
    const String::size_type dotpos = filename.rfind(String("."));
	if (dotpos == String::npos)
		return filename;
    
	return filename.substr(0, dotpos);
}

String FilePath::GetExtension() const
{
    const String filename = GetFilename();
    
    const String::size_type dotpos = filename.rfind(String("."));
	if (dotpos == String::npos)
        return String();
    
    return filename.substr(dotpos);
}

    
FilePath FilePath::GetDirectory() const
{
    return GetDirectory(absolutePathname);
}

FilePath FilePath::GetDirectory(const String &pathname)
{
    const String::size_type slashpos = pathname.rfind(String("/"));
    if (slashpos == String::npos)
        return FilePath();

    return FilePath(pathname.substr(0, slashpos + 1));
}

    
String FilePath::GetRelativePathname() const
{
    return GetRelativePathname(FileSystem::Instance()->GetCurrentWorkingDirectory());
}
    
String FilePath::GetRelativePathname(const FilePath &forDirectory) const
{
    DVASSERT(forDirectory.IsDirectoryPathname() || forDirectory.IsEmpty());
    
    return AbsoluteToRelative(forDirectory.GetAbsolutePathname(), absolutePathname);
}

String FilePath::GetRelativePathname(const String &forDirectory) const
{
    if(IsEmpty())
        return String();
    
	return GetRelativePathname(FilePath(forDirectory));
}
    
String FilePath::GetRelativePathname(const char * forDirectory) const
{
    if(forDirectory == NULL)
        return String();
    
	return GetRelativePathname(FilePath(forDirectory));
}

    
    
void FilePath::ReplaceFilename(const String &filename)
{
    DVASSERT(!IsEmpty());
    
    const FilePath directory = GetDirectory();
    absolutePathname = NormalizePathname(directory + filename);
}
    
void FilePath::ReplaceBasename(const String &basename)
{
    DVASSERT(!IsEmpty());
    
    const FilePath directory = GetDirectory();
    const String extension = GetExtension();
    absolutePathname = NormalizePathname(directory + (basename + extension));
}
    
void FilePath::ReplaceExtension(const String &extension)
{
    DVASSERT(!IsEmpty());
    
    const FilePath directory = GetDirectory();
    const String basename = GetBasename();
    absolutePathname = NormalizePathname(directory + (basename + extension));
}
    
void FilePath::ReplaceDirectory(const String &directory)
{
    DVASSERT(!IsEmpty());
    
    const String filename = GetFilename();
    absolutePathname = NormalizePathname(MakeDirectory(directory)) + filename;
}
    
void FilePath::ReplaceDirectory(const FilePath &directory)
{
    DVASSERT(!IsEmpty());
    
    DVASSERT(directory.IsDirectoryPathname())
    const String filename = GetFilename();
    absolutePathname = directory.GetAbsolutePathname() + filename;
}
    
void FilePath::MakeDirectoryPathname()
{
    DVASSERT(!IsEmpty());
    
    absolutePathname = MakeDirectory(absolutePathname);
}
    
void FilePath::TruncateExtension()
{
    DVASSERT(!IsEmpty());
    
    ReplaceExtension(String(""));
}
    
String FilePath::GetLastDirectoryName() const
{
    DVASSERT(!IsEmpty() && IsDirectoryPathname());
    
    String path = absolutePathname;
    path = path.substr(0, path.length() - 1);
    
    return FilePath(path).GetFilename();
}
    
bool FilePath::IsEqualToExtension( const String & extension ) const
{
	String selfExtension = GetExtension();
	return (CompareCaseInsensitive(extension, selfExtension) == 0);
}

    
FilePath FilePath::CreateWithNewExtension(const FilePath &pathname, const String &extension)
{
    FilePath path(pathname);
    path.ReplaceExtension(extension);
    return path;
}

    
String FilePath::GetSystemPathname(const String &pathname)
{
    if(pathname.empty() || pathname[0] != '~')
    {
		return pathname;
    }
    
    String retPath = pathname;
    String::size_type find = pathname.find("~res:");
	if(find != String::npos)
	{
        if(projectPathname.empty())
        {
            retPath = FileSystem::Instance()->SystemPathForFrameworkPath(retPath).GetAbsolutePathname();
        }
        else
        {
            retPath = retPath.erase(0, 6);
            retPath = projectPathname + "Data/" + retPath;
        }
	}
	else
	{
		find = retPath.find("~doc:");
		if(find != String::npos)
		{
			retPath = retPath.erase(0, 5);
			retPath = FileSystem::Instance()->FilepathInDocuments(retPath).GetAbsolutePathname();
		}
	}
    
    return NormalizePathname(retPath);
}
    

String FilePath::GetFrameworkPath()
{
    String pathInRes = GetFrameworkPathForPrefix("~res:/");
    if(!pathInRes.empty())
    {
        return pathInRes;
    }
    
    String pathInDoc = GetFrameworkPathForPrefix("~doc:/");
    if(!pathInDoc.empty())
    {
        return pathInDoc;
    }
    
	DVASSERT(false);

	return String();
}


String FilePath::GetFrameworkPathForPrefix( const String &typePrefix )
{
    DVASSERT(!typePrefix.empty())
    
	String prefixPathname = GetSystemPathname(typePrefix);

	String::size_type pos = absolutePathname.find(prefixPathname);
	if(pos == 0)
	{
		String pathname = absolutePathname;
		pathname = pathname.replace(pos, prefixPathname.length(), typePrefix);
		return pathname;
	}

	return String();
}


String FilePath::NormalizePathname(const FilePath &pathname)
{
    return NormalizePathname(pathname.GetAbsolutePathname());
}

String FilePath::NormalizePathname(const String &pathname)
{
	if(pathname.empty())
		return String();
	
	String path = pathname;
    std::replace(path.begin(), path.end(),'\\','/');
    
    Vector<String> tokens;
    Split(path, "/", tokens);
    
    //TODO: correctly process situation ../../folders/filename
    for (int32 i = 0; i < (int32)tokens.size(); ++i)
    {
        if (String(".") == tokens[i])
        {
            for (int32 k = i + 1; k < (int32)tokens.size(); ++k)
            {
                tokens[k - 1] = tokens[k];
            }
            --i;
            tokens.pop_back();
        }
        else if ((1 <= i) && (String("..") == tokens[i] && String("..") != tokens[i-1]))
        {
            for (int32 k = i + 1; k < (int32)tokens.size(); ++k)
            {
                tokens[k - 2] = tokens[k];
            }
            i-=2;
            tokens.pop_back();
            tokens.pop_back();
        }
    }
    
    String result = "";
    if('/' == path[0])
		result = "/";
    
    for (int32 k = 0; k < (int32)tokens.size(); ++k)
    {
        result += tokens[k];
        if (k + 1 != (int32)tokens.size())
            result += String("/");
    }
    
	//process last /
	if(('/' == path[path.length() - 1]) && (path.length() != 1))
		result += String("/");
    
    return result;
}

String FilePath::MakeDirectory(const String &pathname)
{
    if(pathname.empty())
    {
        return String();
    }
    
    const int32 lastPosition = pathname.length() - 1;
    if(pathname.at(lastPosition) != '/')
    {
        return pathname + String("/");
    }
    
    return pathname;
}
    
String FilePath::AbsoluteToRelative(const String &directoryPathname, const String &absolutePathname)
{
    if(absolutePathname.empty())
        return String();
    
    
    String workingDirectoryPath = directoryPathname;
    String workingFilePath = absolutePathname;
    
    DVASSERT(IsAbsolutePathname(absolutePathname));
    
    FilePath filePath = GetDirectory(workingFilePath);
    String fileName = GetFilename(workingFilePath);
    
    Vector<String> folders;
    Split(workingDirectoryPath, "/", folders);
    Vector<String> fileFolders;
    Split(filePath.GetAbsolutePathname(), "/", fileFolders);
    
    Vector<String>::size_type equalCount = 0;
    for(; equalCount < folders.size() && equalCount < fileFolders.size(); ++equalCount)
    {
        if(folders[equalCount] != fileFolders[equalCount])
        {
            break;
        }
    }
    
    String retPath = "";
    for(Vector<String>::size_type i = equalCount; i < folders.size(); ++i)
    {
        retPath += "../";
    }
    
    for(Vector<String>::size_type i = equalCount; i < fileFolders.size(); ++i)
    {
        retPath += fileFolders[i] + "/";
    }
    
    return (retPath + fileName);
}
    
bool FilePath::IsAbsolutePathname(const String &pathname)
{
    if(pathname.empty())
        return false;
    
    //Unix style
    if(pathname[0] == '/')
        return true;
    
    //Win or DAVA style (c:/, ~res:/, ~doc:/)
    String::size_type winFound = pathname.find(":/");
    if(winFound != String::npos)
    {
        return true;
    }
    
    return false;
}
    
String FilePath::AddPath(const FilePath &folder, const String & addition)
{
    DVASSERT(folder.IsDirectoryPathname() || folder.IsEmpty());

    String pathname = folder.GetAbsolutePathname() + addition;
	return NormalizePathname(pathname);
}


String FilePath::AddPath(const FilePath &folder, const FilePath & addition)
{
    DVASSERT(folder.IsDirectoryPathname() || folder.IsEmpty());
    
    String pathname = folder.GetAbsolutePathname() + addition.GetAbsolutePathname();
	return NormalizePathname(pathname);
}

    
}
