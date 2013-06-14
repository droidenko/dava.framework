/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "QtUtils.h"
#include "../../SceneEditor/SceneValidator.h"

#include <QFileDialog>
#include <QMessageBox>
#include "mainwindow.h"
#include "QtMainWindowHandler.h"

#include "TexturePacker/CommandLineParser.h"
#include "../../Commands/FileCommands.h"
#include "../../Commands/CommandsManager.h"

#include "DAVAEngine.h"
using namespace DAVA;


DAVA::FilePath PathnameToDAVAStyle(const QString &convertedPathname)
{
    return FilePath((const String &)QSTRING_TO_DAVASTRING(convertedPathname));
}


DAVA::FilePath GetOpenFileName(const DAVA::String &title, const DAVA::FilePath &pathname, const DAVA::String &filter)
{
    QString filePath = QFileDialog::getOpenFileName(NULL, QString(title.c_str()), QString(pathname.GetAbsolutePathname().c_str()),
                                                    QString(filter.c_str()));
    
    QtMainWindowHandler::Instance()->RestoreDefaultFocus();

    FilePath openedPathname = PathnameToDAVAStyle(filePath);
    if(!openedPathname.IsEmpty() && !SceneValidator::Instance()->IsPathCorrectForProject(openedPathname))
    {
        //Need to Show Error
		ShowErrorDialog(String(Format("File(%s) was selected from incorect project.", openedPathname.GetAbsolutePathname().c_str())));
        openedPathname = FilePath();
    }
    
    return openedPathname;
}


DAVA::String SizeInBytesToString(DAVA::float32 size)
{
    DAVA::String retString = "";
    
    if(1000000 < size)
    {
        retString = Format("%0.2f MB", size / (1024 * 1024) );
    }
    else if(1000 < size)
    {
        retString = Format("%0.2f KB", size / 1024);
    }
    else
    {
        retString = Format("%d B", (int32)size);
    }
    
    return  retString;
}

DAVA::WideString SizeInBytesToWideString(DAVA::float32 size)
{
    return StringToWString(SizeInBytesToString(size));
}


DAVA::Image * CreateTopLevelImage(const DAVA::FilePath &imagePathname)
{
    Image *image = NULL;
    Vector<Image *> imageSet = ImageLoader::CreateFromFile(imagePathname);
    if(0 != imageSet.size())
    {
        image = SafeRetain(imageSet[0]);
		for_each(imageSet.begin(), imageSet.end(), SafeRelease<Image>);
    }
    
    return image;
}

void ShowErrorDialog(const DAVA::Set<DAVA::String> &errors)
{
    if(errors.empty())
        return;
    
    String errorMessage = String("");
    Set<String>::const_iterator endIt = errors.end();
    for(Set<String>::const_iterator it = errors.begin(); it != endIt; ++it)
    {
        errorMessage += *it + String("\n");
    }
    
    ShowErrorDialog(errorMessage);
}

void ShowErrorDialog(const DAVA::String &errorMessage)
{
	bool forceMode =    CommandLineParser::CommandIsFound(String("-force"))
					||  CommandLineParser::CommandIsFound(String("-forceclose"));
	if(!forceMode)
	{
		QMessageBox::critical(QtMainWindow::Instance(), "Error", errorMessage.c_str());
	}
}

bool IsKeyModificatorPressed(int32 key)
{
	return InputSystem::Instance()->GetKeyboard()->IsKeyPressed(key);
}

bool IsKeyModificatorsPressed()
{
	return (IsKeyModificatorPressed(DVKEY_SHIFT) || IsKeyModificatorPressed(DVKEY_CTRL) || IsKeyModificatorPressed(DVKEY_ALT));
}

QColor ColorToQColor(const DAVA::Color& color)
{
	return QColor::fromRgbF(color.r, color.g, color.b, color.a);
}

DAVA::Color QColorToColor(const QColor &qcolor)
{
	return Color(qcolor.redF(), qcolor.greenF(), qcolor.blueF(), qcolor.alphaF());
}

int ShowQuestion(const DAVA::String &header, const DAVA::String &question, int buttons, int defaultButton)
{
    int answer = QMessageBox::question(NULL, QString::fromStdString(header), QString::fromStdString(question),
                                       (QMessageBox::StandardButton)buttons, (QMessageBox::StandardButton)defaultButton);
    return answer;
}

int SaveSceneIfChanged(DAVA::Scene *scene)
{
    int answer = MB_FLAG_NO;
    
    int32 changesCount = CommandsManager::Instance()->GetUndoQueueLength(scene);
    if(changesCount)
    {
        answer = ShowQuestion("Scene was changed", "Do you want to save changes in the current scene prior to creating new one?",
                                    MB_FLAG_YES | MB_FLAG_NO | MB_FLAG_CANCEL, MB_FLAG_CANCEL);
        
        if(answer == MB_FLAG_YES)
        {
            // Execute this command directly to do not affect the Undo/Redo queue.
            CommandsManager::Instance()->ExecuteAndRelease(new CommandSaveScene(), scene);
        }
    }
    
    return answer;
}
