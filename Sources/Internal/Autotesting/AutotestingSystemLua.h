/*==================================================================================
    Copyright (c) 2012, DAVA Consulting, LLC
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
        * Created by Dmitry Shpakov 
=====================================================================================*/
#ifndef __DAVAENGINE_AUTOTESTING_SYSTEM_LUA_H__
#define __DAVAENGINE_AUTOTESTING_SYSTEM_LUA_H__

#include "DAVAConfig.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Math/Vector.h"
#include "Base/Singleton.h"
#include "UI/UIControl.h"

#include "FileSystem/LocalizationSystem.h"

struct lua_State;

namespace DAVA
{
    
#if !defined(SWIG)
class AutotestingSystemLuaDelegate
{
public:
    virtual bool LoadWrappedLuaObjects(lua_State *luaState) = 0;
};
#endif //SWIG

class AutotestingSystemLua : public Singleton<AutotestingSystemLua>
{
public:
    AutotestingSystemLua();
    ~AutotestingSystemLua();
    
#if !defined(SWIG)
    void SetDelegate(AutotestingSystemLuaDelegate *_delegate);

    void InitFromFile(const String &luaFilePath);
    
    void StartTest();

    void Update(float32 timeElapsed);
#endif //SWIG
    
    // autotesting system api
    void WaitForMaster();
    void WaitForHelpers(DAVA::int32 helpersCount);
    
    void OnError(const String &errorMessage);
    void OnTestStep(const String &stepName, bool isPassed, const String &error = "");
    void OnTestFinished();
    
    float32 GetTimeElapsed();
    
	// Test organization API
	void OnTestStart(const String &testName);

	void OnStepStart(const String &stepName);

	void Log(const String &level, const String &message);
	
    // autotesting api
    UIControl *GetScreen();
    UIControl *FindControl(const String &path);
    void TouchDown(const Vector2 &point, int32 touchId);
    void TouchMove(const Vector2 &point, int32 touchId);
    void TouchUp(int32 touchId);
    
	// Keyboard action
	void KeyPress(int32 keyChar);

    // helpers
    bool SetText(const String &path, const String &text); // lua uses ansi strings
    bool CheckText(UIControl *control, const String &expectedText);
    bool CheckMsgText(UIControl *control, const String &key);

	// multiplayer api
	void WriteState(const String & device, const String & state);
	void WriteCommand(const String & device, const String & state);

	String ReadState(const String & device);
	String ReadCommand(const String & device);

	void InitializeDevice(const String & device);

	// DB storing
	bool SaveKeyedArchiveToDB(const String &archiveName, KeyedArchive *archive, const String &docName = "aux");

	void WriteString(const String & name, const String & text);
	String ReadString(const String & name);

	String MakeScreenshot();

protected:
#if !defined(SWIG)
    void ParsePath(const String &path, Vector<String> &parsedPath);
    
    bool LoadScript(const String &luaScript);
    bool LoadScriptFromFile(const String &luaFilePath);
    bool RunScript();
    
    bool RunScript(const String &luaScript);
    bool RunScriptFromFile(const String &luaFilePath);
    bool LoadWrappedLuaObjects();
	

	AutotestingSystemLuaDelegate *delegate;
    lua_State *luaState; //TODO: multiple lua states
    
    //TODO: write a copy of localization system for autotesting
    LocalizationSystem *autotestingLocalizationSystem;
    
#endif //SWIG
};

};

#endif //__DAVAENGINE_AUTOTESTING__

#endif //__DAVAENGINE_AUTOTESTING_SYSTEM_LUA_H__