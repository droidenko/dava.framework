#include "Qt/Scene/System/HoodSystem/ScaleHood.h"
#include "Qt/Scene/System/ModifSystem.h"

// framework
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"


ScaleHood::ScaleHood() : HoodObject(4.0f)
{
	DAVA::float32 c = 2 * baseSize / 3;

	axisX = CreateLine(DAVA::Vector3(0, 0, 0), DAVA::Vector3(baseSize, 0, 0));
	axisX->axis = EM_AXIS_X;

	axisY = CreateLine(DAVA::Vector3(0, 0, 0), DAVA::Vector3(0, baseSize, 0));
	axisY->axis = EM_AXIS_Y;

	axisZ = CreateLine(DAVA::Vector3(0, 0, 0), DAVA::Vector3(0, 0, baseSize));
	axisZ->axis = EM_AXIS_Z;

	axisXY = CreateLine(DAVA::Vector3(c, 0, 0), DAVA::Vector3(0, c, 0));
	axisXY->axis = EM_AXIS_XY;

	axisXZ = CreateLine(DAVA::Vector3(c, 0, 0), DAVA::Vector3(0, 0, c));
	axisXZ->axis = EM_AXIS_XZ;

	axisYZ = CreateLine(DAVA::Vector3(0, c, 0), DAVA::Vector3(0, 0, c));
	axisYZ->axis = EM_AXIS_YZ;
}

ScaleHood::~ScaleHood()
{

}

void ScaleHood::Draw(int selectedAxis, int mouseOverAxis)
{
	int oldState = DAVA::RenderManager::Instance()->GetState();
	DAVA::eBlendMode oldBlendSrc = DAVA::RenderManager::Instance()->GetSrcBlend();
	DAVA::eBlendMode oldBlendDst = DAVA::RenderManager::Instance()->GetDestBlend();

	DAVA::RenderManager::Instance()->SetState(DAVA::RenderState::STATE_BLEND | DAVA::RenderState::STATE_COLORMASK_ALL | DAVA::RenderState::STATE_DEPTH_WRITE);
	DAVA::RenderManager::Instance()->SetBlendMode(DAVA::BLEND_SRC_ALPHA, DAVA::BLEND_ONE_MINUS_SRC_ALPHA);

	// x
	if(mouseOverAxis) 
		DAVA::RenderManager::Instance()->SetColor(colorS);
	else 
		DAVA::RenderManager::Instance()->SetColor(colorX);

	DAVA::RenderHelper::Instance()->DrawLine(axisX->curFrom, axisX->curTo);

	// y
	if(mouseOverAxis) 
		DAVA::RenderManager::Instance()->SetColor(colorS);
	else 
		DAVA::RenderManager::Instance()->SetColor(colorY);

	DAVA::RenderHelper::Instance()->DrawLine(axisY->curFrom, axisY->curTo);

	// z
	if(mouseOverAxis) 
		DAVA::RenderManager::Instance()->SetColor(colorS);
	else 
		DAVA::RenderManager::Instance()->SetColor(colorZ);

	DAVA::RenderHelper::Instance()->DrawLine(axisZ->curFrom, axisZ->curTo);

	// xy xz yz axis
	DAVA::RenderManager::Instance()->SetColor(colorS);
	DAVA::RenderHelper::Instance()->DrawLine(axisXY->curFrom, axisXY->curTo);
	DAVA::RenderHelper::Instance()->DrawLine(axisXZ->curFrom, axisXZ->curTo);
	DAVA::RenderHelper::Instance()->DrawLine(axisYZ->curFrom, axisYZ->curTo);

	// xy xz yz plane
	if(mouseOverAxis)
	{
		DAVA::Color colorSBlend(colorS.r, colorS.g, colorS.b, 0.3f);
		DAVA::RenderManager::Instance()->SetColor(colorSBlend);

		DAVA::Polygon3 poly;
		poly.AddPoint(axisXY->curFrom);
		poly.AddPoint(axisXY->curTo);
		poly.AddPoint(axisYZ->curTo);
		DAVA::RenderHelper::Instance()->FillPolygon(poly);
	}

	// draw axis spheres
	DAVA::float32 boxSize = axisX->curScale * baseSize / 12;

	DAVA::RenderManager::Instance()->SetColor(colorX);
	DAVA::RenderHelper::Instance()->FillBox(DAVA::AABBox3(axisX->curTo, boxSize));

	DAVA::RenderManager::Instance()->SetColor(colorY);
	DAVA::RenderHelper::Instance()->FillBox(DAVA::AABBox3(axisY->curTo, boxSize));

	DAVA::RenderManager::Instance()->SetColor(colorZ);
	DAVA::RenderHelper::Instance()->FillBox(DAVA::AABBox3(axisZ->curTo, boxSize));

	DAVA::RenderManager::Instance()->SetBlendMode(oldBlendSrc, oldBlendDst);
	DAVA::RenderManager::Instance()->SetState(oldState);
}
