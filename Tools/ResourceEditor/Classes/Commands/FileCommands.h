#ifndef __FILE_COMMANDS_H__
#define __FILE_COMMANDS_H__

#include "Command.h"
#include "Constants.h"
#include "EditorScene.h"

class CommandOpenScene: public Command
{
public:	
	CommandOpenScene(const DAVA::FilePath &scenePathname = DAVA::FilePath());
    
protected:	
    
    virtual void Execute();
    
protected:
    
    DAVA::FilePath selectedScenePathname;
};

class CommandNewScene: public Command
{
public:	
	CommandNewScene();
    
protected:	
    
    virtual void Execute();
};

class CommandSaveScene: public Command
{
public:	
	CommandSaveScene();
    
protected:	
    
    virtual void Execute();
	void SaveParticleEmitterNodes(EditorScene* scene);
	void SaveParticleEmitterNodeRecursive(Entity* parentNode);
};

class CommandExport: public Command
{
    
public:	
	CommandExport(DAVA::ImageFileFormat fmt);
    
protected:	
    
    virtual void Execute();
    
protected:
    DAVA::ImageFileFormat format;
    
};

class CommandSaveToFolderWithChilds: public Command
{
public:
	CommandSaveToFolderWithChilds();
protected:
        virtual void Execute();
};



#endif // #ifndef __FILE_COMMANDS_H__