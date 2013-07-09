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

#ifndef DIRECTORYMANAGER_H
#define DIRECTORYMANAGER_H

#include <QObject>

class DirectoryManager : public QObject
{
    Q_OBJECT
private:
    explicit DirectoryManager(QObject *parent = 0);
    
public:
    static DirectoryManager* GetInstance();
    static bool DeleteDir(const QString& path);
    static bool CopyAllFromDir(const QString& srcPath, const QString& destPath);
    static bool MoveFileToDir(const QString& srcFileName, const QString& destPath, const QString& newFileName = "");
    static QStringList GetDirectoryStructure(const QString& path);
    static bool IsFilePacked(const QString& fileName);

    void Init();

    QString GetBaseDirectory() const;
    QString GetAppDirectory() const;
    //QString GetRunPath() const {return m_runPath;}
    QString GetDownloadDir() const;
    QString GetStableDir() const;
    QString GetToMasterDir() const;
    QString GetTestDir() const;
    QString GetDevelopment() const;
    QString GetDependencies() const;
    QString GetConfigDir() const;

    QString GetDocumentsDirectory() const;

signals:
    
public slots:
    
private:
    void InitBaseDir();

    static DirectoryManager* m_spInstance;

    QString m_appDir;
    QString m_runPath;
    QString m_configDir;
};

#endif // DIRECTORYMANAGER_H
