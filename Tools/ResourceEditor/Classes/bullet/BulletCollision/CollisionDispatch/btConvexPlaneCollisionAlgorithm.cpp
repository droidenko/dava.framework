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


/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2006 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it freely,
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#include "btConvexPlaneCollisionAlgorithm.h"

#include "BulletCollision/CollisionDispatch/btCollisionDispatcher.h"
#include "BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "BulletCollision/CollisionShapes/btConvexShape.h"
#include "BulletCollision/CollisionShapes/btStaticPlaneShape.h"

//#include <stdio.h>

btConvexPlaneCollisionAlgorithm::btConvexPlaneCollisionAlgorithm(btPersistentManifold* mf,const btCollisionAlgorithmConstructionInfo& ci,btCollisionObject* col0,btCollisionObject* col1, bool isSwapped, int numPerturbationIterations,int minimumPointsPerturbationThreshold)
: btCollisionAlgorithm(ci),
m_ownManifold(false),
m_manifoldPtr(mf),
m_isSwapped(isSwapped),
m_numPerturbationIterations(numPerturbationIterations),
m_minimumPointsPerturbationThreshold(minimumPointsPerturbationThreshold)
{
	btCollisionObject* convexObj = m_isSwapped? col1 : col0;
	btCollisionObject* planeObj = m_isSwapped? col0 : col1;

	if (!m_manifoldPtr && m_dispatcher->needsCollision(convexObj,planeObj))
	{
		m_manifoldPtr = m_dispatcher->getNewManifold(convexObj,planeObj);
		m_ownManifold = true;
	}
}


btConvexPlaneCollisionAlgorithm::~btConvexPlaneCollisionAlgorithm()
{
	if (m_ownManifold)
	{
		if (m_manifoldPtr)
			m_dispatcher->releaseManifold(m_manifoldPtr);
	}
}

void btConvexPlaneCollisionAlgorithm::collideSingleContact (const btQuaternion& perturbeRot, btCollisionObject* body0,btCollisionObject* body1,const btDispatcherInfo& dispatchInfo,btManifoldResult* resultOut)
{
    btCollisionObject* convexObj = m_isSwapped? body1 : body0;
	btCollisionObject* planeObj = m_isSwapped? body0: body1;

	btConvexShape* convexShape = (btConvexShape*) convexObj->getCollisionShape();
	btStaticPlaneShape* planeShape = (btStaticPlaneShape*) planeObj->getCollisionShape();

    bool hasCollision = false;
	const btVector3& planeNormal = planeShape->getPlaneNormal();
	const btScalar& planeConstant = planeShape->getPlaneConstant();
	
	btTransform convexWorldTransform = convexObj->getWorldTransform();
	btTransform convexInPlaneTrans;
	convexInPlaneTrans= planeObj->getWorldTransform().inverse() * convexWorldTransform;
	//now perturbe the convex-world transform
	convexWorldTransform.getBasis()*=btMatrix3x3(perturbeRot);
	btTransform planeInConvex;
	planeInConvex= convexWorldTransform.inverse() * planeObj->getWorldTransform();
	
	btVector3 vtx = convexShape->localGetSupportingVertex(planeInConvex.getBasis()*-planeNormal);

	btVector3 vtxInPlane = convexInPlaneTrans(vtx);
	btScalar distance = (planeNormal.dot(vtxInPlane) - planeConstant);

	btVector3 vtxInPlaneProjected = vtxInPlane - distance*planeNormal;
	btVector3 vtxInPlaneWorld = planeObj->getWorldTransform() * vtxInPlaneProjected;

	hasCollision = distance < m_manifoldPtr->getContactBreakingThreshold();
	resultOut->setPersistentManifold(m_manifoldPtr);
	if (hasCollision)
	{
		/// report a contact. internally this will be kept persistent, and contact reduction is done
		btVector3 normalOnSurfaceB = planeObj->getWorldTransform().getBasis() * planeNormal;
		btVector3 pOnB = vtxInPlaneWorld;
		resultOut->addContactPoint(normalOnSurfaceB,pOnB,distance);
	}
}


