#ifndef __COMMAND_CREATE_NODE_H__
#define __COMMAND_CREATE_NODE_H__

#include "Command.h"
#include "Constants.h"

class CommandCreateNode: public Command
{
public:	
	CommandCreateNode(ResourceEditor::eNodeType type);
    
protected:
    virtual void Execute();

protected:
    ResourceEditor::eNodeType nodeType;
};




#endif // #ifndef __COMMAND_CREATE_NODE_H__