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

#include <QVariant>
#include <QNetworkRequest>
#include <QNetworkReply>
#include "configDownload.h"
#include "settings.h"
#include "logger.h"
#include "yaml-cpp/yaml.h"

#define URL_launcher "URL_launcher"
#define launcher_ver "launcher_ver"
#define URL_stable "URL_stable"
#define URL_to_master "URL_to_master"
#define URL_test "URL_qa"
#define URL_development "URL_development"
#define URL_dependencies "URL_dependencies"
#define URL_page "URL_page"

#define setString(a, b) {std::string c; b->GetScalar(c); a = c.c_str();}

ConfigDownloader::ConfigDownloader(QObject *parent) :
    QObject(parent)
{
    m_State = eDownloadStateInactive;
    m_pManager = new QNetworkAccessManager(this);
    m_pReply = NULL;
}

void ConfigDownloader::Init()
{
}

void ConfigDownloader::UpdateConfig()
{
    m_UpdatedConfig.Clear();
    m_StableUrl.clear();
    m_ToMasterUrl.clear();
    m_TestUrl.clear();
    m_DevelopmentUrl.clear();
    m_DependenciesUrl.clear();

    Logger::GetInstance()->AddLog(tr("Start update config"));
    SetState(eDownloadStateLauncher);
}

void ConfigDownloader::ConfigDownloaded()
{
    if (!m_pReply)
        return;

    QByteArray data;
    QNetworkReply* pReply = m_pReply;
    m_pReply = NULL;
    data = pReply->readAll();

    YAML::Parser parser;
    std::istringstream streamData(data.data());
    parser.Load(streamData);
    YAML::Node doc;

    switch (m_State)
    {
    case eDownloadStateLauncher:{
        if (!parser.GetNextDocument(doc)) {
            Logger::GetInstance()->AddLog(tr("Error parse main config"));
            SetState(eDownloadStateInactive);
            return;
        } else {
            //parse launcher data
            const YAML::Node* URL_launcherNode = doc.FindValue(URL_launcher);
            if (URL_launcherNode) setString(m_UpdatedConfig.m_Launcher.m_Url, URL_launcherNode);
            const YAML::Node* launcher_verNode = doc.FindValue(launcher_ver);
            if (launcher_verNode) setString(m_UpdatedConfig.m_Launcher.m_Version, launcher_verNode);
            const YAML::Node* URL_stableNode = doc.FindValue(URL_stable);
            if (URL_stableNode)  setString(m_StableUrl, URL_stableNode);
            const YAML::Node* URL_to_masterNode = doc.FindValue(URL_to_master);
            if (URL_to_masterNode)  setString(m_ToMasterUrl, URL_to_masterNode);
            const YAML::Node* URL_testNode = doc.FindValue(URL_test);
            if (URL_testNode)  setString(m_TestUrl, URL_testNode);
            const YAML::Node* URL_developmentNode = doc.FindValue(URL_development);
            if (URL_developmentNode)  setString(m_DevelopmentUrl, URL_developmentNode);
            const YAML::Node* URL_dependenciesNode = doc.FindValue(URL_dependencies);
            if (URL_dependenciesNode)  setString(m_DependenciesUrl, URL_dependenciesNode);
            const YAML::Node* URL_pageNode = doc.FindValue(URL_page);
            if (URL_pageNode)  setString(m_UpdatedConfig.m_pageUrl, URL_pageNode);
        }

        SetState(eDownloadStateStable);
    }break;
    case eDownloadStateStable:{
        if (!parser.GetNextDocument(doc)) {
            Logger::GetInstance()->AddLog(tr("Error parse stable config"));
        } else {
            ParseAppData(m_UpdatedConfig.m_Stable, &doc);
        }

        SetState(eDownloadStateToMaster);
    }break;
    case eDownloadStateToMaster: {
        if (!parser.GetNextDocument(doc)) {
            Logger::GetInstance()->AddLog(tr("Error parse to_master config"));
        } else {
            ParseAppData(m_UpdatedConfig.m_toMaster, &doc);
        }

        SetState(eDownloadStateTest);
    }break;
    case eDownloadStateTest:{
        if (!parser.GetNextDocument(doc)) {
            Logger::GetInstance()->AddLog(tr("Error parse qa config"));
        } else {
            ParseAppData(m_UpdatedConfig.m_Test, &doc);
        }

        SetState(eDownloadStateDevelopment);
    }break;
    case eDownloadStateDevelopment:{
        if (!parser.GetNextDocument(doc)) {
            Logger::GetInstance()->AddLog(tr("Error parse development config"));
        } else {
            ParseAppData(m_UpdatedConfig.m_Development, &doc);
        }

        SetState(eDownloadStateDependencies);
    }break;
    case eDownloadStateDependencies:{
        if (!parser.GetNextDocument(doc)) {
            Logger::GetInstance()->AddLog(tr("Error parse dependencies config"));
        } else {
            ParseAppData(m_UpdatedConfig.m_Dependencies, &doc);
        }

        //SetState(eDownloadStateStable);
        emit UpdateConfigFinished(m_UpdatedConfig);
    }break;
    default:
        Q_ASSERT(!"Unknown state");
    };
}

