/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/
#include "DAVAEngine.h"
#include <qnamespace.h>
#include "HierarchyTreeNode.h"
#include "HierarchyTreeController.h"

using namespace DAVA;

class HierarchyTreeControlNode;

class DefaultScreen : public UIScreen
{
public:
	DefaultScreen();
	virtual ~DefaultScreen();
	
	virtual void Draw(const UIGeometricData &geometricData);
	virtual void SystemDraw(const UIGeometricData &geometricData);
	virtual void Update(float32 timeElapsed);
	virtual bool IsPointInside(const Vector2& point, bool expandWithFocus);

	void SetScale(const Vector2& scale);
	void SetPos(const Vector2& pos);

	Vector2 LocalToInternal(const Vector2& localPoint) const;
	
	virtual void Input(UIEvent * touch);
	virtual bool SystemInput(UIEvent *currentInput);
	
	Qt::CursorShape GetCursor(const Vector2&);
	void MouseInputMove(const Vector2& pos);
	
	void BacklightControl(const Vector2& pos);
	bool IsDropEnable(const Vector2& pos)const;
	
private:
	enum InputState
	{
		InputStateSelection,
		InputStateDrag,
		InputStateSize,
		InputStateSelectorControl,
		InputStateScreenMove
	};
	
	enum ResizeType
	{
		ResizeTypeNoResize,
		ResizeTypeLeft,
		ResizeTypeRight,
		ResizeTypeTop,
		ResizeTypeBottom,
		ResizeTypeLeftTop,
		ResizeTypeLeftBottom,
		ResizeTypeRigthTop,
		ResizeTypeRightBottom,
		ResizeTypeMove,
	};
	
	HierarchyTreeControlNode* GetSelectedControl(const Vector2& point, const HierarchyTreeNode* parent) const;
	void GetSelectedControl(HierarchyTreeNode::HIERARCHYTREENODESLIST& list, const Rect& rect, const HierarchyTreeNode* parent) const;
	
	class SmartSelection
	{
	public:
		SmartSelection(HierarchyTreeNode::HIERARCHYTREENODEID id);
		
		bool IsEqual(const SmartSelection* item) const;
		HierarchyTreeNode::HIERARCHYTREENODEID GetFirst() const;
		HierarchyTreeNode::HIERARCHYTREENODEID GetLast() const;
		HierarchyTreeNode::HIERARCHYTREENODEID GetNext(HierarchyTreeNode::HIERARCHYTREENODEID id) const;
	private:
		typedef std::vector<HierarchyTreeNode::HIERARCHYTREENODEID> SelectionVector;
		void FormatSelectionVector(SelectionVector& selection) const;
	public:
		class childsSet: public std::vector<SmartSelection *>
		{
		public:
			virtual ~childsSet();
		};
		SmartSelection* parent;
		childsSet childs;
		HierarchyTreeNode::HIERARCHYTREENODEID id;
	};
	HierarchyTreeControlNode* SmartGetSelectedControl(const Vector2& point);
	void SmartGetSelectedControl(SmartSelection* list, const HierarchyTreeNode* parent, const Vector2& point);
	HierarchyTreeControlNode* GetSelectedControl(const Vector2& point);
	
	void ApplyMoveDelta(const Vector2& delta);
	HierarchyTreeController::SELECTEDCONTROLNODES GetActiveMoveControls() const;
	void ResetMoveDelta();
	void SaveControlsPostion();
	void MoveControl(const Vector2& delta);

	void DeleteSelectedControls();
	
	void ApplySizeDelta(const Vector2& delta);
	bool IsNeedApplyResize() const;
	void ResetSizeDelta();
	void ResizeControl();
	ResizeType GetResizeType(const HierarchyTreeControlNode* selectedControlNode, const Vector2& point) const;
	Qt::CursorShape ResizeTypeToQt(ResizeType);

	void ApplyMouseSelection(const Vector2& rectSize);
	
	void MouseInputBegin(const DAVA::UIEvent* event);
	void MouseInputEnd(const DAVA::UIEvent* event);
	void MouseInputDrag(const DAVA::UIEvent* event);
	void KeyboardInput(const DAVA::UIEvent* event);
	
	Vector2 GetInputDelta(const Vector2& point) const;
	
	Rect GetControlRect(const HierarchyTreeControlNode* control) const;
	void CopySelectedControls();
	
private:
	Vector2 scale;
	Vector2 pos;
	
	InputState inputState;
	ResizeType resizeType;
	Rect resizeRect;
	Vector2 inputPos;
	typedef Map<const UIControl*, Vector2> MAP_START_CONTROL_POS;
	MAP_START_CONTROL_POS startControlPos;
	HierarchyTreeControlNode* lastSelectedControl;
	bool copyControlsInProcess;
	//This flag should prevent additional control selection in MouseInputEnd event handler
	bool useMouseUpSelection;
	
	UIControl* selectorControl;
	
	SmartSelection* oldSmartSelected;
	HierarchyTreeNode::HIERARCHYTREENODEID oldSmartSelectedId;

	// Whether the Mouse Begin event happened?
	bool mouseAlreadyPressed;
};