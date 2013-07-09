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

#include "TextureCompression/PVRConverter.h"

#include "Render/GPUFamilyDescriptor.h"

using namespace DAVA;
 
void PrintUsage()
{
    printf("Usage:\n");

    printf("\t-usage or --help to display this help\n");
    printf("\t-exo - extended output\n"); 
    printf("\t-v or --verbose - detailed output\n");

    printf("\n");
    printf("resourcepacker [src_dir] - will pack resources from src_dir\n");
}


bool CheckPosition(int32 commandPosition)
{
    if(CommandLineParser::CheckPosition(commandPosition))
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
    if(CommandLineParser::Instance()->GetVerbose())
    {
        int32 count = CommandLineParser::GetCommandsCount();
        for(int32 i = 0; i < count; ++i)
        {
            String command = CommandLineParser::GetCommand(i);
            printf("\n\t command: %s, param: %s", command.c_str(), CommandLineParser::GetCommandParam(command).c_str());
        }
        
        printf("\n\n");
        
        count = commandLine.size();
        for(int32 i = 0; i < count; ++i)
        {
            String command = commandLine[i];
            printf("\n\t command: %s", command.c_str());
        }
        
        printf("\n");
    }
    
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
    Logger::Debug("[Resource Packer Started]\n");
    Logger::Debug("[INPUT DIR] - [%s]\n", resourcePacker->inputGfxDirectory.GetAbsolutePathname().c_str());
    Logger::Debug("[OUTPUT DIR] - [%s]\n", resourcePacker->outputGfxDirectory.GetAbsolutePathname().c_str());
    Logger::Debug("[EXCLUDE DIR] - [%s]\n", resourcePacker->excludeDirectory.GetAbsolutePathname().c_str());
    
    Texture::InitializePixelFormatDescriptors();
    GPUFamilyDescriptor::SetupGPUParameters();
    
    
    eGPUFamily exportForGPU = GPU_UNKNOWN;
    if(CommandLineParser::CommandIsFound(String("-gpu")))
    {
        String gpuName = CommandLineParser::GetCommandParam(String("-gpu"));
        exportForGPU = GPUFamilyDescriptor::GetGPUByName(gpuName);
    }
    
    resourcePacker->PackResources(exportForGPU);
    elapsedTime = SystemTimer::Instance()->AbsoluteMS() - elapsedTime;
    Logger::Debug("[Resource Packer Compile Time: %0.3lf seconds]\n", (float64)elapsedTime / 1000.0);
    
    SafeDelete(resourcePacker);
}

void FrameworkDidLaunched()
{
    Logger::Instance()->SetLogLevel(Logger::LEVEL_DEBUG);

	if (Core::Instance()->IsConsoleMode())
	{
        if(     CommandLineParser::GetCommandsCount() < 2
           ||   (CommandLineParser::CommandIsFound(String("-usage")))
           ||   (CommandLineParser::CommandIsFound(String("-help")))
           )
        {
            PrintUsage();
			return;
        }
		
        if(CommandLineParser::CommandIsFound(String("-v")) || CommandLineParser::CommandIsFound(String("-verbose")))
        {
            CommandLineParser::Instance()->SetVerbose(true);
        }
        
        if(CommandLineParser::CommandIsFound(String("-exo")))
        {
            CommandLineParser::Instance()->SetExtendedOutput(true);
        }
	}

    ProcessRecourcePacker();
}


void FrameworkWillTerminate()
{
}
