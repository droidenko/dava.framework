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

#include "installer.h"
#include "settings.h"
#include "logger.h"
#include <QString>
#include "directorymanager.h"
#include "ziphelper.h"
#include "processhelper.h"
#include <QMessageBox>

#ifdef Q_OS_WIN
#include "Windows.h"
#include "ShellAPI.h"
#endif

#define APP_DOWNLOAD "AppDownloaded"
#define APP_TEMP_DIR "AppTempDir"

#define INSTALL_TYPE "INSTALL_TYPE"
#define INSTALL_NAME "INSTALL_NAME"
#define INSTALL_VERISON "INSTALL_VERISON"

#define APP_INSTALLER_DIR "Installers"

Installer::Installer(QObject *parent) :
    QObject(parent)
{
    m_pConfigDownloader = new ConfigDownloader(this);
    m_pSelfUpdater = new SelfUpdater(this);
    m_pNetworkManager = new QNetworkAccessManager(this);
    m_pReply = NULL;
}

void Installer::Init() {
    m_pConfigDownloader->Init();
    connect(m_pConfigDownloader, SIGNAL(UpdateConfigFinished(AppsConfig)), this, SLOT(UpdateConfigFinished(AppsConfig)));
    CheckForUpdate();
}

void Installer::CheckForUpdate() {
    Logger::GetInstance()->AddLog(tr("Start update"));

    emit StartCheckForUpdate();

    m_pConfigDownloader->UpdateConfig();
}

void Installer::UpdateConfigFinished(const AppsConfig & update) {
    Logger::GetInstance()->AddLog(tr("Config updated"));

    m_AppsConfig = update;

    UpdateAvailableSoftware();

    //install all dependencies
    for (AvailableSoftWare::SoftWareMap::const_iterator iter = m_AvailableSoftWare.m_Dependencies.begin();
         iter != m_AvailableSoftWare.m_Dependencies.end();
         ++iter) {
        const SoftWare& soft = iter.value();
        if (soft.m_CurVersion.isEmpty()) {
            Install(iter.key(), soft.m_NewVersion, eAppTypeDependencies);
            break; //wait until install finished
        }
    }

    emit WebPageUpdated(m_AppsConfig.m_pageUrl);

    Update(m_AvailableSoftWare.m_Stable, eAppTypeStable, true);
    Update(m_AvailableSoftWare.m_Test, eAppTypeTest, true);
//  Update(m_AvailableSoftWare.m_Development, eAppTypeDevelopment);
    Update(m_AvailableSoftWare.m_Dependencies, eAppTypeDependencies, true);
}

void Installer::UpdateAvailableSoftware() {
    //merge available software list
    const AppsConfig& currentConfig = Settings::GetInstance()->GetCurrentConfig();

    if (Settings::GetVersion(currentConfig.m_Launcher.m_Version) != Settings::GetVersion(m_AppsConfig.m_Launcher.m_Version)) {
        //run selfupdate
        Logger::GetInstance()->AddLog(tr("Start self update"));
        m_pSelfUpdater->UpdatedConfigDownloaded(m_AppsConfig);
        return;
    }

    m_AvailableSoftWare.Clear();
    //get installed soft
    FormatFromSetting(m_AvailableSoftWare.m_Stable, currentConfig.m_Stable);
    FormatFromSetting(m_AvailableSoftWare.m_Test, currentConfig.m_Test);
    FormatFromSetting(m_AvailableSoftWare.m_Development, currentConfig.m_Development);
    FormatFromSetting(m_AvailableSoftWare.m_Dependencies, currentConfig.m_Dependencies);
    //merge with update
    FormatFromUpdate(m_AvailableSoftWare.m_Stable, m_AppsConfig.m_Stable);
    FormatFromUpdate(m_AvailableSoftWare.m_Test, m_AppsConfig.m_Test);
    FormatFromUpdate(m_AvailableSoftWare.m_Development, m_AppsConfig.m_Development);
    FormatFromUpdate(m_AvailableSoftWare.m_Dependencies, m_AppsConfig.m_Dependencies);

    emit SoftWareAvailable(m_AvailableSoftWare);
}


