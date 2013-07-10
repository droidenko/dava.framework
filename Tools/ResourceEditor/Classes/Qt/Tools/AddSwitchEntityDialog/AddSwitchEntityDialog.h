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

#ifndef __RESOURCEEDITORQT__ADDSWITCHENTITYDIALOG__
#define __RESOURCEEDITORQT__ADDSWITCHENTITYDIALOG__

#include <QDialog.h>
#include "DAVAEngine.h"

class QMimeData;
class SceneEditor2;

namespace Ui
{
	class AddSwitchEntityDialog;
}

class AddSwitchEntityDialog: public QDialog,public DAVA::Singleton<AddSwitchEntityDialog>
{
	Q_OBJECT
    
public:
	explicit AddSwitchEntityDialog(QWidget* parent = 0);
	~AddSwitchEntityDialog();
	
	void SetRelativePath(const DAVA::String& rPath, bool forFirstWidget = true);

	void SetOpenDialogsDefaultPath(const DAVA::String& path);

	void GetSelectedEntities(DAVA::Entity** firstChild, DAVA::Entity** secondChild, SceneEditor2 * );

	void ErasePathWidgets();
	
protected:
	Ui::AddSwitchEntityDialog *ui;
	
};

#endif /* defined(__RESOURCEEDITORQT__ADDSWITCHENTITYDIALOG__) */