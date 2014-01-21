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

#ifndef __EDITOR_MATERIAL_SYSTEM_H__
#define __EDITOR_MATERIAL_SYSTEM_H__

#include "DAVAEngine.h"

class Command2;
class EditorMaterialSystem : public DAVA::SceneSystem
{
	friend class SceneEditor2;

public:
	EditorMaterialSystem(DAVA::Scene * scene);
	virtual ~EditorMaterialSystem();

	void BuildMaterialsTree(DAVA::Map<DAVA::NMaterial*, DAVA::Set<DAVA::NMaterial *> > &in) const;
	void BuildInstancesList(DAVA::NMaterial* parentMaterial, DAVA::Set<DAVA::NMaterial *> &in) const;

	DAVA::Entity* GetEntity(DAVA::NMaterial*) const;
	DAVA::RenderBatch *GetRenderBatch(DAVA::NMaterial*) const;

protected:
	virtual void AddEntity(DAVA::Entity * entity);
	virtual void RemoveEntity(DAVA::Entity * entity);

	void Update(DAVA::float32 timeElapsed);
	void Draw();

	void ProcessUIEvent(DAVA::UIEvent *event);
	void ProcessCommand(const Command2 *command, bool redo);

	void AddMaterial(DAVA::NMaterial *material, DAVA::Entity *entity, DAVA::RenderBatch *rb);
	void RemMaterial(DAVA::NMaterial *material);

private:
	struct MaterialFB
	{
		MaterialFB() : entity(NULL), batch(NULL) {}

		DAVA::Entity *entity;
		DAVA::RenderBatch *batch;
	};

	DAVA::Map<DAVA::NMaterial *, MaterialFB> materialFeedback;
	DAVA::Set<DAVA::NMaterial *> ownedParents;
};

#endif // __EDITOR_MATERIAL_SYSTEM_H__