void Installer::FormatFromSetting(AvailableSoftWare::SoftWareMap& softMap, const AppsConfig::AppMap& setting) {
    for (AppsConfig::AppMap::const_iterator appIter = setting.begin(); appIter != setting.end(); ++appIter) {
        AppsConfig::AppVersion::const_iterator iter = appIter.value().begin();
        if (iter == appIter.value().end())
            continue;

        const AppConfig& appConfig = iter.value();

        SoftWare soft;
        soft.m_CurVersion = appConfig.m_Version;
        soft.m_RunPath = appConfig.m_RunPath;
        softMap[appConfig.m_Name] = soft;
    }
}

void Installer::FormatFromUpdate(AvailableSoftWare::SoftWareMap& softMap, const AppsConfig::AppMap& update) {

    for(AvailableSoftWare::SoftWareMap::iterator softIt = softMap.begin(); softIt != softMap.end(); softIt++) {
        if(!update.contains(softIt.key()))
            softIt = softMap.erase(softIt);
    }

    for (AppsConfig::AppMap::const_iterator appIter = update.begin(); appIter != update.end(); ++appIter) {
        for (AppsConfig::AppVersion::const_iterator iter = appIter.value().begin(); iter != appIter.value().end(); ++iter) {
            const AppConfig& appConfig = iter.value();

            if (softMap.contains(appConfig.m_Name)) {
                softMap[appConfig.m_Name].m_AvailableVersion.insert(appConfig.m_Version);
            } else {
                SoftWare soft;
                soft.m_AvailableVersion.insert(appConfig.m_Version);
                softMap[appConfig.m_Name] = soft;
            }
            if (Settings::GetVersion(softMap[appConfig.m_Name].m_NewVersion) != Settings::GetVersion(appConfig.m_Version))
                softMap[appConfig.m_Name].m_NewVersion = appConfig.m_Version;
        }
    }
}

const AppsConfig::AppMap* Installer::GetAppMap(eAppType type) const {
    const AppsConfig::AppMap* apps = NULL;
    switch (type)
    {
    case eAppTypeStable: {
        apps = &m_AppsConfig.m_Stable;
    }break;
    case eAppTypeTest: {
        apps = &m_AppsConfig.m_Test;
    }break;
    case eAppTypeDevelopment: {
        apps = &m_AppsConfig.m_Development;
    }break;
    case eAppTypeDependencies: {
        apps = &m_AppsConfig.m_Dependencies;
    }break;
    }
    return apps;
}

QString Installer::GetInstallPath(eAppType type) const {
    switch (type) {
    case eAppTypeStable: {
        return DirectoryManager::GetInstance()->GetStableDir();
    }break;
    case eAppTypeTest: {
        return DirectoryManager::GetInstance()->GetTestDir();
    }break;
    case eAppTypeDevelopment: {
        return DirectoryManager::GetInstance()->GetDevelopment();
    }break;
    case eAppTypeDependencies: {
        return DirectoryManager::GetInstance()->GetDependencies();
    }break;
    }
    return "";
}

bool Installer::Install(const QString& appName, const QString& appVersion, eAppType type) {
    Logger::GetInstance()->AddLog(tr("Installing %1").arg(appName));

    const AppsConfig::AppMap* apps = GetAppMap(type);
    if (!apps)
        return false;

    AppsConfig::AppMap::const_iterator appIter = apps->find(appName);
    if (appIter == apps->end())
        return false;

    AppsConfig::AppVersion::const_iterator iter = appIter.value().find(appVersion);
    if (iter == appIter->end())
        return false;

    const AppConfig& config = iter.value();

    m_pReply = m_pNetworkManager->get(QNetworkRequest(config.m_Url));
    connect(m_pReply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(OnDownloadProgress(qint64, qint64)));
    connect(m_pReply, SIGNAL(finished()), this, SLOT(OnAppDownloaded()));
    m_pReply->setProperty(INSTALL_TYPE, type);
    m_pReply->setProperty(INSTALL_NAME, appName);
    m_pReply->setProperty(INSTALL_VERISON, appVersion);

    emit StartDownload();

    return true;
}

bool Installer::AbortCurInstallation() {
    if (!m_pReply)
        return false;

    QString name = m_pReply->property(INSTALL_NAME).toString();
    m_pReply->abort();
    Logger::GetInstance()->AddLog(tr("%1 app installation canceled").arg(name));
    return true;
}

