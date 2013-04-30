#ifndef __NORMAL_HOOD_H__
#define __NORMAL_HOOD_H__

#include "Qt/Scene/System/HoodSystem/HoodObject.h"

struct NormalHood : public HoodObject
{
	NormalHood();
	~NormalHood();

	virtual void Draw(int selectedAxis, int mouseOverAxis);

	HoodCollObject *axisX;
	HoodCollObject *axisY;
	HoodCollObject *axisZ;
};

#endif // __NORMAL_HOOD_H__
