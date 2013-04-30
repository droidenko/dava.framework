#ifndef __TEXTURE_OPTIONS_COMMANDS_H__
#define __TEXTURE_OPTIONS_COMMANDS_H__

#include "Command.h"
#include "Constants.h"
#include "DAVAEngine.h"

class ReloadTexturesAsCommand: public Command
{
public:	
	ReloadTexturesAsCommand(DAVA::ImageFileFormat format);

protected:	
    
    virtual void Execute();
    
protected:
    
    DAVA::ImageFileFormat fileFormat;
};


#endif // #ifndef __TEXTURE_OPTIONS_COMMANDS_H__