//
//  GridManager.h
//  UIEditor
//
//  Created by Yuri Coder on 12/23/13.
//
//

#ifndef __GRIDCONTROLLER__H__
#define __GRIDCONTROLLER__H__

#include "DAVAEngine.h"
using namespace DAVA;

// This class is responsible for handling Grid in UI Editor and calculating the controls positions over it.
class GridController : public Singleton<GridController>
{
public:
    // Construction/destruction.
    GridController();
    virtual ~GridController();

    // Set the grid X and Y spacing.
    void SetGridSpacing(const Vector2& spacing);
    
    // Set the current screen scale.
    void SetScale(float32 scale);

    // Recalculate the mouse position according to the grid and current zoom level.
    Vector2 RecalculateMousePos(const Vector2& mousePos);

protected:
    // Calcute discrete step from position.
    inline float32 CalculateDiscreteStep(float32 position, float32 step);

private:
    // Grid spacing.
    Vector2 gridSpacing;
    
    // Screen scale.
    float32 screenScale;
};

inline float32 GridController::CalculateDiscreteStep(float32 position, float32 step)
{
    float32 scaledValue = position/step;
    float32 integerPart = floor(scaledValue);

    // Check the fractional part.
    if (scaledValue - integerPart < 0.5f)
    {
        return integerPart * step;
    }
    
    return (integerPart + 1) * step;
}

#endif /* defined(__GRIDCONTROLLER__H__) */
