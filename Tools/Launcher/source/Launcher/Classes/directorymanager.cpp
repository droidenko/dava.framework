/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "directorymanager.h"
#include <QtCore>
#include <QList>

DirectoryManager* DirectoryManager::m_spInstance = NULL;

#define DOWNLOAD_DIR "/Downloads/"
#define STABLE_DIR "/Stable/"
#define TOMASTER_DIR "/ToMaster/"
#define TEST_DIR "/QA/"
#define DEVELOPMENT_DIR "/Development/"
#define DEPENDENCIES_DIR "/Dependencies/"

DirectoryManager* DirectoryManager::GetInstance() {
    if (!m_spInstance)
        m_spInstance = new DirectoryManager();
    return m_spInstance;
}

DirectoryManager::DirectoryManager(QObject *parent) :
    QObject(parent) {
}

void DirectoryManager::Init() {
    InitBaseDir();

    QDir().mkdir(GetDownloadDir());
    QDir().mkdir(GetStableDir());
    QDir().mkdir(GetToMasterDir());
    QDir().mkdir(GetTestDir());
    QDir().mkdir(GetDevelopment());
    QDir().mkdir(GetDependencies());
}

void DirectoryManager::InitBaseDir() {
    m_runPath = qApp->applicationFilePath();
    m_appDir = qApp->applicationDirPath();
#ifdef Q_OS_DARWIN
    m_appDir.replace("/Contents/MacOS", "");
#endif

    m_configDir = m_appDir;

    int nPos = m_appDir.lastIndexOf("/");
    if (nPos != -1)
        m_appDir.chop(m_appDir.size() - nPos);

#ifdef Q_OS_DARWIN
     m_configDir = m_appDir;

     nPos = m_appDir.lastIndexOf("/");
     if (nPos != -1)
         m_appDir.chop(m_appDir.size() - nPos);
#endif
}

QString DirectoryManager::GetDocumentsDirectory() const {
    QString path = QDir::homePath() + "/Documents/DAVAProject/Launcher/";
    if(!QDir(path).exists())
        QDir().mkdir(path);
    return path;
}

QString DirectoryManager::GetBaseDirectory() const {
    return m_appDir;
}

QString DirectoryManager::GetAppDirectory() const {
    QString  appDir = qApp->applicationDirPath();
#if defined(Q_OS_DARWIN) || defined(Q_OS_DARWIN64)
    appDir.replace("/Contents/MacOS", "");
#endif
    return appDir;
}

QString DirectoryManager::GetDownloadDir() const {
    QString baseDir = GetBaseDirectory();
    baseDir.append(DOWNLOAD_DIR);
    return baseDir;
}

QString DirectoryManager::GetStableDir() const {
    QString baseDir = GetBaseDirectory();
    baseDir.append(STABLE_DIR);
    return baseDir;
}

QString DirectoryManager::GetToMasterDir() const {
    QString baseDir = GetBaseDirectory();
    baseDir.append(TOMASTER_DIR);
    return baseDir;
}

QString DirectoryManager::GetTestDir() const {
    QString baseDir = GetBaseDirectory();
    baseDir.append(TEST_DIR);
    return baseDir;
}

QString DirectoryManager::GetDevelopment() const {
    QString baseDir = GetBaseDirectory();
    baseDir.append(DEVELOPMENT_DIR);
    return baseDir;
}

QString DirectoryManager::GetDependencies() const {
    QString baseDir = GetBaseDirectory();
    baseDir.append(DEPENDENCIES_DIR);
    return baseDir;
}

bool DirectoryManager::DeleteDir(const QString& path) {
    bool result = true;
    QDir aDir(path);
    if (aDir.exists())//QDir::NoDotAndDotDot
    {
        QFileInfoList entries = aDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files | QDir::Hidden);
        int count = entries.size();
        for (int idx = 0; idx < count; idx++)
        {
            QFileInfo entryInfo = entries[idx];
            QString path = entryInfo.absoluteFilePath();
            if (entryInfo.isDir())
            {
                result = DeleteDir(path);
            }
            else
            {
                QFile file(path);
                if (!file.remove())
                    result = false;
            }
        }
        if (!aDir.rmdir(aDir.absolutePath()))
            result = false;
    }
    return result;
}

bool DirectoryManager::CopyAllFromDir(const QString& srcPath, const QString& destPath) {
    bool result = true;
    QDir aDir(srcPath);
    if (aDir.exists())//QDir::NoDotAndDotDot
    {
        QFileInfoList entries = aDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files | QDir::Hidden);
        int count = entries.size();
        for (int idx = 0; idx < count; idx++)
        {
            QFileInfo entryInfo = entries[idx];

            if (entryInfo.isSymLink())
                continue;

            QString path = entryInfo.absoluteFilePath();
            path = path.replace(srcPath, "", Qt::CaseInsensitive);
            if (entryInfo.isDir())
            {
                if (!QDir(destPath + path).exists() && !QDir().mkdir(destPath + path))
                {
                    result = false;
                    continue;
                }
                result &= CopyAllFromDir(srcPath + path, destPath + path);
            }
            else
            {
                QFile().remove(destPath + path);
                result &= QFile().copy(srcPath + path, destPath + path);
            }
        }
        //if (!aDir.rmdir(aDir.absolutePath()))
        //    result = false;
    }
    return result;
}

bool DirectoryManager::IsFilePacked(const QString& fileName) {
    bool result = false;
    QFileInfo aFileInfo(fileName);
    QString aFileExt = aFileInfo.suffix();

    QList<QString> supportedArchives;
    supportedArchives.push_back("zip");

    if (supportedArchives.contains(aFileExt)) {
        result = true;
    }

    return result;
}

bool DirectoryManager::MoveFileToDir(const QString& srcFileName, const QString& destPath, const QString& newFileName) {
    bool result = true;
    QFile aFile(srcFileName);
    QFileInfo aFileInfo(aFile);
    QDir aDir(destPath);
    if (aFile.exists() && aDir.exists()) {
        QString aNewFileName = newFileName;
        if(newFileName == "") {
            aNewFileName = aFileInfo.fileName();
        }
        result = aFile.rename(destPath + "/" + aNewFileName);
    }
    return result;
}

QStringList DirectoryManager::GetDirectoryStructure(const QString& path) {
    QStringList list;

    QDir aDir(path);
    if (aDir.exists())//QDir::NoDotAndDotDot
    {
        QFileInfoList entries = aDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files | QDir::Hidden);
        int count = entries.size();
        for (int idx = 0; idx < count; idx++)
        {
            QFileInfo entryInfo = entries[idx];
            list.append(entryInfo.fileName());
        }
    }

    return list;
}

QString DirectoryManager::GetConfigDir() const {
    return m_configDir;
}
