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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QMap>
#include "configDownload.h"
#include "AppType.h"

class Settings : public QObject
{
    Q_OBJECT

public:
    explicit Settings(QObject *parent = 0);
    
    static Settings* GetInstance();
    static QString GetVersion(const QString& strVersion);

    void Init();

    QString GetInstalledAppVersion(const QString& appName, eAppType type);

    int GetUpdateTimerInterval() const {return m_nUpdateTimer;}
    void SetLauncherVersion(const QString& version);
    QString GetLauncherVersion() const;
    void SetLauncherUrl(const QString& url);
    QString GetLauncherUrl() const;

    const AppsConfig& GetCurrentConfig() const;
    void UpdateConfig(const AppsConfig&);
    void SaveConfig() {UpdateInitConfig();};

signals:

public slots:

private:
    void ParseInitConfig();
    void UpdateInitConfig();

    void ParseAppConfig(const YAML::Node* pNode, const char* appType, AppsConfig::AppMap& appMap);
    void EmitAppConfig(YAML::Emitter& emitter, const char* appType, const AppsConfig::AppMap& appMap);
    
private:
    static Settings* m_spInstance;

    int m_nUpdateTimer;
    AppsConfig m_Config;
};

#endif // SETTINGS_H
