//
//  main.m
//  TemplateProjectMacOS
//
//  Created by Vitaliy  Borodovsky on 3/16/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#include "DAVAEngine.h"
#include "GameCore.h"
#include "TexturePacker/ResourcePacker2D.h"
#include "TexturePacker/CommandLineParser.h"

#include "../SceneEditor/CommandLineTool.h"

#include "PVRConverter.h"

using namespace DAVA;
 
void PrintUsage()
{
    printf("Usage:\n");

    printf("\t-usage or --help to display this help\n");
    printf("\t-exo - extended output\n"); 
    printf("\t-v or --verbose - detailed output\n");

    printf("\n");
    printf("resourcepacker [src_dir] - will pack resources from src_dir\n");
    
    printf("\n");
    printf("-sceneexporter [-clean [directory]] [-export [-indir [directory]] [-outdir [directory]] [-processdir [directory]] [-processfile [directory]] [-format]\n");
    printf("\t-clean - will delete all files from Data/3d/\n"); 
    printf("\t-export - will export level to Data/3d/\n"); 
    printf("\t-indir - path for Poject/DataSource/3d/ folder \n"); 
    printf("\t-outdir - path for Poject/Data/3d/ folder\n"); 
    printf("\t-processdir - foldername from DataSource/3d/ for exporting\n"); 
    printf("\t-processfile - filename from DataSource/3d/ for exporting\n"); 
    printf("\t-format - png, pvr, dxt\n"); 
    printf("\t-force - to don't display error dialogs");
}


bool CheckPosition(int32 commandPosition)
{
    if(CommandLineTool::Instance()->CheckPosition(commandPosition))
    {
        printf("Wrong arguments\n");
        PrintUsage();

        return false;
    }
    
    return true;
}


void ProcessRecourcePacker()
{
    Vector<String> & commandLine = Core::Instance()->GetCommandLine();
    
    ResourcePacker2D * resourcePacker = new ResourcePacker2D();
    
    FilePath commandLinePath(commandLine[1]);
    commandLinePath.MakeDirectoryPathname();
    
    String lastDir = commandLinePath.GetDirectory().GetLastDirectoryName();
    FilePath outputh = commandLinePath + ("../../Data/" + lastDir + "/");
    
    resourcePacker->InitFolders(commandLinePath, outputh);
    
    if(resourcePacker->excludeDirectory.IsEmpty())
    {
        printf("[FATAL ERROR: Packer has wrong input pathname]");
        return;
    }
    
    if (resourcePacker->excludeDirectory.GetLastDirectoryName() != "DataSource")
    {
        printf("[FATAL ERROR: Packer working only inside DataSource directory]");
        return;
    }
    
    new PVRConverter();
    
    
    if(commandLine.size() < 3)
    {
        printf("[FATAL ERROR: PVRTexTool path need to be second parameter]");
        return;
    }
    
#if defined (__DAVAENGINE_MACOS__)
	String toolName = String("/PVRTexToolCL");
#elif defined (__DAVAENGINE_WIN32__)
	String toolName = String("/PVRTexToolCL.exe");
#endif
    PVRConverter::Instance()->SetPVRTexTool(resourcePacker->excludeDirectory + (commandLine[2] + toolName));
    
    uint64 elapsedTime = SystemTimer::Instance()->AbsoluteMS();
    printf("[Resource Packer Started]\n");
    printf("[INPUT DIR] - [%s]\n", resourcePacker->inputGfxDirectory.GetAbsolutePathname().c_str());
    printf("[OUTPUT DIR] - [%s]\n", resourcePacker->outputGfxDirectory.GetAbsolutePathname().c_str());
    printf("[EXCLUDE DIR] - [%s]\n", resourcePacker->excludeDirectory.GetAbsolutePathname().c_str());
    
    Texture::InitializePixelFormatDescriptors();
    resourcePacker->PackResources();
    elapsedTime = SystemTimer::Instance()->AbsoluteMS() - elapsedTime;
    printf("[Resource Packer Compile Time: %0.3lf seconds]\n", (float64)elapsedTime / 1000.0);
    
    PVRConverter::Instance()->Release();
    
    SafeDelete(resourcePacker);
}

void FrameworkDidLaunched()
{
    new CommandLineTool();
	if (Core::Instance()->IsConsoleMode())
	{
        if(     CommandLineTool::Instance()->CommandsCount() < 2 
           ||   (CommandLineTool::Instance()->CommandIsFound(String("-usage")))
           ||   (CommandLineTool::Instance()->CommandIsFound(String("-help")))
           )
        {
            PrintUsage();
			return;
        }
		
        if(CommandLineTool::Instance()->CommandIsFound(String("-v")) || CommandLineTool::Instance()->CommandIsFound(String("-verbose")))
        {
            CommandLineParser::Instance()->SetVerbose(true);
        }
        
        if(CommandLineTool::Instance()->CommandIsFound(String("-exo")))
        {
            CommandLineParser::Instance()->SetExtendedOutput(true);
        }
	}

    ProcessRecourcePacker();
}


void FrameworkWillTerminate()
{
    CommandLineTool::Instance()->Release();
}
