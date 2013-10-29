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

#include "SpeedTreeLeafBatch.h"
#include "Render/Shader.h"
#include "Scene3D/SceneFileV2.h"

namespace DAVA
{

SpeedTreeLeafBatch::SpeedTreeLeafBatch(DAVA::Texture * tex)
{	
    shader = new Shader();
    shader->LoadFromYaml("~res:/Shaders/SpeedTree/SpeedTreeBillLeaf.shader");
    shader->Recompile();

    uniformWorldTranslate = shader->FindUniformIndexByName("worldTranslate");
    uniformTexture0 = shader->FindUniformIndexByName("texture0");
    uniformWorldScale = shader->FindUniformIndexByName("worldScale");

    SetTexture(tex);
}

SpeedTreeLeafBatch::~SpeedTreeLeafBatch()
{
    SafeRelease(shader);
    SafeRelease(texture);
}

void SpeedTreeLeafBatch::SetTexture(Texture * _texture)
{
    texture = SafeRetain(_texture);
}

void SpeedTreeLeafBatch::Draw(Camera * camera)
{
    if(!renderObject)
        return;

    if(!RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::SPEEDTREE_LEAFS_DRAW))
        return;

    Matrix4 * worldTransformPtr = renderObject->GetWorldTransformPtr();
    if (!worldTransformPtr)
    {
        return;
    }

    if(!GetVisible())
        return;

    Matrix4 finalMatrix = (*worldTransformPtr) * camera->GetMatrix();
    RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, finalMatrix);

    RenderManager::State()->SetTexture(texture);

    RenderManager::Instance()->SetShader(shader);
    RenderManager::Instance()->SetRenderData(dataSource->renderDataObject);
    RenderManager::Instance()->FlushState();
    RenderManager::Instance()->AttachRenderData();

    if(uniformWorldTranslate != -1)
    {
        shader->SetUniformValueByIndex(uniformWorldTranslate, finalMatrix.GetTranslationVector());
    }

    if(uniformTexture0 != -1)
    {
        shader->SetUniformValueByIndex(uniformTexture0, 0);
    }
    
    if(uniformWorldScale != -1)
    {
        shader->SetUniformValueByIndex(uniformWorldScale, worldTransformPtr->GetScaleVector());
    }

    if (dataSource->renderDataObject->GetIndexBufferID() != 0)
    {
        RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, dataSource->indexCount, EIF_16, 0);
    }
    else
    {
        RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, dataSource->indexCount, EIF_16, dataSource->indexArray);
    }
}

RenderBatch * SpeedTreeLeafBatch::Clone(RenderBatch * dstNode)
{
    if (!dstNode) 
    {
        DVASSERT_MSG(IsPointerToExactClass<SpeedTreeLeafBatch>(this), "Can clone only SpeedTreeLeafBatch");
        dstNode = new SpeedTreeLeafBatch();
    }
    
    RenderBatch::Clone(dstNode);
    SpeedTreeLeafBatch *nd = (SpeedTreeLeafBatch *)dstNode;
    
    SafeRelease(nd->texture);
    SafeRelease(nd->shader);
    
    nd->shader = SafeRetain(shader);
    nd->texture = SafeRetain(texture);

    return nd;
}

void SpeedTreeLeafBatch::Save(KeyedArchive *archive, SceneFileV2 *sceneFile)
{
    RenderBatch::Save(archive, sceneFile);

    if(NULL != archive)
    {
        FilePath path = texture->GetPathname();
        if(!path.IsEmpty())
        {
            String filename = path.GetRelativePathname(sceneFile->GetScenePath());
            archive->SetString("speedtree.leaftexture", filename);
        }
    }
}

void SpeedTreeLeafBatch::Load(KeyedArchive *archive, SceneFileV2 *sceneFile)
{
    RenderBatch::Load(archive, sceneFile);

    if(NULL != archive && NULL != sceneFile)
    {
        String relativePathname = archive->GetString("speedtree.leaftexture");
        if (!relativePathname.empty())
        {
            FilePath texPath;

            if(relativePathname[0] == '~') //path like ~res:/Gfx...
                texPath = FilePath(relativePathname);
            else
                texPath = sceneFile->GetScenePath() + relativePathname;

            texture = Texture::CreateFromFile(texPath);
        }
    }
}

};