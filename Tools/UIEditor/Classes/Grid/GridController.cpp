//
//  GridManager.cpp
//  UIEditor
//
//  Created by Yuri Coder on 12/23/13.
//
//

#include "GridController.h"

// Construction/destruction.
GridController::GridController() :
    gridSpacing(1.0f, 1.0f),
    screenScale(1.0f)
{
}

GridController::~GridController()
{
}

void GridController::SetGridSpacing(const Vector2& spacing)
{
    gridSpacing = spacing;
}

void GridController::SetScale(float32 scale)
{
    screenScale = scale;
}

Vector2 GridController::RecalculateMousePos(const Vector2& mousePos)
{
    float32 stepX = (screenScale * gridSpacing.x);
    float32 stepY = (screenScale * gridSpacing.y);

    return Vector2(CalculateDiscreteStep(mousePos.x, stepX),
                   CalculateDiscreteStep(mousePos.y, stepY));
}

