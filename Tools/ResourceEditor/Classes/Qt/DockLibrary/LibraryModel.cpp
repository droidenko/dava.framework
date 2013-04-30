#include <QTreeView>
#include "SceneEditor/EditorSettings.h"
#include "Qt/Main/QtUtils.h"
#include "Qt/DockLibrary/LibraryModel.h"
#include "DAVAEngine.h"

using namespace DAVA;

// If you need to specify different color backgrounds for different files extensions,
// expand this list.
ExtensionToColorMap LibraryModel::extensionToBackgroundColorMap[] =
{
	{"dae", QColor(222, 239, 254) },
	{ NULL, Qt::black }
};
	
LibraryModel::LibraryModel(QObject *parent)
    :   QFileSystemModel(parent)
{
	SetFileNameFilters(true, true);
    setNameFilterDisables(false);
}

LibraryModel::~LibraryModel()
{ }

void LibraryModel::SetLibraryPath(const QString &path)
{
	setRootPath(QDir(path).canonicalPath());
}

QVariant LibraryModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
	{
		return QFileSystemModel::data(index, role);
	}

	if ((Qt::BackgroundColorRole == role))
    {
        QFileInfo info = fileInfo(index);
        
        if(info.isFile())
        {
			return GetColorForExtension(info.suffix(), extensionToBackgroundColorMap, index, role);
        }
    }

    return QFileSystemModel::data(index, role);
}

void LibraryModel::SetFileNameFilters(bool showDAEFiles, bool showSC2Files)
{
	QStringList nameFilters;
	if (showDAEFiles)
	{
	    nameFilters << QString("*.dae");
	}
	if (showSC2Files)
	{
		nameFilters << QString("*.sc2");
	}

	QDir::Filters filterForNoneSelected = QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Dirs | QDir::Drives;
	if (nameFilters.empty())
	{
		// Show directories only in case nothing is selected.
		this->setFilter(filterForNoneSelected);
	}
	else
	{
		// Show directories and files in case file tyoes are selected.
		this->setFilter(filterForNoneSelected | QDir::Files);
		setNameFilters(nameFilters);
	}
}

QVariant LibraryModel::GetColorForExtension(const QString& extension, const ExtensionToColorMap* colorMap, const QModelIndex &index, int role) const
{
	int pos = 0;
	while (colorMap[pos].extension != NULL)
	{
		if (colorMap[pos].extension.compare(extension,  Qt::CaseInsensitive) == 0)
		{
			return colorMap[pos].color;
		}
		
		pos ++;
	}

    return QFileSystemModel::data(index, role);
}
