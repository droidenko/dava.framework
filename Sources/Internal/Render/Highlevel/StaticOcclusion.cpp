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
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"
#include "Render/Highlevel/StaticOcclusion.h"
#include "Render/Highlevel/StaticOcclusionRenderPass.h"
#include "Render/Highlevel/RenderBatchArray.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/RenderFastNames.h"
#include "Render/Image.h"
#include "Render/ImageLoader.h"
#include "Utils/StringFormat.h"
#include "Platform/SystemTimer.h"

namespace DAVA
{
    
static const uint32 RENDER_TARGET_WIDTH = 1024;
static const uint32 RENDER_TARGET_HEIGHT = 512;
static DAVA::uint32 *decompressedBlockBuffer = NULL;  
    
StaticOcclusion::StaticOcclusion()
    : manager(5000)
{
    staticOcclusionRenderPass = 0;
    renderTargetSprite = 0;
    renderTargetTexture = 0;
    currentData = 0;
}
    
StaticOcclusion::~StaticOcclusion()
{
    SafeDelete(renderPassBatchArray);
    SafeDelete(staticOcclusionRenderPass);
    SafeRelease(renderTargetSprite);
    SafeRelease(renderTargetTexture);
}

    
void StaticOcclusion::BuildOcclusionInParallel(Vector<RenderObject*> & renderObjects,
                                               StaticOcclusionData * _currentData,
                                               RenderHierarchy * _renderHierarchy)
{
    staticOcclusionRenderPass = new StaticOcclusionRenderPass(renderSystem, PASS_FORWARD, this, RENDER_PASS_FORWARD_ID);
    
    staticOcclusionRenderPass->AddRenderLayer(new StaticOcclusionRenderLayer(LAYER_OPAQUE,
                                                                             RenderLayerBatchArray::SORT_ENABLED | RenderLayerBatchArray::SORT_BY_DISTANCE_FRONT_TO_BACK,
                                                                             this,
                                                                             RENDER_LAYER_OPAQUE_ID), LAST_LAYER);
	staticOcclusionRenderPass->AddRenderLayer(new StaticOcclusionRenderLayer(LAYER_AFTER_OPAQUE,
                                                                             RenderLayerBatchArray::SORT_ENABLED | RenderLayerBatchArray::SORT_BY_DISTANCE_FRONT_TO_BACK,
                                                                             this,
                                                                             RENDER_LAYER_AFTER_OPAQUE_ID), LAST_LAYER);
	staticOcclusionRenderPass->AddRenderLayer(new StaticOcclusionRenderLayer(LAYER_ALPHA_TEST_LAYER,
                                                                             RenderLayerBatchArray::SORT_ENABLED | RenderLayerBatchArray::SORT_BY_DISTANCE_FRONT_TO_BACK,
                                                                             this,
                                                                             RENDER_LAYER_ALPHA_TEST_LAYER_ID), LAST_LAYER);
    staticOcclusionRenderPass->AddRenderLayer(new StaticOcclusionRenderLayer(LAYER_TRANSLUCENT,
                                                                             RenderLayerBatchArray::SORT_ENABLED | RenderLayerBatchArray::SORT_BY_DISTANCE_FRONT_TO_BACK,
                                                                             this,
                                                                             RENDER_LAYER_TRANSLUCENT_ID), LAST_LAYER);
	staticOcclusionRenderPass->AddRenderLayer(new StaticOcclusionRenderLayer(LAYER_AFTER_TRANSLUCENT,
                                                                             RenderLayerBatchArray::SORT_ENABLED | RenderLayerBatchArray::SORT_BY_DISTANCE_FRONT_TO_BACK,
                                                                             this,
                                                                             RENDER_LAYER_AFTER_TRANSLUCENT_ID), LAST_LAYER);

    renderPassBatchArray = new RenderPassBatchArray(renderSystem);
    renderPassBatchArray->InitPassLayers(staticOcclusionRenderPass);
    
    
    currentData = _currentData;
    renderHierarchy = _renderHierarchy;
    occlusionAreaRect = currentData->bbox;
    xBlockCount = currentData->sizeX;
    yBlockCount = currentData->sizeY;
    zBlockCount = currentData->sizeZ;
    
    currentFrameX = 0; //xBlockCount - 1;
    currentFrameY = 0; //yBlockCount - 1;
    currentFrameZ = 0; //zBlockCount - 1;
    
    
    for (uint32 k = 0; k < 6; ++k)
    {
        cameras[k] = new Camera();
        cameras[k]->SetupPerspective(95.0f, (float32)RENDER_TARGET_WIDTH / (float32)RENDER_TARGET_HEIGHT, 1.0f, 2500.0f);
    }
    
    
    if (!renderTargetTexture)
        renderTargetTexture = Texture::CreateFBO(RENDER_TARGET_WIDTH, RENDER_TARGET_HEIGHT, FORMAT_RGBA8888, Texture::DEPTH_RENDERBUFFER);
    if (!renderTargetSprite)
        renderTargetSprite = Sprite::CreateFromTexture(renderTargetTexture, 0, 0, RENDER_TARGET_WIDTH, RENDER_TARGET_HEIGHT);
    
    for (uint32 k = 0; k < renderObjects.size(); ++k)
        renderObjects[k]->SetStaticOcclusionIndex((uint16)k);
    
    renderObjectsArray = renderObjects;
}
    
void StaticOcclusion::SetEqualVisibilityVector(Map<RenderObject*, Vector<RenderObject*> > & equalVisibility)
{
    equalVisibilityArray = equalVisibility;
}

    
//uint32 StaticOcclusion::CameraToCellIndex(Camera * camera)
//{
//    const Vector3 & position = camera->GetPosition();
//    if (!occlusionAreaRect.IsInside(position))return 0xFFFFFFFF;
//    
//    uint32 x = (uint32)(position.x / (float32)xBlockCount);
//    uint32 y = (uint32)(position.y / (float32)yBlockCount);
//    uint32 z = (uint32)(position.z / (float32)zBlockCount);
//    
//    
//}

//uint32 * StaticOcclusion::GetCellVisibilityData(Camera * camera)
//{
//    const Vector3 & position = camera->GetPosition();
//    if (!occlusionAreaRect.IsInside(position))return 0;
//    
//    uint32 x = (uint32)(position.x / (float32)xBlockCount);
//    uint32 y = (uint32)(position.y / (float32)yBlockCount);
//    uint32 z = (uint32)(position.z / (float32)zBlockCount);
//    
//    uint32 * objectsVisibility = visibilityData + z * (xBlockCount * yBlockCount * objectCount / 32) + y * (xBlockCount * objectCount / 32) + (x * objectCount / 32);
//    return objectsVisibility;
//}
    
AABBox3 StaticOcclusion::GetCellBox(uint32 x, uint32 y, uint32 z)
{
    Vector3 size = occlusionAreaRect.GetSize();
    
    size.x /= xBlockCount;
    size.y /= yBlockCount;
    size.z /= zBlockCount;
    
    Vector3 min(occlusionAreaRect.min.x + currentFrameX * size.x,
                occlusionAreaRect.min.y + currentFrameY * size.y,
                occlusionAreaRect.min.z + currentFrameZ * size.z);
    AABBox3 blockBBox(min, Vector3(min.x + size.x, min.y + size.y, min.z + size.z));
    return blockBBox;
}

uint32 StaticOcclusion::RenderFrame()
{
//    for (uint32 k = 0; k < 1000; ++k)
//    {
//        for (uint32 m = 0; m < 3000; ++m)
//        {
//            OcclusionQueryManagerHandle handle = manager.CreateQueryObject();
//            manager.ReleaseQueryObject(handle);
//        }
//    }
//    
    
    
    
    uint64 t1 = SystemTimer::Instance()->GetAbsoluteNano();
    const uint32 stepCount = 10;
    //uint32 renderFrameCount = xBlockCount * yBlockCount * zBlockCount * 4 * (stepSizeX * stepSizeY);
    
    AABBox3 cellBox = GetCellBox(currentFrameX, currentFrameY, currentFrameZ);
    Vector3 stepSize = cellBox.GetSize();
    stepSize /= stepCount;
    
    
    Vector3 directions[6] =
    {
        Vector3(1.0f, 0.0f, 0.0f),  // x
        Vector3(0.0f, 1.0f, 0.0f),  // y
        Vector3(-1.0f, 0.0f, 0.0f), // -x
        Vector3(0.0f, -1.0f, 0.0f), // -y
        Vector3(0.0f, 0.0f, 1.0f),
        Vector3(0.0f, 0.0f, -1.0f),
    };
    
    // Render One Block (100%)
    frameGlobalVisibleInfo.clear();

    uint32 blockIndex = currentFrameZ * (currentData->sizeX * currentData->sizeY)
                        + currentFrameY * (currentData->sizeX) + currentFrameX;

    //renderPositions.clear();
    
    uint32 effectiveSides[4][3]=
    {
        {0, 1, 3},
        {1, 0, 2},
        {2, 1, 3},
        {3, 0, 2},
    };
    
    for (uint32 side = 0; side < 4; ++side)
    {
        Camera * camera = cameras[side];

        Vector3 startPosition, directionX, directionY;
        if (side == 0) // +x
        {
            startPosition = Vector3(cellBox.max.x, cellBox.min.y, cellBox.min.z);
            directionX = Vector3(0.0f, 1.0f, 0.0f);
            directionY = Vector3(0.0f, 0.0f, 1.0f);
        }else if (side == 2) // -x
        {
            startPosition = Vector3(cellBox.min.x, cellBox.min.y, cellBox.min.z);
            directionX = Vector3(0.0f, 1.0f, 0.0f);
            directionY = Vector3(0.0f, 0.0f, 1.0f);
        }else if (side == 1) // +y
        {
            startPosition = Vector3(cellBox.min.x, cellBox.max.y, cellBox.min.z);
            directionX = Vector3(1.0f, 0.0f, 0.0f);
            directionY = Vector3(0.0f, 0.0f, 1.0f);
        }else if (side == 3) // -y
        {
            startPosition = Vector3(cellBox.min.x, cellBox.min.y, cellBox.min.z);
            directionX = Vector3(1.0f, 0.0f, 0.0f);
            directionY = Vector3(0.0f, 0.0f, 1.0f);
        }
        
        //Vector3 centerOnTargetSide = cellBox.GetCenter() + camera->GetDirection() * halfSize;
        //Vector3 crossProduct = CrossProduct(camera->GetDirection(), )

        
        // Render 360 from each point
        for (uint32 realSide = 0; realSide < 3; ++realSide)
            for (uint32 stepX = 0; stepX <= stepCount; ++stepX)
                for (uint32 stepY = 0; stepY <= stepCount; ++stepY)
                {
                    Vector3 renderPosition = startPosition + directionX * stepX * stepSize + directionY * stepY * stepSize;
                    
                    //renderPositions.push_back(renderPosition);
                    camera->SetPosition(renderPosition);
                    camera->SetDirection(directions[effectiveSides[side][realSide]]);
                    camera->SetUp(Vector3(0.0f, 0.0f, 1.0f));
                    camera->SetLeft(Vector3(1.0f, 0.0f, 1.0f));
                    camera->SetupDynamicParameters();
                    // Do Render
                    
                    RenderManager::Instance()->SetRenderTarget(renderTargetSprite);
                    RenderManager::Instance()->SetViewport(Rect(0, 0, RENDER_TARGET_WIDTH, RENDER_TARGET_HEIGHT), false);
                    
                    RenderManager::Instance()->SetDefault3DState();
                    RenderManager::Instance()->FlushState();
                    RenderManager::Instance()->Clear(Color(0.5f, 0.5f, 0.5f, 1.0f), 1.0f, 0);
                    
                    camera->SetupDynamicParameters();
                    
                    recordedBatches.clear();
                    
                    visibilityArray.Clear();
                    renderHierarchy->Clip(camera, &visibilityArray);

                    renderPassBatchArray->Clear();
                    renderPassBatchArray->PrepareVisibilityArray(&visibilityArray);
                    
                    staticOcclusionRenderPass->Draw(camera, renderPassBatchArray);
                    
                    size_t size = recordedBatches.size();
                    for (size_t k = 0; k < size; ++k)
                    {
                        std::pair<RenderBatch*, OcclusionQueryManagerHandle> & batchInfo = recordedBatches[k];
                        OcclusionQuery & query = manager.Get(batchInfo.second);
                        while (!query.IsResultAvailable())
                        {
                        }
                        uint32 result;
                        query.GetQuery(&result);
                        if (result != 0)
                        {
                            frameGlobalVisibleInfo.insert(batchInfo.first->GetRenderObject());
                        }
                        manager.ReleaseQueryObject(batchInfo.second);
                    }

                    RenderManager::Instance()->RestoreRenderTarget();
                    
                    if ((stepX == 0) && (stepY == 0) && (effectiveSides[side][realSide] == side))
                    {
                        Image * image = renderTargetTexture->CreateImageFromMemory();
                        ImageLoader::Save(image, Format("~doc:/renderimage_%d_%d_%d_%d.png", blockIndex, side, stepX, stepY));
                        SafeRelease(image);
                    }
                }
        
    }
    
    // Invisible on every frame
    uint32 invisibleObjectCount =  (uint32)renderObjectsArray.size() - frameGlobalVisibleInfo.size();
    uint32 visibleCount = frameGlobalVisibleInfo.size();
//    for (Map<RenderObject*, uint32>::iterator it = frameGlobalOccludedInfo.begin(), end = frameGlobalOccludedInfo.end(); it != end; ++it)
//    {
//        if (renderFrameCount == it->second)
//            invisibleObjectCount++;
//    }
    
    //uint32 blockIndex = z * (data->sizeX * data->sizeY) + y * (data->sizeX) + (x);
    
    for (Set<RenderObject*>::iterator it = frameGlobalVisibleInfo.begin(), end = frameGlobalVisibleInfo.end(); it != end; ++it)
    {
        RenderObject * obj = *it;
        DVASSERT(obj->GetStaticOcclusionIndex() != INVALID_STATIC_OCCLUSION_INDEX);
        currentData->SetVisibilityForObject(blockIndex, obj->GetStaticOcclusionIndex(), 1);
        
        Map<RenderObject*, Vector<RenderObject*> >::iterator findIt = equalVisibilityArray.find(obj);
        if (findIt != equalVisibilityArray.end())
        {
            Vector<RenderObject*> & equalObjects = findIt->second;
            uint32 size = equalObjects.size();
            for (uint32 k = 0; k < size; ++k)
            {
                DVASSERT(equalObjects[k]->GetStaticOcclusionIndex() != INVALID_STATIC_OCCLUSION_INDEX);
                currentData->SetVisibilityForObject(blockIndex, equalObjects[k]->GetStaticOcclusionIndex(), 1);
                invisibleObjectCount --;
                visibleCount++;
            }
        }
    }
    currentData->CompressData();
    t1 = SystemTimer::Instance()->GetAbsoluteNano() - t1;

    Logger::FrameworkDebug(Format("Object count:%d Vis Count: %d Invisible Object Count:%d time: %0.9llf", renderObjectsArray.size(), visibleCount, invisibleObjectCount, (double)t1 / 1e+9).c_str());
    
    //RenderManager::Instance()->SetRenderTarget((Texture*)0);
    
    //
    currentFrameX++;
    if (currentFrameX >= xBlockCount)
    {
        currentFrameX = 0;
        currentFrameY++;
        if (currentFrameY >= yBlockCount)
        {
            currentFrameY = 0;
            currentFrameZ++;
            if (currentFrameZ >= zBlockCount)
            {
                return 0;
            }
        }
    }
    return (xBlockCount * yBlockCount * zBlockCount - currentFrameX * currentFrameY * currentFrameZ);
}
    

void StaticOcclusion::RecordFrameQuery(RenderBatch * batch, OcclusionQueryManagerHandle handle)
{
    recordedBatches.push_back(std::pair<RenderBatch*, OcclusionQueryManagerHandle>(batch, handle));
}

  
StaticOcclusionData::StaticOcclusionData()
    : data(0)
    , objectCount(0)
    , blockCount(0)
    , sizeX(5)
    , sizeY(5)
    , sizeZ(2)
    , dataFormat(UNPACKED)
{
    
}
    
StaticOcclusionData::~StaticOcclusionData()
{
    SafeDeleteArray(data);
}
    
StaticOcclusionData & StaticOcclusionData::operator= (const StaticOcclusionData & other)
{
    sizeX = other.sizeX;
    sizeY = other.sizeY;
    sizeZ = other.sizeZ;
    objectCount = other.objectCount;
    blockCount = other.blockCount;
    bbox = other.bbox;
    
    SafeDeleteArray(data);
    data = new uint32[(blockCount * objectCount / 32)];
    memcpy(data, other.data, (blockCount * objectCount / 32) * 4);
    return *this;
}


void StaticOcclusionData::Init(uint32 _sizeX, uint32 _sizeY, uint32 _sizeZ, uint32 _objectCount, const AABBox3 & _bbox)
{
    objectCount = _objectCount;
    sizeX = _sizeX;
    sizeY = _sizeY;
    sizeZ = _sizeZ;
    blockCount = sizeX * sizeY * sizeZ;
    bbox = _bbox;
    
    objectCount += (32 - objectCount & 31);
    
    data = new uint32[(blockCount * objectCount / 32)];
    memset(data, 0, (blockCount * objectCount / 32));
    if (decompressedBlockBuffer) {
        delete [] decompressedBlockBuffer;
    }
    decompressedBlockBuffer = new uint32[(objectCount / 8)];
    uncompressedDataSize = blockCount * objectCount;
}

void StaticOcclusionData::SetVisibilityForObject(uint32 blockIndex, uint32 objectIndex, uint32 visible)
{
    data[(blockIndex * objectCount / 32) + (objectIndex / 32)] |= visible << (objectIndex & 31);
}
    
uint32 * StaticOcclusionData::GetBlockVisibilityData(uint32 blockIndex)
{
    if (dataFormat == UNPACKED) {
        return &data[(blockIndex * objectCount / 32)];
    } else {
        memset(decompressedBlockBuffer, 0, objectCount / 8);
        DecompessBlock(packedData +offsetPackedBlock[blockIndex],decompressedBlockBuffer,offsetPackedBlock[blockIndex+1]-offsetPackedBlock[blockIndex]);
        return decompressedBlockBuffer;
    }}


Vector<uint8> StaticOcclusionData::CompressBlock(uint32 *src, int size)
{
    Vector<uint8> compressedData;
    compressedData.reserve(1024);
    uint8 *b_src= (uint8*)src;
    uint8 bit = 0;
    uint8 count = 0;
    uint8 flag = 0;
    for (int byte=0; byte<size; byte++){
        for (uint8 bitPosition=0; bitPosition<8; bitPosition++)
        {
            bit = b_src[byte] & (b_src[byte] << bitPosition);
            if (bit) bit = 1;
            if ((flag!=bit)||(127==count))
            {
                uint8 compress = 0;
                if (flag) compress = 128;
                compress += count;
                compressedData.push_back(compress);
                count = 0;
                flag = bit;
                bitPosition--;
            }else
            {
                count++;
            }
        }
    }
    if (count>0)
    {
        uint8 compress = 0;
        if (flag) compress = 128;
        compress += count;
        compressedData.push_back(compress);
    }
    return compressedData;
}

void StaticOcclusionData::DecompessBlock(uint32 *src, uint32* dst,uint32 sizecompr)
{
    uint8 *b_src= (uint8*)src;
    uint8 *b_dst= (uint8*)dst;
    uint8 flag = 0;
    uint32 position = 0;
    for (int byte=0; byte<(sizecompr/8); byte++)
    {
        flag = (b_src[byte]>>7 );
        uint8 count=0;
        if (flag)
        {
            count = b_src[byte]&127;
            for (int bit=position%8; bit<count; bit++)
            {
                b_dst[position/8] |= 1 << (bit & 7);
                position++;
            }
        } else
        {
            count = b_src[byte];
            position+=count;
        }
    }
    
}

void StaticOcclusionData::CompressData()
{
    int size = 0;
    int offset = 0;
    Vector<uint32> offsetPackedBlockV;
    packedData = new uint32();
    for (int blockIndex=0; blockIndex<blockCount; blockIndex++)
    {
        Vector<uint8> compressedBlock = CompressBlock(&data[(blockIndex * objectCount / 32)],objectCount);
        size = compressedBlock.size();
        uint8 *b_dst = new uint8[size];
        for (uint32 i=0;i<size;i++)
        {
            b_dst[i] = compressedBlock[i];
        }
        offsetPackedBlockV.push_back(offset);
        uint8 * newPackedData = new uint8[offset+size];
        memcpy(newPackedData, packedData, offset);
        memcpy(newPackedData + offset, b_dst, size);
        SafeDeleteArray(packedData);
        SafeDeleteArray(b_dst);
        packedData = (uint32 *)newPackedData;
        offset += size;
    }
    offsetPackedBlockV.push_back(offset);
    offsetPackedBlock = new uint32[offsetPackedBlockV.size()];
    for (int i=0; i<offsetPackedBlockV.size(); ++i)
    {
        offsetPackedBlock[i]=offsetPackedBlockV[i];
    }
    dataFormat = PACKED;
    compressedDataSize = offsetPackedBlockV[offsetPackedBlockV.size()-1];
    //delete [] data;
}

    
};
