#ifndef __DAVAENGINE_COMMANDLINEPARSER_H__
#define __DAVAENGINE_COMMANDLINEPARSER_H__

#include "Base/BaseTypes.h"
#include "Base/StaticSingleton.h"

namespace DAVA
{

class CommandLineParser : public StaticSingleton<CommandLineParser>
{
public:
	CommandLineParser();
	virtual ~CommandLineParser();
	
	void SetVerbose(bool isVerbose);
	bool GetVerbose();

	void SetExtendedOutput(bool isExO);
	bool IsExtendedOutput();
	
	void SetArguments(int argc, char * argv[]);
	void SetArguments(const Vector<String> & arguments);

	void Clear();

	bool IsFlagSet(const String & s);
    String	GetParamForFlag(const String & flag);

public:
    
    static const int32 INVALID_POSITION = -1;
    
    static int32 GetCommandsCount();
    
    static String GetCommand(uint32 commandPosition);
    static String GetCommandParam(const String &command);
    
    static bool CommandIsFound(const String &command);
    static bool CheckPosition(int32 commandPosition);
    
protected:
    
    static DAVA::int32 GetCommandPosition(const DAVA::String &command);

    
    
private:
    
	Vector<String>	params;
	Vector<String>	flags;
	bool isVerbose;
	bool isExtendedOutput;
};

};

    
#endif // __DAVAENGINE_COMMANDLINEPARSER_H__