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


#ifndef __UIEditor__BackgroundGridWidgetHelper__
#define __UIEditor__BackgroundGridWidgetHelper__

#include "Base/BaseTypes.h"
#include "UI/UIControl.h"
#include <QString>

namespace DAVA {
    
class BackgroundGridWidgetHelper
{
public:
    // Draw Type.
    static int GetDrawTypesCount();
    static UIControlBackground::eDrawType GetDrawType(int index);
    static QString GetDrawTypeDesc(int index);

    // Get the Draw Type description by the Value.
    static QString GetDrawTypeDescByType(UIControlBackground::eDrawType drawType);

    // Color Inherit.
    static int GetColorInheritTypesCount();
    static UIControlBackground::eColorInheritType GetColorInheritType(int index);
    static QString GetColorInheritTypeDesc(int index);

    // Get the Color Inherit Type Description by the Value.
    static QString GetColorInheritTypeDescByType(UIControlBackground::eColorInheritType inheritType);

    //Align.
    static int GetAlignTypesCount();
    static int GetAlignType(int index);
    static QString GetAlignTypeDesc(int index);

    // Get the Align Type Description by the Value.
    static QString GetAlignTypeDescByType(int alignType);

	//SpriteModificationType
	static int GetModificationType(int index);
    static QString GetModificationTypeDesc(int index);
	static int GetModificationTypesCount();

	static QString GetModificationTypeDescByType(int modificationType);

protected:
    // Validate the indexes.
    static bool ValidateDrawTypeIndex(int index);
    static bool ValidateColorInheritTypeIndex(int index);
    static bool ValidateAlginTypeIndex(int index);
	static bool ValidateSpriteModificationIndex(int index);
    
    // Maps.
    struct DrawTypesData
    {
        UIControlBackground::eDrawType drawType;
        const char* drawTypeDesc;
    };
    
    struct ColorInheritTypesData
    {
        UIControlBackground::eColorInheritType colorInheritType;
        const char* colorInheritTypeDesc;
    };

    struct AlignTypesData
    {
        int alignType; // int, not eAlign, since align types combinations are possible.
        const char* alignTypeDesc;
    };

	struct SpriteModificationTypesData
    {
        int spriteModificationType;
        const char* spriteModificationTypeDescription;
    };

    static const DrawTypesData drawTypesData[];
    static const ColorInheritTypesData colorInheritTypesData[];
    static const AlignTypesData alignTypesData[];
	static const SpriteModificationTypesData spriteModificationTypesData[];
};

};

#endif /* defined(__UIEditor__BackgroundGridWidgetHelper__) */