void ConfigDownloader::SetState(eDownloadState newState) {
    m_State = newState;

    switch (m_State) {
    case eDownloadStateLauncher: {
        QString url = Settings::GetInstance()->GetLauncherUrl();
        m_pReply = m_pManager->get(QNetworkRequest(url));
    }break;
    case eDownloadStateStable: {
        m_pReply = m_pManager->get(QNetworkRequest(m_StableUrl));
    }break;
    case eDownloadStateToMaster: {
        m_pReply = m_pManager->get(QNetworkRequest(m_ToMasterUrl));
    }break;
    case eDownloadStateTest: {
        m_pReply = m_pManager->get(QNetworkRequest(m_TestUrl));
    }break;
    case eDownloadStateDevelopment: {
        m_pReply = m_pManager->get(QNetworkRequest(m_DevelopmentUrl));
    }break;
    case eDownloadStateDependencies: {
        m_pReply = m_pManager->get(QNetworkRequest(m_DependenciesUrl));
    }break;
    }

    if (m_pReply)
        connect(m_pReply, SIGNAL(finished()), this, SLOT(ConfigDownloaded()));
}

void ConfigDownloader::ParseAppData(AppsConfig::AppMap &appMap, const YAML::Node *pNode) {
    for (YAML::Iterator iter = pNode->begin(); iter != pNode->end(); ++iter) {
        const YAML::Node* nameNode = &(iter.first());
        if (!nameNode)
            continue;
        QString name;
        setString(name, nameNode);
        const YAML::Node& value = iter.second();
        const YAML::Node* pApp = value.FindValue("app");
        if (pApp)
            setString(name, pApp);
        const YAML::Node* pVer = value.FindValue("ver");
        const YAML::Node* pUrl = value.FindValue("url");
        const YAML::Node* pRunPath = value.FindValue("runPath");
        const YAML::Node* pInstallCmd = value.FindValue("install");
        const YAML::Node* pUninstallCmd = value.FindValue("uninstall");
        const YAML::Node* pInstallParams = value.FindValue("installParams");
        const YAML::Node* pUninstallParams = value.FindValue("uninstallParams");

        AppConfig config;
        config.m_Name = name;
        if (pVer) setString(config.m_Url, pUrl);
        if (pUrl) setString(config.m_Version, pVer);
        if (pRunPath) setString(config.m_RunPath, pRunPath);
        if (pInstallCmd) setString(config.m_InstallCmd, pInstallCmd);
        if (pUninstallCmd) setString(config.m_UninstallCmd, pUninstallCmd);
        if (pInstallParams) setString(config.m_InstallParams, pInstallParams);
        if (pUninstallParams) setString(config.m_UninstallParams, pUninstallParams);

        appMap[name][config.m_Version] = config;
    }
}

void AppsConfig::Clear() {
    m_Launcher.Clear();
    m_Stable.clear();
    m_toMaster.clear();
    m_Test.clear();
    m_Development.clear();
    m_Dependencies.clear();
}
