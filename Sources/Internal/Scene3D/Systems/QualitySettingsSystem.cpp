/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "Scene3D/Components/QualitySettingsComponent.h"

#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Scene.h"


namespace DAVA
{
	
void QualitySettingsSystem::EnableOption( const FastName & option, bool enabled )
{
	qualityOptions[option] = enabled;
}

bool QualitySettingsSystem::IsOptionEnabled( const FastName & option ) const
{
	if(qualityOptions.count(option) > 0)
	{
		return qualityOptions[option];
	}

	return false;
}

void QualitySettingsSystem::UpdateEntityAfterLoad(Entity *entity)
{
	if(qualityOptions.empty() || (NULL == entity)) return;

	Vector<Entity *> entitiesWithQualityComponent;
	entity->GetChildEntitiesWithComponent(entitiesWithQualityComponent, Component::QUALITY_SETTINGS_COMPONENT);

	if(entitiesWithQualityComponent.empty()) return;

	RemoveModelsByType(entitiesWithQualityComponent);
}

void QualitySettingsSystem::RemoveModelsByType( const Vector<Entity *> & models )
{
	uint32 count = (uint32)models.size();
	for(uint32 m = 0; m < count; ++m)
	{
		QualitySettingsComponent * comp = GetQualitySettingsComponent(models[m]);

		if(IsOptionEnabled(comp->GetModelType()) == false)
		{
			Entity *parent = models[m]->GetParent();
			parent->RemoveNode(models[m]);
		}
	}
}

bool QualitySettingsSystem::NeedLoadEntity(const Entity *entity)
{
    QualitySettingsComponent * comp = GetQualitySettingsComponent(entity);
    if(comp)
    {
        return IsOptionEnabled(comp->GetModelType());
    }
    
    return true;
}


}