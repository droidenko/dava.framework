#ifndef __TOOLS_COMMANDS_H__
#define __TOOLS_COMMANDS_H__

#include "Command.h"
#include "Constants.h"

class CommandBeast: public Command
{
public:
	CommandBeast();
    
protected:
    
    virtual void Execute();
};

class CommandConvertToShadow : public Command
{
public:
	CommandConvertToShadow();

protected:
	virtual void Execute();
};


#endif // #ifndef __TOOLS_COMMANDS_H__