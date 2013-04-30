#ifndef __TEXTURE_BROWSER_H__
#define __TEXTURE_BROWSER_H__

#include <QDialog>
#include <QMap>
#include "DAVAEngine.h"
#include "Qt/QtPosSaver/QtPosSaver.h"
#include "Qt/Scene/SceneDataManager.h"

class QModelIndex;
class TextureListDelegate;
class TextureListModel;
class TextureConvertor;
class QAbstractItemDelegate;
class QStatusBar;
class QLabel;
class QSlider;
struct JobItem;

namespace Ui {
class TextureBrowser;
}

class TextureBrowser : public QDialog
{
    Q_OBJECT

public:
	enum TextureView
	{
		ViewPVR,
		ViewDXT
	};

public:
    explicit TextureBrowser(QWidget *parent = 0);
    ~TextureBrowser();

protected:
	void closeEvent(QCloseEvent * e);

public slots:
	void sceneActivated(SceneData *scene);
	void sceneChanged(SceneData *scene);
	void sceneReleased(SceneData *scene);
	void sceneNodeSelected(SceneData *scene, DAVA::Entity *node);

private:
    Ui::TextureBrowser *ui;
	QtPosSaver posSaver;

	TextureListModel *textureListModel;
	TextureListDelegate *textureListImagesDelegate;

	QSlider *toolbarZoomSlider;
	QLabel *toolbarZoomSliderValue;
	
	QStatusBar *statusBar;
	QLabel *statusBarLabel;
	
	QMap<QString, int> textureListSortModes;

	DAVA::Scene *curScene;
	TextureView curTextureView;

	DAVA::Texture *curTexture;
	DAVA::TextureDescriptor *curDescriptor;

	void setScene(DAVA::Scene *scene);

	void setupTextureListToolbar();
	void setupTextureToolbar();
	void setupTexturesList();
	void setupImagesScrollAreas();
	void setupTextureListFilter();
	void setupTextureConverAllButton();
	void setupStatusBar();
	void setupTextureProperties();
	void setupTextureViewToolbar();
	
	void resetTextureInfo();

	void setTexture(DAVA::Texture *texture, DAVA::TextureDescriptor *descriptor);
	void setTextureView(TextureView view, bool forceConvert = false);

	void updateConvertedImageAndInfo(const QImage &image);
	void updateInfoColor(QLabel *label, const QColor &color = QColor());
	void updateInfoPos(QLabel *label, const QPoint &pos = QPoint());
	void updateInfoOriginal(const QImage &origImage);
	void updateInfoConverted();
	void updatePropertiesWarning();

	void reloadTextureToScene(DAVA::Texture *texture, const DAVA::TextureDescriptor *descriptor, DAVA::ImageFileFormat format);

private slots:
	void textureListViewImages(bool checked);
	void textureListViewText(bool checked);
	void textureListFilterChanged(const QString &text);
	void textureListFilterSelectedNodeChanged(bool checked);
	void textureListSortChanged(const QString &text);
	void texturePressed(const QModelIndex & index);
	void textureColorChannelPressed(bool checked);
	void textureBorderPressed(bool checked);
	void textureBgMaskPressed(bool checked);
	void texturePropertyChanged(const int propGroup);
	void textureViewPVR(bool checked);
	void textureViewDXT(bool checked);
	void textureReadyOriginal(const DAVA::TextureDescriptor *descriptor, const QImage &image);
	void textureReadyPVR(const DAVA::TextureDescriptor *descriptor, const QImage &image);
	void textureReadyDXT(const DAVA::TextureDescriptor *descriptor, const QImage &image);
	void texturePixelOver(const QPoint &pos);
	void textureZoomSlide(int value);
	void textureZoom100(bool checked);
	void textureZoomFit(bool checked);
	void textureAreaWheel(int delta);
	void textureConverAll();

	void convertStatus(const JobItem *jobCur, int jobLeft);
};

#endif // __TEXTURE_BROWSER_H__
