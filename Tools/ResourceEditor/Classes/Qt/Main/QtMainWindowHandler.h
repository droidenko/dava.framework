#ifndef __QT_MAIN_WINDOW_HANDLER_H__
#define __QT_MAIN_WINDOW_HANDLER_H__

#include <QObject>
#include <QPoint>
#include <QVector>
#include <QAbstractButton>
#include <QRadioButton.h>

#include "DAVAEngine.h"
#include "Constants.h"
#include "SceneEditor/EditorSettings.h"

#include "Qt/TextureBrowser/TextureBrowser.h"
#include "Qt/MaterialBrowser/MaterialBrowser.h"
#include "Qt/DockSetSwitchIndex/SetSwitchIndexHelper.h"

class Command;
class QMenu;
class QAction;
class QTreeView;
class QStatusBar;
class QPushButton;
class QSlider;
class QComboBox;
class ModificationWidget;
class QSpinBox;
class QCheckBox;
class QDoubleSpinBox;

class QtMainWindowHandler: public QObject, public DAVA::Singleton<QtMainWindowHandler>
{
    Q_OBJECT
    
public:
    QtMainWindowHandler(QObject *parent = 0);
    virtual ~QtMainWindowHandler();

    void RegisterNodeActions(DAVA::int32 count, ...);
    void RegisterViewportActions(DAVA::int32 count, ...);
    void RegisterDockActions(DAVA::int32 count, ...);
    void RegisterTextureFormatActions(DAVA::int32 count, ...);
	void RegisterModificationActions(DAVA::int32 count, ...);
	void RegisterEditActions(DAVA::int32 count, ...);

    void SetResentMenu(QMenu *menu);

    //MENU FILE
    void UpdateRecentScenesList();

	void SetDefaultFocusWidget(QWidget *widget);
	void RestoreDefaultFocus();
    
    void RegisterStatusBar(QStatusBar *registeredSatusBar);
    void ShowStatusBarMessage(const DAVA::String &message, DAVA::int32 displayTime = 0);
    
    void SetWaitingCursorEnabled(bool enabled);
    
	//custom colors
	void RegisterCustomColorsWidgets(QPushButton*, QPushButton*, QSlider*, QComboBox*, QPushButton*);
    void SetCustomColorsWidgetsState(bool state);

	//set switch index
	void RegisterSetSwitchIndexWidgets(QSpinBox*, QRadioButton*, QRadioButton*, QPushButton*);
    void SetSwitchIndexWidgetsState(bool state);

	//material view options
	void RegisterMaterialViewOptionsWidgets(QComboBox*);
	void SetMaterialViewOptionsWidgetsState(bool state);
	void SelectMaterialViewOption(Material::eViewOptions value);

	//hanging objects
	void RegisterHangingObjectsWidgets(QCheckBox*, QDoubleSpinBox*, QPushButton*);
    void SetHangingObjectsWidgetsState(bool state);

	//visibility check tool
	void RegisterWidgetsVisibilityTool(QPushButton*, QPushButton*, QPushButton*, QPushButton*, QSlider*);
	void SetWidgetsStateVisibilityTool(bool state);
	void SetPointButtonStateVisibilityTool(bool state);
	void SetAreaButtonStateVisibilityTool(bool state);

	void UpdateUndoActionsState();

public slots:
    void CreateNodeTriggered(QAction *nodeAction);
    void ViewportTriggered(QAction *viewportAction);
    void FileMenuTriggered(QAction *resentScene);

    //File
    void NewScene();
    void OpenScene();
    void OpenProject();
    void OpenResentScene(DAVA::int32 index);
    void SaveScene();
    void ExportAsPNG();
    void ExportAsPVR();
    void ExportAsDXT();
    void SaveToFolderWithChilds();

	//Edit
	void UndoAction();
	void RedoAction();
	void ConvertToShadow();

    //View
    void RestoreViews();

    //tools
    void Materials();
    void ConvertTextures();
    void HeightmapEditor();
    void TilemapEditor();
    void RulerTool();
    void ShowSettings();
    void Beast();
    
