#ifndef __ROTATE_HOOD_H__
#define __ROTATE_HOOD_H__

#include "Qt/Scene/System/HoodSystem/HoodObject.h"

#define ROTATE_HOOD_CIRCLE_PARTS_COUNT 5

struct RotateHood : public HoodObject
{
	RotateHood();
	~RotateHood();

	virtual void Draw(int selectedAxis, int mouseOverAxis);

	HoodCollObject* axisX;
	HoodCollObject* axisY;
	HoodCollObject* axisZ;

	HoodCollObject* axisXc[ROTATE_HOOD_CIRCLE_PARTS_COUNT];
	HoodCollObject* axisYc[ROTATE_HOOD_CIRCLE_PARTS_COUNT];
	HoodCollObject* axisZc[ROTATE_HOOD_CIRCLE_PARTS_COUNT];
};

#endif // __ROTATE_HOOD_H__