void btConvexPlaneCollisionAlgorithm::processCollision (btCollisionObject* body0,btCollisionObject* body1,const btDispatcherInfo& dispatchInfo,btManifoldResult* resultOut)
{
	(void)dispatchInfo;
	if (!m_manifoldPtr)
		return;

	btCollisionObject* convexObj = m_isSwapped? body1 : body0;
	btCollisionObject* planeObj = m_isSwapped? body0: body1;

	btConvexShape* convexShape = (btConvexShape*) convexObj->getCollisionShape();
	btStaticPlaneShape* planeShape = (btStaticPlaneShape*) planeObj->getCollisionShape();

	bool hasCollision = false;
	const btVector3& planeNormal = planeShape->getPlaneNormal();
	const btScalar& planeConstant = planeShape->getPlaneConstant();
	btTransform planeInConvex;
	planeInConvex= convexObj->getWorldTransform().inverse() * planeObj->getWorldTransform();
	btTransform convexInPlaneTrans;
	convexInPlaneTrans= planeObj->getWorldTransform().inverse() * convexObj->getWorldTransform();

	btVector3 vtx = convexShape->localGetSupportingVertex(planeInConvex.getBasis()*-planeNormal);
	btVector3 vtxInPlane = convexInPlaneTrans(vtx);
	btScalar distance = (planeNormal.dot(vtxInPlane) - planeConstant);

	btVector3 vtxInPlaneProjected = vtxInPlane - distance*planeNormal;
	btVector3 vtxInPlaneWorld = planeObj->getWorldTransform() * vtxInPlaneProjected;

	hasCollision = distance < m_manifoldPtr->getContactBreakingThreshold();
	resultOut->setPersistentManifold(m_manifoldPtr);
	if (hasCollision)
	{
		/// report a contact. internally this will be kept persistent, and contact reduction is done
		btVector3 normalOnSurfaceB = planeObj->getWorldTransform().getBasis() * planeNormal;
		btVector3 pOnB = vtxInPlaneWorld;
		resultOut->addContactPoint(normalOnSurfaceB,pOnB,distance);
	}

	//the perturbation algorithm doesn't work well with implicit surfaces such as spheres, cylinder and cones:
	//they keep on rolling forever because of the additional off-center contact points
	//so only enable the feature for polyhedral shapes (btBoxShape, btConvexHullShape etc)
	if (convexShape->isPolyhedral() && resultOut->getPersistentManifold()->getNumContacts()<m_minimumPointsPerturbationThreshold)
	{
		btVector3 v0,v1;
		btPlaneSpace1(planeNormal,v0,v1);
		//now perform 'm_numPerturbationIterations' collision queries with the perturbated collision objects

		const btScalar angleLimit = 0.125f * SIMD_PI;
		btScalar perturbeAngle;
		btScalar radius = convexShape->getAngularMotionDisc();
		perturbeAngle = gContactBreakingThreshold / radius;
		if ( perturbeAngle > angleLimit ) 
				perturbeAngle = angleLimit;

		btQuaternion perturbeRot(v0,perturbeAngle);
		for (int i=0;i<m_numPerturbationIterations;i++)
		{
			btScalar iterationAngle = i*(SIMD_2_PI/btScalar(m_numPerturbationIterations));
			btQuaternion rotq(planeNormal,iterationAngle);
			collideSingleContact(rotq.inverse()*perturbeRot*rotq,body0,body1,dispatchInfo,resultOut);
		}
	}

	if (m_ownManifold)
	{
		if (m_manifoldPtr->getNumContacts())
		{
			resultOut->refreshContactPoints();
		}
	}
}

btScalar btConvexPlaneCollisionAlgorithm::calculateTimeOfImpact(btCollisionObject* col0,btCollisionObject* col1,const btDispatcherInfo& dispatchInfo,btManifoldResult* resultOut)
{
	(void)resultOut;
	(void)dispatchInfo;
	(void)col0;
	(void)col1;

	//not yet
	return btScalar(1.);
}
