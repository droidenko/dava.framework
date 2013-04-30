#ifndef __MOVE_HOOD_H__
#define __MOVE_HOOD_H__

#include "Qt/Scene/System/HoodSystem/HoodObject.h"

struct MoveHood : public HoodObject
{
	MoveHood();
	~MoveHood();

	virtual void Draw(int selectedAxis, int mouseOverAxis);

	HoodCollObject *axisX;
	HoodCollObject *axisY;
	HoodCollObject *axisZ;

	HoodCollObject *axisXY1;
	HoodCollObject *axisXY2;
	HoodCollObject *axisXZ1;
	HoodCollObject *axisXZ2;
	HoodCollObject *axisYZ1;
	HoodCollObject *axisYZ2;
};

#endif // __MOVE_HOOD_H__
