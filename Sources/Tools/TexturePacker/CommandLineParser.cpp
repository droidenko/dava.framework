#include <stdlib.h>
#include "CommandLineParser.h"
#include "Core/Core.h"

#include <cerrno>

namespace DAVA
{
    
CommandLineParser::CommandLineParser()
{
	isVerbose = false;
}

void CommandLineParser::SetArguments(const Vector<String> &arguments)
{
    Clear();
    
    bool prevIsFlag = false;
	for (int i = 0; i < (int)arguments.size(); ++i)
	{
		String arg = arguments[i];
        String::size_type argLen = arg.length();
        
		if ((argLen >= 1) && (arg[0] == '-'))
        {
            if(prevIsFlag)
                params.push_back(String());
            
			flags.push_back(arg);
            prevIsFlag = true;
        }
		else
        {
            params.push_back(arg);
            prevIsFlag = false;
        }
	}
    
    if(prevIsFlag)
        params.push_back(String());
}

    
void CommandLineParser::SetArguments(int argc, char * argv[])
{
    Clear();

    bool prevIsFlag = false;
	for (int i = 0; i < argc; ++i)
	{
		char * arg = argv[i];
		size_t argLen = strlen(arg);
        
		if ((argLen >= 1) && (arg[0] == '-'))
        {
            if(prevIsFlag)
                params.push_back(String());
            
			flags.push_back(String(arg));
            prevIsFlag = true;
        }
		else
        {
            params.push_back(String(arg));
            prevIsFlag = false;
        }
	}
    
    if(prevIsFlag)
        params.push_back(String());
}

void CommandLineParser::Clear()
{	
	flags.clear();
    params.clear();
}


void CommandLineParser::SetVerbose(bool _isVerbose)
{
	isVerbose = _isVerbose;
}

bool CommandLineParser::GetVerbose()
{
	return isVerbose;
}

void CommandLineParser::SetExtendedOutput(bool isExO)
{
	isExtendedOutput = isExO;
}
bool CommandLineParser::IsExtendedOutput()
{
	return isExtendedOutput;
}


CommandLineParser::~CommandLineParser()
{


}

bool CommandLineParser::IsFlagSet(const String & s)
{
	for (uint32 k = 0; k < flags.size(); ++k)
		if (flags[k] == s)return true;
	return false;
}

    
String	CommandLineParser::GetParamForFlag(const String & flag)
{
    DVASSERT(flags.size() == params.size());
    
	for (uint32 k = 0; k < flags.size(); ++k)
    {
		if (flags[k] == flag)
        {
            return params[k];
        }
    }

	return String();
}
    

bool CommandLineParser::CommandIsFound(const DAVA::String &command)
{
    return (INVALID_POSITION != GetCommandPosition(command));
}

DAVA::String CommandLineParser::GetCommand(DAVA::uint32 commandPosition)
{
    Vector<String> & commandLine = Core::Instance()->GetCommandLine();
    DVASSERT(commandPosition < commandLine.size());
    
    return commandLine[commandPosition];
}


DAVA::int32 CommandLineParser::GetCommandPosition(const DAVA::String &command)
{
    int32 position = INVALID_POSITION;
    
    Vector<String> & commandLine = Core::Instance()->GetCommandLine();
    for(int32 i = 0; i < (int32)commandLine.size(); ++i)
    {
        if(command == commandLine[i])
        {
            position = i;
            break;
        }
    }
    
    return position;
}

DAVA::String CommandLineParser::GetCommandParam(const DAVA::String &command)
{
    int32 commandPosition = GetCommandPosition(command);
    if(CheckPosition(commandPosition))
    {
        return GetCommand(commandPosition + 1);
    }
    
    return String();
}


int32 CommandLineParser::GetCommandsCount()
{
    Vector<String> & commandLine = Core::Instance()->GetCommandLine();
    return commandLine.size();
}

bool CommandLineParser::CheckPosition(int32 commandPosition)
{
    if(     (INVALID_POSITION == commandPosition)
       ||   (GetCommandsCount() < commandPosition + 2))
    {
        return false;
    }
    
    return true;
}


};
