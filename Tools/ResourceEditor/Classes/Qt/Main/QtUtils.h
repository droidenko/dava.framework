#ifndef __QT_UTILS_H__
#define __QT_UTILS_H__

#include "DAVAEngine.h"
#include <QString>
#include <QColor>

DAVA::FilePath PathnameToDAVAStyle(const QString &convertedPathname);
DAVA::FilePath GetOpenFileName(const DAVA::String &title, const DAVA::FilePath &pathname, const DAVA::String &filter);

DAVA::WideString SizeInBytesToWideString(DAVA::float32 size);
DAVA::String SizeInBytesToString(DAVA::float32 size);

DAVA::Image * CreateTopLevelImage(const DAVA::FilePath &imagePathname);

void ShowErrorDialog(const DAVA::Set<DAVA::String> &errors);
void ShowErrorDialog(const DAVA::String &errorMessage);

bool IsKeyModificatorPressed(DAVA::int32 key);
bool IsKeyModificatorsPressed();

QColor ColorToQColor(const DAVA::Color& color);
DAVA::Color QColorToColor(const QColor& qcolor);

#endif // __QT_UTILS_H__