    //ViewOptions
    void MenuViewOptionsWillShow();
    void ToggleNotPassableTerrain();
    void ReloadAsPNG();
    void ReloadAsPVR();
    void ReloadAsDXT();

    
    //scene graph
    void RefreshSceneGraph();
    
	//set switch index
	void ToggleSetSwitchIndex(DAVA::uint32  value, DAVA::SetSwitchIndexHelper::eSET_SWITCH_INDEX state);

	//material view options
	void MaterialViewOptionChanged(int index);

	//hanging objects
	void ToggleHangingObjects(float value, bool isEnabled);

    //custom colors
    void ToggleCustomColors();
    void SaveTextureCustomColors();
    void ChangeBrushSizeCustomColors(int newSize);
    void ChangeColorCustomColors(int newColorIndex);
	void LoadTextureCustomColors();
	
	//visibility check tool
	void ToggleVisibilityTool();
	void SaveTextureVisibilityTool();
	void ChangleAreaSizeVisibilityTool(int newSize);
	void SetVisibilityPointVisibilityTool();
	void SetVisibilityAreaVisibilityTool();

    //
    void RepackAndReloadTextures();

	//particles editor
	void CreateParticleEmitterNode();
	
	//modification options
	void ModificationSelect();
	void ModificationMove();
	void ModificationRotate();
	void ModificationScale();
	void ModificationPlaceOnLand();
	void ModificationSnapToLand();
	void OnApplyModification(double x, double y, double z);
	void OnResetModification();
	void SetModificationMode(ResourceEditor::eModificationActions mode);

	void OnSceneActivated(SceneData *scene);
	void OnSceneReleased(SceneData *scene);
	void OnSceneCreated(SceneData *scene);

	void ReloadSceneTextures();

signals:
	void ProjectChanged();

private:
    //create node
    void CreateNode(ResourceEditor::eNodeType type);
    //viewport
    void SetViewport(ResourceEditor::eViewportType type);

    void RegisterActions(QAction **actions, DAVA::int32 count, va_list &vl);
    
    void ClearActions(int32 count, QAction **actions);

	void UpdateModificationActions();

private:
	//set switch index
	QPushButton*	setSwitchIndexToggleButton;
	QSpinBox*		editSwitchIndexValue;
	QRadioButton*	rBtnSelection;
	QRadioButton*	rBtnScene;

	//material view options
	QComboBox* comboMaterialViewOption;

	//hanging objects
	QPushButton*	hangingObjectsToggleButton;
	QDoubleSpinBox*	editHangingObjectsValue;
	QCheckBox *		checkBoxHangingObjects;

	//custom colors
	QPushButton* customColorsToggleButton;
	QPushButton* customColorsSaveTextureButton;
	QSlider* customColorsBrushSizeSlider;
	QComboBox* customColorsColorComboBox;
	QPushButton* customColorsLoadTextureButton;
	
	//visibility check tool
	QPushButton* visibilityToolToggleButton;
	QPushButton* visibilityToolSaveTextureButton;
	QPushButton* visibilityToolSetPointButton;
	QPushButton* visibilityToolSetAreaButton;
	QSlider* visibilityToolAreaSizeSlider;
    
    QAction *resentSceneActions[EditorSettings::RESENT_FILES_COUNT];
    QAction *nodeActions[ResourceEditor::NODE_COUNT];
    QAction *viewportActions[ResourceEditor::VIEWPORT_COUNT];
    QAction *hidablewidgetActions[ResourceEditor::HIDABLEWIDGET_COUNT];
    QAction *textureFileFormatActions[DAVA::FILE_FORMAT_COUNT];
	QAction *modificationActions[ResourceEditor::MODIFY_COUNT];
	QAction *editActions[ResourceEditor::EDIT_COUNT];

    QMenu *menuResentScenes;
	QWidget *defaultFocusWidget;
    
    QStatusBar *statusBar;
};

#endif // __QT_MAIN_WINDOW_HANDLER_H__
