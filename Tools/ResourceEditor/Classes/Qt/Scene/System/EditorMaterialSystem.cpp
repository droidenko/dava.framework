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

#include "EditorMaterialSystem.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/MaterialSystem.h"

EditorMaterialSystem::EditorMaterialSystem(DAVA::Scene * scene)
: DAVA::SceneSystem(scene)
{

}

EditorMaterialSystem::~EditorMaterialSystem()
{
	while(materialFeedback.size() > 0)
	{
		RemMaterial(materialFeedback.begin()->first);
	}

	while(ownedParents.size() > 0)
	{
		DAVA::NMaterial *parent = *(ownedParents.begin());
		ownedParents.erase(parent);
		SafeRelease(parent);
	}
}

DAVA::Entity* EditorMaterialSystem::GetEntity(DAVA::NMaterial* material) const
{
	DAVA::Entity *entity = NULL;

	auto it = materialFeedback.find(material);
	if(it != materialFeedback.end())
	{
		entity = it->second.entity;
	}

	return entity;
}

DAVA::RenderBatch* EditorMaterialSystem::GetRenderBatch(DAVA::NMaterial* material) const
{
	DAVA::RenderBatch *batch = NULL;

	auto it = materialFeedback.find(material);
	if(it != materialFeedback.end())
	{
		batch = it->second.batch;
	}

	return batch;
}

void EditorMaterialSystem::BuildMaterialsTree(DAVA::Map<DAVA::NMaterial*, DAVA::Set<DAVA::NMaterial *> > &in) const
{
	// init set with already owned materials
	DAVA::Set<DAVA::NMaterial *> materials = ownedParents;

	GetScene()->GetMaterialSystem()->BuildMaterialList(GetScene(), materials, DAVA::NMaterial::MATERIALTYPE_MATERIAL, false);

	DAVA::Set<DAVA::NMaterial *>::const_iterator i = materials.begin();
	DAVA::Set<DAVA::NMaterial *>::const_iterator end = materials.end();

	for(; i != end; ++i)
	{
		DAVA::NMaterial *parent = *i;

		// add parent
		in[parent];

		// add childs
		BuildInstancesList(parent, in[parent]);
	}
}

void EditorMaterialSystem::BuildInstancesList(DAVA::NMaterial* parent, DAVA::Set<DAVA::NMaterial *> &in) const
{
	if(NULL != parent)
	{
		// add childs
		for(DAVA::uint32 j = 0; j < parent->GetChildrenCount(); ++j)
		{
			DAVA::NMaterial *child = parent->GetChild(j);
			if(materialFeedback.count(child) > 0)
			{
				in.insert(child);
			}
		}
	}
}

void EditorMaterialSystem::AddEntity(DAVA::Entity * entity)
{
	DAVA::RenderObject *ro = GetRenderObject(entity);
	if(NULL != ro)
	{
		for(DAVA::uint32 i = 0; i < ro->GetRenderBatchCount(); ++i)
		{
			DAVA::RenderBatch *rb  = ro->GetRenderBatch(i);
			DAVA::NMaterial *material = rb->GetMaterial();

			if(NULL != material)
			{
				MaterialFB fb;

				fb.entity = entity;
				fb.batch = rb;

				materialFeedback[material] = fb;

				// remember parent material, if still isn't
				DAVA::NMaterial *parent = material->GetParent();
				if(NULL != parent && 0 == ownedParents.count(parent))
				{
					ownedParents.insert(parent);
					parent->Retain();
				}
			}
		}
	}
}

void EditorMaterialSystem::RemoveEntity(DAVA::Entity * entity)
{
	DAVA::RenderObject *ro = GetRenderObject(entity);
	if(NULL != ro)
	{
		for(DAVA::uint32 i = 0; i < ro->GetRenderBatchCount(); ++i)
		{
			DAVA::RenderBatch *rb = ro->GetRenderBatch(i);
			DAVA::NMaterial *material = rb->GetMaterial();

			if(NULL != material)
			{
				materialFeedback.erase(material);
			}
		}
	}
}

void EditorMaterialSystem::Update(DAVA::float32 timeElapsed)
{
	
}

void EditorMaterialSystem::Draw()
{

}

void EditorMaterialSystem::ProcessCommand(const Command2 *command, bool redo)
{

}

void EditorMaterialSystem::ProcessUIEvent(DAVA::UIEvent *event)
{

}

void EditorMaterialSystem::RemMaterial(DAVA::NMaterial *material)
{
}
