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

#ifndef __DAVAENGINE_LOD_COMPONENT_H__
#define __DAVAENGINE_LOD_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Debug/DVAssert.h"

namespace DAVA
{

class Entity;
class LodComponent : public Component
{
public:
	IMPLEMENT_COMPONENT_TYPE(LOD_COMPONENT);

	static const int32 MAX_LOD_LAYERS = 4;
	static const int32 INVALID_LOD_LAYER = -1;
	static const float32 MIN_LOD_DISTANCE;
	static const float32 MAX_LOD_DISTANCE;
	static const float32 INVALID_DISTANCE;

	enum eFlags
	{
		NEED_UPDATE_AFTER_LOAD = 1 << 0,
	};

	struct LodDistance
	{
		float32 distance;

		float32 nearDistance;
		float32 farDistance;

		float32 nearDistanceSq;
		float32 farDistanceSq;

		LodDistance();
		void SetDistance(const float32 &newDistance);
        float32 GetDistance() const { return distance; };
        
		void SetNearDistance(const float32 &newDistance);
        float32 GetNearDistance() const {return  nearDistance; };
        
		void SetFarDistance(const float32 &newDistance);
        float32 GetFarDistance() const {return farDistance; };
        
        INTROSPECTION(LodDistance,
            PROPERTY("distance", "Distance", GetDistance, SetDistance, I_SAVE | I_VIEW | I_EDIT)
            PROPERTY("nearDistance", "Near Distance", GetNearDistance, SetNearDistance, I_EDIT)
            PROPERTY("farDistance", "Far Distance", GetFarDistance, SetFarDistance, I_EDIT)
        );
	};

	struct LodData
	{
		LodData()
		:	layer(INVALID_LOD_LAYER),
			isDummy(false)
		{ }

		Vector<Entity*> nodes;
		Vector<int32> indexes;
		int32 layer;
		bool isDummy;
	};

	LodComponent();
	virtual Component * Clone(Entity * toEntity);
	virtual void Serialize(KeyedArchive *archive, SceneFileV2 *sceneFile);
	virtual void Deserialize(KeyedArchive *archive, SceneFileV2 *sceneFile);

	static float32 GetDefaultDistance(int32 layer);
	void SetCurrentLod(LodData *newLod);

	inline int32 GetLodLayersCount();
	inline float32 GetLodLayerDistance(int32 layerNum);
	inline float32 GetLodLayerDistanceOriginal(int32 layerNum);
	inline float32 GetLodLayerNear(int32 layerNum);
	inline float32 GetLodLayerFar(int32 layerNum);
	inline float32 GetLodLayerNearSquare(int32 layerNum);
	inline float32 GetLodLayerFarSquare(int32 layerNum);

	void GetLodData(Vector<LodData*> &retLodLayers);

	LodData *currentLod;
	Vector<LodData> lodLayers;
	Vector<LodDistance> lodLayersArrayOriginal; // stored at *.sc2 data
	Vector<LodDistance> lodLayersArrayWorking; // lodLayersArrayOriginal + Scene lod correction
	int32 forceLodLayer;

    void SetForceDistance(const float32 &newDistance);
    float32 GetForceDistance() const;
	float32 forceDistance;
	float32 forceDistanceSq;

	int32 flags;
    
    /**
         \brief Registers LOD layer into the LodComponent.
         \param[in] layerNum is the layer index
         \param[in] distance near view distance for the layer
	 */
    void SetLodLayerDistance(int32 layerNum, float32 distance);

    
    /**
         \brief Sets lod layer thet would be forcely used in the whole scene.
         \param[in] layer layer to set on the for the scene. Use -1 to disable forced lod layer.
	 */
    void SetForceLodLayer(int32 layer);
    int32 GetForceLodLayer();

	int32 GetMaxLodLayer();

    void RecalcWorkingDistances();

protected:
    
    void RecalcWorkingDistance(int32 forLayer);
    float32 RecalcDistance(float32 originalDistance, float32 persentage);
    float32 GetPersentage(uint32 forLayer);
    
    void SetDistanceToArray(Vector<LodDistance> &lodArray, float32 distance, float32 forLayer);
    
public:
    
    INTROSPECTION_EXTEND(LodComponent, Component,
        COLLECTION(lodLayersArrayWorking, "Lod Layers Array. Current Values", I_SAVE | I_VIEW | I_EDIT)
        COLLECTION(lodLayersArrayOriginal, "Lod Layers Array. Original Values", I_SAVE | I_VIEW | I_EDIT)
        MEMBER(forceLodLayer, "Force Lod Layer", I_SAVE | I_VIEW | I_EDIT)
        PROPERTY("forceDistance", "Force Distance", GetForceDistance, SetForceDistance, I_SAVE | I_VIEW | I_EDIT)
        MEMBER(flags, "Flags", I_SAVE | I_VIEW | I_EDIT)
    );
};

int32 LodComponent::GetLodLayersCount()
{
	return (int32)lodLayers.size();
}

float32 LodComponent::GetLodLayerDistance(int32 layerNum)
{
	DVASSERT(0 <= layerNum && layerNum < MAX_LOD_LAYERS);
	return lodLayersArrayWorking[layerNum].distance;
}
    
float32 LodComponent::GetLodLayerDistanceOriginal(int32 layerNum)
{
    DVASSERT(0 <= layerNum && layerNum < MAX_LOD_LAYERS);
    return lodLayersArrayOriginal[layerNum].distance;
}


float32 LodComponent::GetLodLayerNear(int32 layerNum)
{
	DVASSERT(0 <= layerNum && layerNum < MAX_LOD_LAYERS);
	return lodLayersArrayWorking[layerNum].nearDistance;
}

float32 LodComponent::GetLodLayerFar(int32 layerNum)
{
	DVASSERT(0 <= layerNum && layerNum < MAX_LOD_LAYERS);
	return lodLayersArrayWorking[layerNum].farDistance;
}

float32 LodComponent::GetLodLayerNearSquare(int32 layerNum)
{
	DVASSERT(0 <= layerNum && layerNum < MAX_LOD_LAYERS);
	return lodLayersArrayWorking[layerNum].nearDistanceSq;
}

float32 LodComponent::GetLodLayerFarSquare(int32 layerNum)
{
	DVASSERT(0 <= layerNum && layerNum < MAX_LOD_LAYERS);
	return lodLayersArrayWorking[layerNum].farDistanceSq;
}

};

#endif //__DAVAENGINE_LOD_COMPONENT_H__
