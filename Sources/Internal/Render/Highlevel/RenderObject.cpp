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


#include "Render/Highlevel/RenderObject.h"
#include "Base/ObjectFactory.h"
#include "Debug/DVAssert.h"
#include "Utils/Utils.h"

namespace DAVA
{


static const int32 DEFAULT_FLAGS = RenderObject::VISIBLE | RenderObject::VISIBLE_LOD | RenderObject::VISIBLE_SWITCH | RenderObject::VISIBLE_STATIC_OCCLUSION;

RenderObject::RenderObject()
    :   type(TYPE_RENDEROBJECT)
    ,   flags(DEFAULT_FLAGS)
    ,   removeIndex(-1)
	,   treeNodeIndex(INVALID_TREE_NODE_INDEX)
    ,   staticOcclusionIndex(INVALID_STATIC_OCCLUSION_INDEX)
	,   startClippingPlane(0)
    ,   debugFlags(0)
    ,   worldTransform(0)
	,	renderSystem(0)
{
}
    
RenderObject::~RenderObject()
{
	uint32 size = renderBatchArray.size();
	for(uint32 i = 0; i < size; ++i)
	{
		renderBatchArray[i]->Release();
	}
}
  
void RenderObject::AddRenderBatch(RenderBatch * batch)
{
	batch->Retain();
	batch->SetRenderObject(this);
    renderBatchArray.push_back(batch);
    //batch->AttachToRenderSystem(renderSystem);
    if (renderSystem)
        renderSystem->RegisterBatch(batch);
    
    const AABBox3 & boundingBox = batch->GetBoundingBox();
//    DVASSERT(boundingBox.min.x != AABBOX_INFINITY &&
//             boundingBox.min.y != AABBOX_INFINITY &&
//             boundingBox.min.z != AABBOX_INFINITY);
    
    bbox.AddAABBox(boundingBox);
}

void RenderObject::RemoveRenderBatch(RenderBatch * batch)
{
    if (renderSystem)
        renderSystem->UnregisterBatch(batch);
    
    batch->SetRenderObject(0);
	batch->Release();

    FindAndRemoveExchangingWithLast(renderBatchArray, batch);
    RecalcBoundingBox();
}
    
void RenderObject::RecalcBoundingBox()
{
    bbox = AABBox3();
    
    uint32 size = (uint32)renderBatchArray.size();
    for (uint32 k = 0; k < size; ++k)
    {
        bbox.AddAABBox(renderBatchArray[k]->GetBoundingBox());
    }
}
    
RenderObject * RenderObject::Clone(RenderObject *newObject)
{
	if(!newObject)
	{
		DVASSERT_MSG(IsPointerToExactClass<RenderObject>(this), "Can clone only RenderObject");
		newObject = new RenderObject();
	}

	newObject->type = type;
	newObject->flags = flags;
	newObject->RemoveFlag(MARKED_FOR_UPDATE);
	newObject->debugFlags = debugFlags;
    newObject->staticOcclusionIndex = staticOcclusionIndex;
	//ro->bbox = bbox;
	//ro->worldBBox = worldBBox;

	//TODO:VK: Do we need remove all renderbatches from newObject?
	DVASSERT(newObject->GetRenderBatchCount() == 0);

	uint32 size = GetRenderBatchCount();
    newObject->renderBatchArray.reserve(size);
	for(uint32 i = 0; i < size; ++i)
	{
        RenderBatch *batch = GetRenderBatch(i)->Clone();
		newObject->AddRenderBatch(batch);
        batch->Release();
	}
    newObject->ownerDebugInfo = ownerDebugInfo;

	return newObject;
}

void RenderObject::Save(KeyedArchive * archive, SerializationContext* serializationContext)
{
	AnimatedObject::Save(archive);

	if(NULL != archive)
	{
		archive->SetUInt32("ro.type", type);
		//archive->SetUInt32("ro.flags", flags);
		archive->SetUInt32("ro.debugflags", debugFlags);
		archive->SetUInt32("ro.batchCount", GetRenderBatchCount());
        archive->SetUInt32("ro.sOclIndex", staticOcclusionIndex);

		KeyedArchive *batchesArch = new KeyedArchive();
		for(uint32 i = 0; i < GetRenderBatchCount(); ++i)
		{
			RenderBatch *batch = GetRenderBatch(i);
			if(NULL != batch)
			{
				KeyedArchive *batchArch = new KeyedArchive();
				batch->Save(batchArch, serializationContext);
				if(batchArch->Count() > 0)
				{
					batchArch->SetString("rb.classname", batch->GetClassName());
				}
				batchesArch->SetArchive(KeyedArchive::GenKeyFromIndex(i), batchArch);
				batchArch->Release();
			} 
		}

		archive->SetArchive("ro.batches", batchesArch);
		batchesArch->Release();
	}
}

void RenderObject::Load(KeyedArchive * archive, SerializationContext *serializationContext)
{
	if(NULL != archive)
	{
		type = archive->GetUInt32("ro.type", TYPE_RENDEROBJECT);
		debugFlags = archive->GetUInt32("ro.debugflags", 0);
        staticOcclusionIndex = (uint16)archive->GetUInt32("ro.sOclIndex", INVALID_STATIC_OCCLUSION_INDEX);

		uint32 roBatchCount = archive->GetUInt32("ro.batchCount");
        KeyedArchive *batchesArch = archive->GetArchive("ro.batches");
        for(uint32 i = 0; i < roBatchCount; ++i)
			{
				KeyedArchive *batchArch = batchesArch->GetArchive(KeyedArchive::GenKeyFromIndex(i));
				if(NULL != batchArch)
				{
					RenderBatch *batch = ObjectFactory::Instance()->New<RenderBatch>(batchArch->GetString("rb.classname"));
					
					if(NULL != batch)
					{
						batch->Load(batchArch, serializationContext);
						AddRenderBatch(batch);
						batch->Release();
					}
				}
			}
		}

	AnimatedObject::Load(archive);
}

void RenderObject::SetRenderSystem(RenderSystem * _renderSystem)
{
	renderSystem = _renderSystem;
}

RenderSystem * RenderObject::GetRenderSystem()
{
	return renderSystem;
}

void RenderObject::BakeTransform(const Matrix4 & /*transform*/)
{
}

void RenderObject::RecalculateWorldBoundingBox()
{
	DVASSERT(!bbox.IsEmpty());
	bbox.GetTransformedBox(*worldTransform, worldBBox);
}

};
