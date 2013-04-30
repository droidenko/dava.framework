#include "Qt/Scene/System/HoodSystem/RotateHood.h"
#include "Qt/Scene/System/ModifSystem.h"

// framework
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"

RotateHood::RotateHood() : HoodObject(4.0f)
{
	DAVA::float32 b = baseSize / 4;
	DAVA::float32 r = 2 * baseSize / 3;

	DAVA::float32 step = DAVA::PI_05 / ROTATE_HOOD_CIRCLE_PARTS_COUNT;
	DAVA::float32 x, y, lx = r, ly = 0;

	axisX = CreateLine(DAVA::Vector3(b, 0, 0), DAVA::Vector3(baseSize, 0, 0));
	axisX->axis = EM_AXIS_X;
	axisY = CreateLine(DAVA::Vector3(0, b, 0), DAVA::Vector3(0, baseSize, 0));
	axisY->axis = EM_AXIS_Y;
	axisZ = CreateLine(DAVA::Vector3(0, 0, b), DAVA::Vector3(0, 0, baseSize));
	axisZ->axis = EM_AXIS_Z;

	for(int i = 0; i < ROTATE_HOOD_CIRCLE_PARTS_COUNT; ++i)
	{
		DAVA::float32 angle = step * (i + 1);

		x = r * cosf(angle);
		y = r * sinf(angle);

		axisXc[i] = CreateLine(DAVA::Vector3(0, lx, ly), DAVA::Vector3(0, x, y));
		axisXc[i]->axis = EM_AXIS_X;
		axisYc[i] = CreateLine(DAVA::Vector3(lx, 0, ly), DAVA::Vector3(x, 0, y));
		axisYc[i]->axis = EM_AXIS_Y;
		axisZc[i] = CreateLine(DAVA::Vector3(lx, ly, 0), DAVA::Vector3(x, y, 0));
		axisZc[i]->axis = EM_AXIS_Z;

		lx = x;
		ly = y;
	}
}

RotateHood::~RotateHood()
{ }

void RotateHood::Draw(int selectedAxis, int mouseOverAxis)
{
	int oldState = DAVA::RenderManager::Instance()->GetState();
	DAVA::eBlendMode oldBlendSrc = DAVA::RenderManager::Instance()->GetSrcBlend();
	DAVA::eBlendMode oldBlendDst = DAVA::RenderManager::Instance()->GetDestBlend();

	DAVA::RenderManager::Instance()->SetBlendMode(DAVA::BLEND_SRC_ALPHA, DAVA::BLEND_ONE_MINUS_SRC_ALPHA);
	DAVA::RenderManager::Instance()->SetState(DAVA::RenderState::STATE_BLEND | DAVA::RenderState::STATE_COLORMASK_ALL);

	DAVA::Color colorSBlend(colorS.r, colorS.g, colorS.b, 0.3f);
	DAVA::Vector3 curPos = axisX->curPos;

	// x
	if(selectedAxis == EM_AXIS_X || selectedAxis == EM_AXIS_YZ) 
	{
		DAVA::RenderManager::Instance()->SetColor(colorSBlend);

		DAVA::Polygon3 poly;
		poly.AddPoint(curPos);
		for(int i = 0; i < ROTATE_HOOD_CIRCLE_PARTS_COUNT; ++i)
		{
			poly.AddPoint(axisXc[i]->curFrom);
		}
		poly.AddPoint(axisXc[ROTATE_HOOD_CIRCLE_PARTS_COUNT - 1]->curTo);
		DAVA::RenderHelper::Instance()->FillPolygon(poly);

		DAVA::RenderManager::Instance()->SetColor(colorS);
	}
	else
		DAVA::RenderManager::Instance()->SetColor(colorX);

	DAVA::RenderHelper::Instance()->DrawLine(axisX->curFrom, axisX->curTo);
	for(int i = 0; i < ROTATE_HOOD_CIRCLE_PARTS_COUNT; ++i)
	{
		DAVA::RenderHelper::Instance()->DrawLine(axisXc[i]->curFrom, axisXc[i]->curTo);
	}

	// y
	if(selectedAxis == EM_AXIS_Y || selectedAxis == EM_AXIS_XZ) 
	{
		DAVA::RenderManager::Instance()->SetColor(colorSBlend);

		DAVA::Polygon3 poly;
		poly.AddPoint(curPos);
		for(int i = 0; i < ROTATE_HOOD_CIRCLE_PARTS_COUNT; ++i)
		{
			poly.AddPoint(axisYc[i]->curFrom);
		}
		poly.AddPoint(axisYc[ROTATE_HOOD_CIRCLE_PARTS_COUNT - 1]->curTo);
		DAVA::RenderHelper::Instance()->FillPolygon(poly);
	
		DAVA::RenderManager::Instance()->SetColor(colorS);
	}
	else
		DAVA::RenderManager::Instance()->SetColor(colorY);

	DAVA::RenderHelper::Instance()->DrawLine(axisY->curFrom, axisY->curTo);
	for(int i = 0; i < ROTATE_HOOD_CIRCLE_PARTS_COUNT; ++i)
	{
		DAVA::RenderHelper::Instance()->DrawLine(axisYc[i]->curFrom, axisYc[i]->curTo);
	}

	// z
	if(selectedAxis == EM_AXIS_Z || selectedAxis == EM_AXIS_XY)
	{
		DAVA::RenderManager::Instance()->SetColor(colorSBlend);

		DAVA::Polygon3 poly;
		poly.AddPoint(curPos);
		for(int i = 0; i < ROTATE_HOOD_CIRCLE_PARTS_COUNT; ++i)
		{
			poly.AddPoint(axisZc[i]->curFrom);
		}
		poly.AddPoint(axisZc[ROTATE_HOOD_CIRCLE_PARTS_COUNT - 1]->curTo);
		DAVA::RenderHelper::Instance()->FillPolygon(poly);
	
		DAVA::RenderManager::Instance()->SetColor(colorS);
	}
	else
		DAVA::RenderManager::Instance()->SetColor(colorZ);

	DAVA::RenderHelper::Instance()->DrawLine(axisZ->curFrom, axisZ->curTo);
	for(int i = 0; i < ROTATE_HOOD_CIRCLE_PARTS_COUNT; ++i)
	{
		DAVA::RenderHelper::Instance()->DrawLine(axisZc[i]->curFrom, axisZc[i]->curTo);
	}

	// draw axis spheres
	DAVA::float32 radius = axisX->curScale * baseSize / 12;

	DAVA::RenderManager::Instance()->SetColor(colorX);
	DAVA::RenderHelper::Instance()->FillDodecahedron(axisX->curTo, radius);

	DAVA::RenderManager::Instance()->SetColor(colorY);
	DAVA::RenderHelper::Instance()->FillDodecahedron(axisY->curTo, radius);

	DAVA::RenderManager::Instance()->SetColor(colorZ);
	DAVA::RenderHelper::Instance()->FillDodecahedron(axisZ->curTo, radius);

	DAVA::RenderManager::Instance()->SetBlendMode(oldBlendSrc, oldBlendDst);
	DAVA::RenderManager::Instance()->SetState(oldState);
}
