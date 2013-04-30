#include "SpritePackerHelper.h"
#include "Qt/DockParticleEditor/ParticlesEditorController.h"

#include "SpritesPacker.h"
#include "SceneEditor/EditorSettings.h"
#include "Qt/Scene/SceneDataManager.h"

#include <QtConcurrentRun>

#include "TexturePacker/ResourcePacker2D.h"
#include "Platform/Qt/QtLayer.h"

using namespace DAVA;

SpritePackerHelper::SpritePackerHelper()
{
	QObject::connect(&watcher, SIGNAL(finished()), this, SLOT(threadRepackAllFinished()), Qt::QueuedConnection);
}

void SpritePackerHelper::UpdateParticleSprites()
{
	FilePath projectPath = EditorSettings::Instance()->GetProjectPath();
    if(projectPath.IsEmpty())
    {
        Logger::Warning("[ParticlesEditorSpritePackerHelper::UpdateParticleSprites] Project path not set.");
        return;
    }

	Pack();
	
	Reload();
}

void SpritePackerHelper::Pack()
{
	void *pool = DAVA::QtLayer::Instance()->CreateAutoreleasePool();
	FilePath projectPath = EditorSettings::Instance()->GetProjectPath();
	FilePath inputDir = projectPath + "DataSource/Gfx/Particles/";
	FilePath outputDir = projectPath + "Data/Gfx/Particles/";

	if(!FileSystem::Instance()->IsDirectory(inputDir))
	{
		Logger::Error("[SpritePackerHelper::Pack] inputDir is not directory (%s)", inputDir.GetAbsolutePathname().c_str());
		return;
	}

	ResourcePacker2D * resourcePacker = new ResourcePacker2D();
	
	bool isChanged = resourcePacker->IsMD5ChangedDir(projectPath+"DataSource/Gfx/",inputDir,"particles.md5",true);
	
	SafeDelete(resourcePacker);
	if(!isChanged)
	{
		return;
	}
	
	SpritesPacker packer;
	packer.SetInputDir(inputDir);
	packer.SetOutputDir(outputDir);
	packer.Pack();
	DAVA::QtLayer::Instance()->ReleaseAutoreleasePool(pool);
}

void SpritePackerHelper::Reload()
{
    Map<String, Sprite *>spritesForReloading;
    
	for(int i = 0; i < SceneDataManager::Instance()->SceneCount(); ++i)
	{
		SceneData *sceneData = SceneDataManager::Instance()->SceneGet(i);
		if(NULL != sceneData)
		{
			// All the Particle Effects must be re-started after sprites are reloaded to avoid
			// issue like DF-545.
			EnumerateSpritesForReloading(sceneData, spritesForReloading);
		}
    }

    Map<String, Sprite *>::const_iterator endIt = spritesForReloading.end();
    for(Map<String, Sprite *>::const_iterator it = spritesForReloading.begin(); it != endIt; ++it)
    {
        it->second->Reload();
    }
    
	ParticlesEditorController::Instance()->RefreshSelectedNode();
}

void SpritePackerHelper::EnumerateSpritesForReloading(SceneData* sceneData, Map<String, Sprite *> &sprites)
{
    List<Entity*> particleEffects;
	sceneData->GetAllParticleEffects(particleEffects);
    
	for (auto it = particleEffects.begin(); it != particleEffects.end(); ++it)
	{
		Entity* curNode = (*it);
	    ParticleEffectComponent * effectComponent = cast_if_equal<ParticleEffectComponent*>(curNode->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
		
		if (!effectComponent)
		{
			continue;
		}
        
		effectComponent->Stop();
        
		// All the children of this Scene Node must have Emitter components.
		int32 emittersCount = curNode->GetChildrenCount();
		for (int32 i = 0; i < emittersCount; i ++)
		{
			Entity* childNode = curNode->GetChild(i);
			ParticleEmitter * emitter = GetEmitter(childNode);
            
			if (!emitter)
			{
				continue;
			}
			
            Vector<ParticleLayer*> & layers = emitter->GetLayers();
            int32 layersCount = layers.size();
            for (int il = 0; il < layersCount; ++il)
            {
                Sprite *sprite = layers[il]->GetSprite();
                sprites[sprite->GetRelativePathname().GetAbsolutePathname()] = sprite;
            }
		}
        
		effectComponent->Start();
	}
}

void SpritePackerHelper::UpdateParticleSpritesAsync()
{
	FilePath projectPath = EditorSettings::Instance()->GetProjectPath();
    if(projectPath.IsEmpty())
    {
        Logger::Warning("[ParticlesEditorSpritePackerHelper::UpdateParticleSprites] Project path not set.");
        return;
    }

	if(NULL == future)
	{
		future = new QFuture<void>;
		*future = QtConcurrent::run(this, &SpritePackerHelper::Pack);
		watcher.setFuture(*future);
	}
}

void SpritePackerHelper::threadRepackAllFinished()
{
	future = NULL;
	
	Reload();

	emit readyAll();
}