void Installer::OnDownloadProgress(qint64 c, qint64 t) {
    if (t > 0) {
        int nPercent = c * 100 / t;
        emit DownloadProgress(nPercent);
    }
}

void Installer::OnAppDownloaded() {
    emit DownloadFinished();

    if (!m_pReply)
        return;

    eAppType type = (eAppType)m_pReply->property(INSTALL_TYPE).toInt();
    QString appName = m_pReply->property(INSTALL_NAME).toString();
    QString appVersion = m_pReply->property(INSTALL_VERISON).toString();
    const AppsConfig::AppMap* pUpdateMap = m_AppsConfig.GetAppMap(type);
    if (!pUpdateMap)
        return;

    AppsConfig::AppMap::const_iterator appIter = pUpdateMap->find(appName);
    if (appIter == pUpdateMap->end())
        return;

    AppsConfig::AppVersion::const_iterator iter = appIter.value().find(appVersion);
    if (iter == appIter.value().end())
        return;

    const AppConfig& updateConfig = iter.value();

    QByteArray data = m_pReply->readAll();
    if (!data.size()) {
        Logger::GetInstance()->AddLog(tr("Error download app"));
        return;
    }

    //save archive
    QString tempFilePath = DirectoryManager::GetInstance()->GetDownloadDir() + APP_DOWNLOAD;
    QFile tempFile(tempFilePath);
    if (!tempFile.open(QFile::WriteOnly | QFile::Truncate)) {
        Logger::GetInstance()->AddLog(tr("Error save archive"));
        return;
    }
    tempFile.write(data);
    tempFile.close();

    //unpack archive
    QString tempDir = DirectoryManager::GetInstance()->GetDownloadDir() + APP_TEMP_DIR;
    if (!DirectoryManager::DeleteDir(tempDir)) {
        Logger::GetInstance()->AddLog(tr("Error delete old directory"));
        return;
    }
    if (!QDir().mkdir(tempDir)) {
        Logger::GetInstance()->AddLog(tr("Error create temp directory"));
        return;
    }

    //if file is a zip-archive - unpack, otherwise - just move to the tempDir
    //check is performed by server's file extension
    QFileInfo serverFileInfo(updateConfig.m_Url.toString());
    QString serverFileName = serverFileInfo.fileName();

    if (DirectoryManager::IsFilePacked(serverFileName)) {
        if (!zipHelper::unZipFile(tempFilePath, tempDir)) {
            Logger::GetInstance()->AddLog(tr("Error unpack archive"));
            return;
        }
    } else {
        if (!DirectoryManager::MoveFileToDir(tempFilePath, tempDir, serverFileName)) {
            Logger::GetInstance()->AddLog(tr("Error move file to the temp directory"));
            return;
        }
    }

    //copy to destination
    QString installPath = GetInstallPath(type);

    //no install just copy file to new folder
    QStringList installedFiles = DirectoryManager::GetDirectoryStructure(tempDir);

    bool bInstalled = false;
#ifdef Q_OS_WIN
    if (updateConfig.m_InstallCmd.isEmpty()) {
#endif
        bInstalled = DirectoryManager::CopyAllFromDir(tempDir, installPath);
        if (!bInstalled) {
            Logger::GetInstance()->AddLog(tr("Error copy apps to destination"));
        }
#ifdef Q_OS_WIN
    }else {
        QString InstallerPath;
        do {
            //copy installer
            //create path for save installer
            QString InstallersPath = installPath + APP_INSTALLER_DIR;
            if (!QDir().exists(InstallersPath) &&
                !QDir().mkdir(InstallersPath)) {
                Logger::GetInstance()->AddLog(tr("Error create installers path"));
                break;
            }
            //create app installer path
            InstallerPath = InstallersPath + "/" + appName;
            DirectoryManager::DeleteDir(InstallerPath);
            if (!QDir().mkdir(InstallerPath)) {
                Logger::GetInstance()->AddLog(tr("Error create installer path"));
                break;
            }
            //copy installer
            if (!DirectoryManager::CopyAllFromDir(tempDir, InstallerPath)) {
                Logger::GetInstance()->AddLog(tr("Error copy installer"));
                break;
            }
            QString installer = InstallerPath + "/" + updateConfig.m_InstallCmd;

            QFile aFile(installer);
            if (!aFile.exists()) {
                Logger::GetInstance()->AddLog(tr("Error running installer - file not found"));
                break;
            }

            QString params = updateConfig.m_InstallParams;

            int nExitCode = RunAndWaitForFinish(installer, params);

            if (updateConfig.m_nSuccessInstallCode == nExitCode)
                bInstalled = true;
        } while(false);

        if (!bInstalled) {
            //hack after cancel install directory can't be delete at once
            //so we rename it and then delete
            QString temp = InstallerPath + QString::number(QDateTime().currentMSecsSinceEpoch());
            DirectoryManager::DeleteDir(QDir().rename(InstallerPath, temp) ? temp : InstallerPath);
        }
    }
#endif

    if (bInstalled) {
        AppConfig config;

        //const AppConfig installedConfig = installedApp.value(name);
        config.m_Name = appName;
        if (!updateConfig.m_RunPath.isEmpty())
            config.m_RunPath = updateConfig.m_RunPath;
        config.m_InstalledFiles = installedFiles;
        config.m_Version = updateConfig.m_Version;
        config.m_InstallCmd = updateConfig.m_InstallCmd;
        config.m_UninstallCmd = updateConfig.m_UninstallCmd;
        config.m_InstallParams = updateConfig.m_InstallParams;
        config.m_UninstallParams = updateConfig.m_UninstallParams;

        AppsConfig setting = Settings::GetInstance()->GetCurrentConfig();
        AppsConfig::AppMap* pSettingMap = setting.GetAppMap(type);
        if (pSettingMap) {
            pSettingMap->remove(config.m_Name);
            //pSettingMap->insert(appName, config);
            (*pSettingMap)[config.m_Name][config.m_Version] = config;
        }
        Settings::GetInstance()->UpdateConfig(setting);
    }

    //delete temp files
    DirectoryManager::DeleteDir(tempDir);
    QFile().remove(tempFilePath);

    UpdateAvailableSoftware();
    if (bInstalled) {
        Logger::GetInstance()->AddLog(tr("%1 installed.").arg(appName));
        CheckForUpdate();
    }
}

