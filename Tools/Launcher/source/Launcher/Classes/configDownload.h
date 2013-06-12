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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QTimer>
#include <QString>
#include <QUrl>
#include <QMap>
#include <QStringList>
#include "AppType.h"
#include "yaml-cpp/yaml.h"

class AppConfig {
public:
    QString m_Name;
    QString m_Version;
    QUrl m_Url;
    QString m_RunPath;
    QStringList m_InstalledFiles;
    QString m_InstallCmd;
    QString m_UninstallCmd;
    QString m_InstallParams;
    QString m_UninstallParams;
    int m_nSuccessInstallCode;

    AppConfig() {
        m_nSuccessInstallCode = 0;
    }

    void Clear() {
        m_Name.clear();
        m_Version.clear();
        m_Url.clear();
        m_RunPath.clear();
        m_InstalledFiles.clear();
        m_InstallCmd.clear();
        m_UninstallCmd.clear();
        m_InstallParams.clear();
        m_UninstallParams.clear();
    }
};

class AppsConfig {
public:
    AppConfig m_Launcher;

    typedef QMap<QString, AppConfig> AppVersion;    //version, app config
    typedef QMap<QString, AppVersion> AppMap;
    AppMap m_Stable;
    AppMap m_Test;
    AppMap m_Development;
    AppMap m_Dependencies;

    QString m_pageUrl;

    void Clear();

    AppMap* GetAppMap(eAppType type) {
        switch (type) {
        case eAppTypeStable: {
            return &m_Stable;
        }break;
        case eAppTypeTest: {
            return &m_Test;
        }break;
        case eAppTypeDevelopment: {
            return &m_Development;
        }break;
        case eAppTypeDependencies: {
            return &m_Dependencies;
        }break;
        }
        return NULL;
    }
};

class ConfigDownloader : public QObject
{
    Q_OBJECT
public:
    explicit ConfigDownloader(QObject *parent = 0);
    
    void Init();

signals:
    void UpdateConfigFinished(const AppsConfig&);
    
public slots:
    void UpdateConfig();
    void ConfigDownloaded();

private:
    enum eDownloadState {
        eDownloadStateInactive,
        eDownloadStateLauncher,
        eDownloadStateStable,
        eDownloadStateTest,
        eDownloadStateDevelopment,
        eDownloadStateDependencies
    };

    void SetState(eDownloadState newState);
    void ParseAppData(AppsConfig::AppMap& appMap, const YAML::Node* pNode);
    
private:
    eDownloadState m_State;

    QNetworkAccessManager* m_pManager;
    QNetworkReply* m_pReply;

    AppsConfig m_UpdatedConfig;
    QString m_StableUrl;
    QString m_TestUrl;
    QString m_DevelopmentUrl;
    QString m_DependenciesUrl;
};

#endif // CONFIGURATION_H
