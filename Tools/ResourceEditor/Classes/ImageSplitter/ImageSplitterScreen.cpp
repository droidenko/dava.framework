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
        * Created by Ivan "Dizz" Petrochenko
=====================================================================================*/

#include "ImageSplitterScreen.h"

#include "SceneEditor/CommandLineTool.h"
#include "Qt/Main/QtUtils.h"
#include "ImageSplitter.h"


void ImageSplitterScreen::LoadResources()
{
    GetBackground()->SetColor(Color::White());
}

void ImageSplitterScreen::UnloadResources()
{
    
}

void ImageSplitterScreen::WillAppear()
{
    errorLog.clear();
    
    Vector<String> & commandLine = Core::Instance()->GetCommandLine();
    if(CommandLineTool::Instance()->CommandIsFound(String("-split")))
    {
        int32 inPosition = CommandLineTool::Instance()->CommandPosition(String("-file"));
        if(CommandLineTool::Instance()->CheckPosition(inPosition))
        {
            String filepathname = commandLine[inPosition + 1];
            ImageSplitter::SplitImage(filepathname, errorLog);
        }
        else
        {
            errorLog.insert(String("Incorrect params for splitting of the file"));
        }
    }
    else if(CommandLineTool::Instance()->CommandIsFound(String("-merge")))
    {
        int32 inPosition = CommandLineTool::Instance()->CommandPosition(String("-folder"));
        if(CommandLineTool::Instance()->CheckPosition(inPosition))
        {
            String folderPathname = commandLine[inPosition + 1];
            ImageSplitter::MergeImages(folderPathname, errorLog);
        }
        else
        {
            errorLog.insert(String("Incorrect params for merging of the files"));
        }
    }
}

void ImageSplitterScreen::DidAppear()
{
    ShowErrorDialog(errorLog);

    bool forceMode =    CommandLineTool::Instance()->CommandIsFound(String("-force"))
                    ||  CommandLineTool::Instance()->CommandIsFound(String("-forceclose"));
    if(forceMode || 0 == errorLog.size())
    {
        Core::Instance()->Quit();
    }
}