bool Installer::Delete(const QString& appName, eAppType type, bool force) {
    Logger::GetInstance()->AddLog(tr("Deleting %1").arg(appName));
    AppsConfig appsConfig = Settings::GetInstance()->GetCurrentConfig();
    AppsConfig::AppMap* appMap = appsConfig.GetAppMap(type);

    AppsConfig::AppMap::const_iterator appIter = appMap->find(appName);
    if (appIter == appMap->end())
        return "";

    AppsConfig::AppVersion::const_iterator iter = appIter.value().begin();
    if (iter == appIter.value().end())
        return "";

    const AppConfig& config = iter.value();

    QString installPath = GetInstallPath(type);
    if (!config.m_RunPath.isEmpty()){
        while (ProcessHelper::IsProcessRuning(installPath + config.m_RunPath)) {
            Logger::GetInstance()->AddLog(tr("%1 app is running.").arg(appName));
            if(force)
            {
                if (1 == QMessageBox::information(NULL,//this,
                                                 tr("Error"),
                                                 tr("%1 app is running please close it.").arg(appName),
                                                 tr("Retry"))) {
                    Logger::GetInstance()->AddLog(tr("Deleting %1 canceled").arg(appName));
                    return false;
                }
            }
            else
            {
                if (1 == QMessageBox::information(NULL,//this,
                                                 tr("Error"),
                                                 tr("%1 app is running please close it.").arg(appName),
                                                 tr("Retry"),
                                                 tr("Cancel"))) {
                    Logger::GetInstance()->AddLog(tr("Deleting %1 canceled").arg(appName));
                    return false;
                }
            }
        }
    }

    bool result = false;
#ifdef Q_OS_WIN
    if (!config.m_InstallCmd.isEmpty() || !config.m_UninstallCmd.isEmpty()) {
        QString path = GetInstallPath(type) + APP_INSTALLER_DIR + "/" + appName + "/";
        if (!config.m_UninstallCmd.isEmpty())
            path += config.m_UninstallCmd;
        else
            path += config.m_InstallCmd;

        QString params = config.m_UninstallParams;

        int nExitCode = RunAndWaitForFinish(path, params);
        if (nExitCode == config.m_nSuccessInstallCode)
            result = true;
    } else {
#endif
        result = true;
        for (int i = 0; i < config.m_InstalledFiles.size(); ++i) {
            QString path = installPath + config.m_InstalledFiles.at(i);

            QFile file(path);
            if (file.exists()) {
                QFileInfo info(file);
                if (info.isDir()) {
                    result &= DirectoryManager::DeleteDir(path);
                } else {
                    result &= QFile().remove(path);
                }
            }
        }
#ifdef Q_OS_WIN
    }
#endif

    if (!result) {
        Logger::GetInstance()->AddLog(tr("Error delete app"));
        return false;
    }

    Logger::GetInstance()->AddLog(tr("%1 deleted").arg(appName));
    appMap->remove(appName);
    Settings::GetInstance()->UpdateConfig(appsConfig);

    UpdateAvailableSoftware();
    return true;
}

