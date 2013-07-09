/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "Scene3D/Components/LodComponent.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"

namespace DAVA
{

const float32 LodComponent::INVALID_DISTANCE = -1.f;
const float32 LodComponent::MIN_LOD_DISTANCE = 0.f;
const float32 LodComponent::MAX_LOD_DISTANCE = 1000.f;

LodComponent::LodDistance::LodDistance()
{
	distance = nearDistance = nearDistanceSq = farDistance = farDistanceSq = (float32) INVALID_DISTANCE;
}

void LodComponent::LodDistance::SetDistance(const float32 &newDistance)
{
	distance = newDistance;
}

void LodComponent::LodDistance::SetNearDistance(const float32 &newDistance)
{
	nearDistance = newDistance;
	nearDistanceSq = nearDistance * nearDistance;
}

void LodComponent::LodDistance::SetFarDistance(const float32 &newDistance)
{
	farDistance = newDistance;
	farDistanceSq = farDistance * farDistance;
}

Component * LodComponent::Clone(Entity * toEntity)
{
	LodComponent * newLod = new LodComponent();
	newLod->SetEntity(toEntity);

	newLod->lodLayers = lodLayers;
	const Vector<LodData>::const_iterator endLod = newLod->lodLayers.end();
	for (Vector<LodData>::iterator it = newLod->lodLayers.begin(); it != endLod; ++it)
	{
		LodData & ld = *it;
		ld.nodes.clear();
	}

	//Lod values
	for(int32 iLayer = 0; iLayer < MAX_LOD_LAYERS; ++iLayer)
	{
        newLod->lodLayersArrayOriginal[iLayer] = lodLayersArrayOriginal[iLayer];
        newLod->lodLayersArrayWorking[iLayer] = lodLayersArrayWorking[iLayer];
	}

	newLod->forceDistance = forceDistance;
	newLod->forceDistanceSq = forceDistanceSq;
	newLod->forceLodLayer = forceLodLayer;

	return newLod;
}

void LodComponent::Serialize(KeyedArchive *archive, SceneFileV2 *sceneFile)
{
	Component::Serialize(archive, sceneFile);

	if(NULL != archive)
	{
		uint32 i;

		archive->SetUInt32("lc.flags", flags);
		archive->SetFloat("lc.forceDistance", forceDistance);
		archive->SetInt32("lc.forceLodLayer", forceLodLayer);

		KeyedArchive *lodDistArch = new KeyedArchive();
		for (i = 0; i < MAX_LOD_LAYERS; ++i)
		{
			KeyedArchive *lodDistValuesArch = new KeyedArchive();
			lodDistValuesArch->SetFloat("ld.distance", lodLayersArrayOriginal[i].distance);
			lodDistValuesArch->SetFloat("ld.neardist", lodLayersArrayOriginal[i].nearDistance);
			lodDistValuesArch->SetFloat("ld.fardist", lodLayersArrayOriginal[i].farDistance);

			lodDistArch->SetArchive(KeyedArchive::GenKeyFromIndex(i), lodDistValuesArch);
			lodDistValuesArch->Release();
		}
		archive->SetArchive("lc.loddist", lodDistArch);
		lodDistArch->Release();

		i = 0;
		KeyedArchive *lodDataArch = new KeyedArchive();
		Vector<LodData>::iterator it = lodLayers.begin();
		for(; it != lodLayers.end(); ++it)
		{
			KeyedArchive *lodDataValuesArch = new KeyedArchive();
			KeyedArchive *lodDataIndexesArch = new KeyedArchive();

			for(uint32 j = 0; j < it->indexes.size(); ++j)
			{
				lodDataIndexesArch->SetInt32(KeyedArchive::GenKeyFromIndex(j), it->indexes[j]);
			}

			lodDataValuesArch->SetArchive("indexes", lodDataIndexesArch);
			lodDataValuesArch->SetInt32("layer", it->layer);
			lodDataValuesArch->SetBool("isdummy", it->isDummy);
			lodDataValuesArch->SetUInt32("indexescount", it->indexes.size());

			lodDataArch->SetArchive(KeyedArchive::GenKeyFromIndex(i), lodDataValuesArch);

			lodDataIndexesArch->Release();
			lodDataValuesArch->Release();
			++i;
		}
		archive->SetUInt32("lc.loddatacount", lodLayers.size());
		archive->SetArchive("lc.loddata", lodDataArch);
		lodDataArch->Release();
	}
}

void LodComponent::Deserialize(KeyedArchive *archive, SceneFileV2 *sceneFile)
{
	if(NULL != archive)
	{
        flags = archive->GetUInt32("lc.flags", 0);
        
        forceDistance = archive->GetFloat("lc.forceDistance", (float32)INVALID_DISTANCE);
        forceDistanceSq = forceDistance * forceDistance;
        forceLodLayer = archive->GetInt32("lc.forceLodLayer", INVALID_LOD_LAYER);

        KeyedArchive *lodDistArch = archive->GetArchive("lc.loddist");
		if(NULL != lodDistArch)
		{
			for(uint32 i = 0; i < MAX_LOD_LAYERS; ++i)
			{
				KeyedArchive *lodDistValuesArch = lodDistArch->GetArchive(KeyedArchive::GenKeyFromIndex(i));
				if(NULL != lodDistValuesArch)
				{
					lodLayersArrayOriginal[i].distance = lodDistValuesArch->GetFloat("ld.distance");
					lodLayersArrayOriginal[i].nearDistance = lodDistValuesArch->GetFloat("ld.neardist");
					lodLayersArrayOriginal[i].farDistance = lodDistValuesArch->GetFloat("ld.fardist");

					lodLayersArrayOriginal[i].nearDistanceSq = lodLayersArrayOriginal[i].nearDistance * lodLayersArrayOriginal[i].nearDistance;
					lodLayersArrayOriginal[i].farDistanceSq = lodLayersArrayOriginal[i].farDistance * lodLayersArrayOriginal[i].farDistance;
				}
			}
            
            RecalcWorkingDistances();
		}

		KeyedArchive *lodDataArch = archive->GetArchive("lc.loddata");
		if(NULL != lodDataArch)
		{
            uint32 lodDataCount = archive->GetUInt32("lc.loddatacount");
			for(uint32 i = 0; i < lodDataCount; ++i)
			{
				KeyedArchive *lodDataValuesArch = lodDataArch->GetArchive(KeyedArchive::GenKeyFromIndex(i));
				if(NULL != lodDataValuesArch)
				{
					LodData data;

					data.layer = lodDataValuesArch->GetInt32("layer", INVALID_LOD_LAYER);
                    data.isDummy = lodDataValuesArch->GetBool("isdummy", false);

					KeyedArchive *lodDataIndexesArch = lodDataValuesArch->GetArchive("indexes");
					if(NULL != lodDataIndexesArch)
					{
                        uint32 indexesCount = lodDataValuesArch->GetUInt32("indexescount");
						for(uint32 j = 0; j < indexesCount; ++j)
						{
							data.indexes.push_back(lodDataIndexesArch->GetInt32(KeyedArchive::GenKeyFromIndex(j)));
						}
					}

					lodLayers.push_back(data);
				}
			}
		}
	}

	flags |= NEED_UPDATE_AFTER_LOAD;
	Component::Deserialize(archive, sceneFile);
}

LodComponent::LodComponent()
:	forceLodLayer(INVALID_LOD_LAYER),
	forceDistance(INVALID_DISTANCE),
	forceDistanceSq(INVALID_DISTANCE)
{
    lodLayersArrayOriginal.resize(MAX_LOD_LAYERS);
	lodLayersArrayWorking.resize(MAX_LOD_LAYERS);

	for(int32 iLayer = 0; iLayer < MAX_LOD_LAYERS; ++iLayer)
	{
		lodLayersArrayOriginal[iLayer].SetDistance(GetDefaultDistance(iLayer));
		lodLayersArrayOriginal[iLayer].SetFarDistance(MAX_LOD_DISTANCE * 2);
        
        lodLayersArrayWorking[iLayer] = lodLayersArrayOriginal[iLayer];
	}

	lodLayersArrayOriginal[0].SetNearDistance(0.0f);
	lodLayersArrayWorking[0].SetNearDistance(0.0f);
    
    flags = NEED_UPDATE_AFTER_LOAD;
}

float32 LodComponent::GetDefaultDistance(int32 layer)
{
	float32 distance = MIN_LOD_DISTANCE + ((float32)(MAX_LOD_DISTANCE - MIN_LOD_DISTANCE) / (MAX_LOD_LAYERS-1)) * layer;
	return distance;
}

void LodComponent::SetCurrentLod(LodData *newLod)
{
	if (newLod != currentLod) 
	{
		if (currentLod) 
		{
			int32 size = currentLod->nodes.size();
			for (int i = 0; i < size; i++) 
			{
				currentLod->nodes[i]->SetLodVisible(false);
			}
		}
		currentLod = newLod;
		int32 size = currentLod->nodes.size();
		for (int i = 0; i < size; i++) 
		{
			currentLod->nodes[i]->SetLodVisible(true);
		}
	}
}

void LodComponent::SetForceDistance(const float32 &newDistance)
{
    forceDistance = newDistance;
    forceDistanceSq = forceDistance * forceDistance;
}
    
float32 LodComponent::GetForceDistance() const
{
    return forceDistance;
}

void LodComponent::GetLodData(Vector<LodData*> &retLodLayers)
{
	retLodLayers.clear();

	Vector<LodData>::const_iterator endIt = lodLayers.end();
	for(Vector<LodData>::iterator it = lodLayers.begin(); it != endIt; ++it)
	{
		LodData *ld = &(*it);
		retLodLayers.push_back(ld);
	}
}
    
void LodComponent::SetLodLayerDistance(int32 layerNum, float32 distance)
{
    DVASSERT(0 <= layerNum && layerNum < MAX_LOD_LAYERS);
    
    if(INVALID_DISTANCE != distance)
    {
        float32 nearDistance = distance * 0.95f;
        float32 farDistance = distance * 1.05f;
        
        if(GetLodLayersCount() - 1 == layerNum)
        {
            lodLayersArrayOriginal[layerNum].SetFarDistance(MAX_LOD_DISTANCE * 2);
        }
        if(layerNum)
        {
            lodLayersArrayOriginal[layerNum-1].SetFarDistance(farDistance);
        }
        
        lodLayersArrayOriginal[layerNum].SetDistance(distance);
        lodLayersArrayOriginal[layerNum].SetNearDistance(nearDistance);
    }
    else 
    {
        lodLayersArrayOriginal[layerNum].SetDistance(distance);
    }
    
    RecalcWorkingDistance(layerNum);
}

void LodComponent::SetForceLodLayer(int32 layer)
{
    forceLodLayer = layer;
}
    
int32 LodComponent::GetForceLodLayer()
{
    return forceLodLayer;
}

int32 LodComponent::GetMaxLodLayer()
{
	int32 ret = INVALID_LOD_LAYER;
	const Vector<LodData>::const_iterator &end = lodLayers.end();
	for (Vector<LodData>::iterator it = lodLayers.begin(); it != end; ++it)
	{
		LodData & ld = *it;
		if(ld.layer > ret)
		{
			ret = ld.layer;
		}
	}

	return ret;
}

void LodComponent::RecalcWorkingDistances()
{
    for(uint32 i = 0; i < lodLayersArrayWorking.size(); ++i)
    {
        RecalcWorkingDistance(i);
    }
}

void LodComponent::RecalcWorkingDistance(int32 forLayer)
{
    DVASSERT(0 <= forLayer && forLayer < MAX_LOD_LAYERS);
    
    float32 persentage = GetPersentage(forLayer);
    
    lodLayersArrayWorking[forLayer].distance = RecalcDistance(lodLayersArrayOriginal[forLayer].distance, persentage);
    lodLayersArrayWorking[forLayer].nearDistance = RecalcDistance(lodLayersArrayOriginal[forLayer].nearDistance, persentage);
    lodLayersArrayWorking[forLayer].farDistance = RecalcDistance(lodLayersArrayOriginal[forLayer].farDistance, persentage);
    
    lodLayersArrayWorking[forLayer].nearDistanceSq = lodLayersArrayWorking[forLayer].nearDistance * lodLayersArrayWorking[forLayer].nearDistance;
    lodLayersArrayWorking[forLayer].farDistanceSq = lodLayersArrayWorking[forLayer].farDistance * lodLayersArrayWorking[forLayer].farDistance;
    
    flags |= NEED_UPDATE_AFTER_LOAD;
}
    
float32 LodComponent::RecalcDistance(float32 originalDistance, float32 persentage)
{
    if(originalDistance == INVALID_DISTANCE)
        return INVALID_DISTANCE;
    
    return originalDistance * persentage;
}

float32 LodComponent::GetPersentage(uint32 forLayer)
{
    if(entity)
    {
        Scene *scene = entity->GetScene();
        if(scene)
        {
            const Vector<float32> &shifts = scene->GetLodLayersCorrection();
            DVASSERT(forLayer < shifts.size())
            
            return (1.f + shifts[forLayer]); //TODO: need real multiplier from scene
        }
    }
    
    return 1.f;
}

    
};
