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

#ifndef UPDATER_H
#define UPDATER_H

#include <QObject>
#include "configDownload.h"
#include "selfupdater.h"
#include <QNetworkAccessManager>
#include "AppType.h"

//format sorfware list
class SoftWare
{
public:
    QString m_CurVersion;
    QString m_NewVersion;
    QString m_RunPath;
    bool m_bInstalling;
    QSet<QString> m_AvailableVersion;
    SoftWare() {
        m_bInstalling = false;
    }
};

class AvailableSoftWare
{
public:
    void Clear() {
        m_Stable.clear();
        m_toMaster.clear();
        m_Test.clear();
        m_Development.clear();
        m_Dependencies.clear();
    }

    typedef QMap<QString, SoftWare> SoftWareMap;
    SoftWareMap m_Stable;
    SoftWareMap m_toMaster;
    SoftWareMap m_Test;
    SoftWareMap m_Development;
    SoftWareMap m_Dependencies;
};

class Installer : public QObject
{
    Q_OBJECT
public:

    explicit Installer(QObject *parent = 0);
    void Init();
    
    void CheckForUpdate();
    bool Install(const QString& appName, const QString& appVersion, eAppType type);
    bool Delete(const QString& appName, eAppType type, bool force = false);
    QString GetRunPath(const QString& appName, eAppType type);
    bool Update(AvailableSoftWare::SoftWareMap softMap, eAppType type, bool force = false);
    bool AbortCurInstallation();

signals:
    void StartCheckForUpdate();
    void SoftWareAvailable(const AvailableSoftWare&);

    void StartDownload();
    void DownloadProgress(int);
    void DownloadFinished();
    void WebPageUpdated(const QString&);
    
public slots:
    void UpdateConfigFinished(const AppsConfig&);
    void OnAppDownloaded();
    void OnDownloadProgress(qint64,qint64);

private:
    void FormatFromSetting(AvailableSoftWare::SoftWareMap& softMap, const AppsConfig::AppMap& setting);
    void FormatFromUpdate(AvailableSoftWare::SoftWareMap& softMap, const AppsConfig::AppMap& update);

#ifdef Q_OS_WIN
    int RunAndWaitForFinish(const QString& fileName, const QString& params = "");
#endif

    const AppsConfig::AppMap* GetAppMap(eAppType type) const;
    QString GetInstallPath(eAppType) const;

    void UpdateAvailableSoftware();
    
private:
    ConfigDownloader* m_pConfigDownloader;
    SelfUpdater* m_pSelfUpdater;
    AvailableSoftWare m_AvailableSoftWare;

    AppsConfig m_AppsConfig;
    QNetworkAccessManager* m_pNetworkManager;
    QNetworkReply* m_pReply;
};

#endif // UPDATER_H