QString Installer::GetRunPath(const QString& appName, eAppType type) {
    AppsConfig appsConfig = Settings::GetInstance()->GetCurrentConfig();
    AppsConfig::AppMap* appMap = appsConfig.GetAppMap(type);
    if (!appMap) {
        return "";
    }

    AppsConfig::AppMap::const_iterator appIter = appMap->find(appName);
    if (appIter == appMap->end())
        return "";

    AppsConfig::AppVersion::const_iterator iter = appIter.value().begin();
    if (iter == appIter.value().end())
        return "";

    const AppConfig& config = iter.value();
    return GetInstallPath(type) + config.m_RunPath;
}

bool Installer::Update(AvailableSoftWare::SoftWareMap softMap, eAppType type, bool force /*= false*/) {
    AvailableSoftWare::SoftWareMap::const_iterator iter;
    for (iter = softMap.begin(); iter != softMap.end(); ++iter) {
        const QString& name = iter.key();
        const SoftWare& soft = iter.value();
        if (!soft.m_CurVersion.isEmpty() &&
            Settings::GetVersion(soft.m_CurVersion) != Settings::GetVersion(soft.m_NewVersion)) {
            if (force ||
                0 == QMessageBox::information(NULL,//this,
                                             tr("Update available"),
                                             tr("%1 update available.").arg(name),
                                             tr("Install"),
                                             tr("Cancel"))) {
                if (type != eAppTypeDependencies && !Delete(name, type, force)) {
                    Logger::GetInstance()->AddLog(tr("Error update %1"));
                    return false;
                }
                Install(name, soft.m_NewVersion, type);
            }
        }
    }
    return true;
}

#ifdef Q_OS_WIN
int Installer::RunAndWaitForFinish(const QString& fileName, const QString &params) {
    wchar_t *fileNameW = new wchar_t[fileName.length() + 1];
    fileNameW[fileName.toWCharArray(fileNameW)] = 0;

    QFileInfo aFileInfo(fileName);
    QString filePath = aFileInfo.absolutePath();
    wchar_t *filePathW = new wchar_t[filePath.length() + 1];
    filePathW[filePath.toWCharArray(filePathW)] = 0;

    wchar_t *fileParamsW = new wchar_t[params.length() + 1];
    fileParamsW[params.toWCharArray(fileParamsW)] = 0;

    SHELLEXECUTEINFO executeInfo;
    ZeroMemory(&executeInfo, sizeof(SHELLEXECUTEINFO));
    executeInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    executeInfo.lpFile = fileNameW;
    executeInfo.lpParameters = fileParamsW;
    executeInfo.lpDirectory = filePathW;
    executeInfo.lpVerb = L"open";
    executeInfo.nShow = SW_SHOWNORMAL;
    executeInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    ShellExecuteEx(&executeInfo);
    WaitForSingleObject(executeInfo.hProcess, INFINITE);

    DWORD exitCode = 0;
    //GetExitCodeProcess(executeInfo.hProcess, &exitCode);

    delete fileNameW;
    delete filePathW;
    delete fileParamsW;
    return exitCode;
}
#endif
