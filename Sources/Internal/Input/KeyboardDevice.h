/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __DAVAENGINE_KEYBOARD_DEVICE_H__
#define __DAVAENGINE_KEYBOARD_DEVICE_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/Singleton.h"
#include "Base/BaseObject.h"
//#include "UI/UIControl.h"
//#include "UI/UIEvent.h"
//#include "UI/UIScreenTransition.h"
//#include "UI/UILoadingTransition.h"
//#include "UI/UIPopup.h"

/**
	\defgroup inputsystem	Input System
*/
namespace DAVA
{

const static int MAX_KEYS = 256;
    
    enum eDavaKeys 
    {
        DVKEY_UNKNOWN   =   0,
        DVKEY_ESCAPE		,
        DVKEY_BACKSPACE		,
        DVKEY_TAB			,
        DVKEY_ENTER			,
        DVKEY_SPACE			,
        
        DVKEY_SHIFT			,
        DVKEY_CTRL			,
        DVKEY_ALT			,
        
        DVKEY_LWIN			,
        DVKEY_RWIN			,
        DVKEY_APPS			,
        
        DVKEY_PAUSE			,
        DVKEY_CAPSLOCK		,
        DVKEY_NUMLOCK		,
        DVKEY_SCROLLLOCK	,
        
        DVKEY_PGUP			,
        DVKEY_PGDN			,
        DVKEY_HOME			,
        DVKEY_END			,
        DVKEY_INSERT		,
        DVKEY_DELETE		,
        
        DVKEY_LEFT			,
        DVKEY_UP			,
        DVKEY_RIGHT			,
        DVKEY_DOWN			,
        
        DVKEY_0				,
        DVKEY_1				,
        DVKEY_2				,
        DVKEY_3				,
        DVKEY_4				,
        DVKEY_5				,
        DVKEY_6				,
        DVKEY_7				,
        DVKEY_8				,
        DVKEY_9				,
        
        DVKEY_A				,
        DVKEY_B				,
        DVKEY_C				,
        DVKEY_D				,
        DVKEY_E				,
        DVKEY_F				,
        DVKEY_G				,
        DVKEY_H				,
        DVKEY_I				,
        DVKEY_J				,
        DVKEY_K				,
        DVKEY_L				,
        DVKEY_M				,
        DVKEY_N				,
        DVKEY_O				,
        DVKEY_P				,
        DVKEY_Q				,
        DVKEY_R				,
        DVKEY_S				,
        DVKEY_T				,
        DVKEY_U				,
        DVKEY_V				,
        DVKEY_W				,
        DVKEY_X				,
        DVKEY_Y				,
        DVKEY_Z				,
        
        DVKEY_GRAVE			,
        DVKEY_MINUS			,
        DVKEY_EQUALS		,
        DVKEY_BACKSLASH		,
        DVKEY_LBRACKET		,
        DVKEY_RBRACKET		,
        DVKEY_SEMICOLON		,
        DVKEY_APOSTROPHE	,
        DVKEY_COMMA			,
        DVKEY_PERIOD		,
        DVKEY_SLASH			,
        
        DVKEY_NUMPAD0		,
        DVKEY_NUMPAD1		,
        DVKEY_NUMPAD2		,
        DVKEY_NUMPAD3		,
        DVKEY_NUMPAD4		,
        DVKEY_NUMPAD5		,
        DVKEY_NUMPAD6		,
        DVKEY_NUMPAD7		,
        DVKEY_NUMPAD8		,
        DVKEY_NUMPAD9		,
        
        DVKEY_MULTIPLY		,
        DVKEY_DIVIDE		,
        DVKEY_ADD			,
        DVKEY_SUBTRACT		,
        DVKEY_DECIMAL		,
        
        DVKEY_F1			,
        DVKEY_F2			,
        DVKEY_F3			,
        DVKEY_F4			,
        DVKEY_F5			,
        DVKEY_F6			,
        DVKEY_F7			,
        DVKEY_F8			,
        DVKEY_F9			,
        DVKEY_F10			,
        DVKEY_F11			,
        DVKEY_F12			,

        DVKEY_COUNT
        
    };
    
    enum eMacOsModiferKeys 
    {
        DVMACOS_COMMAND = 0xFF,
        DVMACOS_OPTION = 0xFE,
        DVMACOS_CONTROL = 0xFD,
        DVMACOS_SHIFT = 0xFC,
        DVMACOS_CAPS_LOCK = 0xFB,
    };
    
class KeyboardDevice : public BaseObject
{
    friend class InputSystem;
        
protected:
    ~KeyboardDevice();
	/**
	 \brief Don't call this constructor!
	 */
	KeyboardDevice();
			
public:
    
    bool IsKeyPressed(int32 keyCode);
    
    int32 GetDavaKeyForSystemKey(int32 systemKeyCode);
    
    void OnKeyPressed(int32 keyCode);
    void OnKeyUnpressed(int32 keyCode);

    void OnBeforeUpdate();
    void OnAfterUpdate();
    
    void OnSystemKeyPressed(int32 systemKeyCode);
    void OnSystemKeyUnpressed(int32 systemKeyCode);

	void ClearAllKeys();

protected:
    
    void PrepareKeyTranslator();
        
    bool keyStatus[DVKEY_COUNT];//keys pressed for the current frame
    bool realKeyStatus[DVKEY_COUNT];
        
    int32 keyTranslator[MAX_KEYS];
};
};

#endif
