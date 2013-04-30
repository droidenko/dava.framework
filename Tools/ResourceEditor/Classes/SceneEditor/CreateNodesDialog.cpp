#include "CreateNodesDialog.h"
#include "ControlsFactory.h"

#include "NodesPropertyControl.h"
#include "PropertyControlCreator.h"
#include "EditorScene.h"
#include "SceneEditorScreenMain.h"
#include "AppScreens.h"
#include "EditorBodyControl.h"
#include "Scene3D/Components/UserComponent.h"
#include "EditorSettings.h"

CreateNodesDialog::CreateNodesDialog(const Rect & rect)
    :   DraggableDialog(rect)
{
    ControlsFactory::CustomizeDialog(this);

    sceneNode = NULL;
    scene = NULL;
    
    dialogDelegate = NULL;
	propertyList = 0;
    
    header = new UIStaticText(Rect(0, 0, rect.dx, ControlsFactory::BUTTON_HEIGHT));
    header->SetFont(ControlsFactory::GetFont12());
	header->SetTextColor(ControlsFactory::GetColorLight());
    header->SetAlign(ALIGN_HCENTER | ALIGN_VCENTER);
    AddControl(header);
    
    float32 buttonY = rect.dy - ControlsFactory::BUTTON_HEIGHT;
    float32 buttonX = (rect.dx - ControlsFactory::BUTTON_WIDTH * 2) / 2;
    
    UIButton *btnCancel = ControlsFactory::CreateButton(Vector2(buttonX, buttonY), LocalizedString(L"dialog.cancel"));
    btnCancel->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &CreateNodesDialog::OnCancel));
    AddControl(btnCancel);
    
    buttonX += ControlsFactory::BUTTON_WIDTH;
    UIButton *btnOk = ControlsFactory::CreateButton(Vector2(buttonX, buttonY), LocalizedString(L"dialog.ok"));
    btnOk->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &CreateNodesDialog::OnOk));
    AddControl(btnOk);
    
    propertyRect = Rect(0, ControlsFactory::BUTTON_HEIGHT, rect.dx, buttonY - ControlsFactory::BUTTON_HEIGHT);

    SafeRelease(btnCancel);
    SafeRelease(btnOk);
}
    
CreateNodesDialog::~CreateNodesDialog()
{
    SafeRelease(sceneNode);

    SafeRelease(header);
    SafeRelease(propertyList);
    dialogDelegate = NULL;
}

void CreateNodesDialog::SetDelegate(CreateNodesDialogDelegeate *delegate)
{
    dialogDelegate = delegate;
}

void CreateNodesDialog::OnCancel(BaseObject *, void *, void *)
{
    if(dialogDelegate)
    {
        dialogDelegate->DialogClosed(RCODE_CANCEL);
    }
}

void CreateNodesDialog::OnOk(BaseObject *, void *, void *)
{    
    if(dialogDelegate)
    {
        dialogDelegate->DialogClosed(RCODE_OK);
    }
}

Entity * CreateNodesDialog::GetSceneNode()
{
    return sceneNode;
}


void CreateNodesDialog::SetScene(Scene *_scene)
{
    scene = _scene;
    propertyList->SetWorkingScene(scene);
}

void CreateNodesDialog::SetHeader(const WideString &headerText)
{
    header->SetText(headerText);
}

void CreateNodesDialog::CreateNode(ResourceEditor::eNodeType nodeType)
{
    SafeRelease(sceneNode);

	SceneEditorScreenMain * screen = (SceneEditorScreenMain*)UIScreenManager::Instance()->GetScreen(SCREEN_MAIN);
	EditorScene * editorScene = screen->FindCurrentBody()->bodyControl->GetScene();
	scene = editorScene;

    switch (nodeType) 
    {
        case ResourceEditor::NODE_LANDSCAPE:
            SetHeader(LocalizedString(L"createnode.landscape"));
            sceneNode = new Entity();
            sceneNode->AddComponent(new RenderComponent(ScopedPtr<Landscape>(new Landscape())));
            sceneNode->SetName("Landscape");
            break;

        case ResourceEditor::NODE_LIGHT:
        {
            SetHeader(LocalizedString(L"createnode.light"));
            
            //sceneNode = //EditorLightNode::CreateSceneAndEditorLight();
            sceneNode = new Entity();
            sceneNode->AddComponent(new LightComponent(ScopedPtr<Light>(new Light)));
            sceneNode->SetName("Light");
            break;
        }

        case ResourceEditor::NODE_SERVICE_NODE:
        {
            SetHeader(LocalizedString(L"createnode.servicenode"));
            sceneNode = new Entity();
            KeyedArchive *customProperties = sceneNode->GetCustomProperties();
            customProperties->SetBool("editor.isLocked", true);
            sceneNode->SetName("Servicenode");
            break;
        }

        case ResourceEditor::NODE_CAMERA:
        {
            SetHeader(LocalizedString(L"createnode.camera"));
            sceneNode = new Entity();
            
            Camera * camera = new Camera();
            camera->SetUp(Vector3(0.0f, 0.0f, 1.0f));
            sceneNode->AddComponent(new CameraComponent(camera));
            sceneNode->SetName("Camera");
            SafeRelease(camera);
        }break;

		case ResourceEditor::NODE_IMPOSTER:
			SetHeader(LocalizedString(L"createnode.imposter"));
			sceneNode = new ImposterNode();
			sceneNode->SetName("Imposter");
			break;

		case ResourceEditor::NODE_PARTICLE_EMITTER:
		{
			SetHeader(LocalizedString(L"createnode.particleemitter"));
			sceneNode = new Entity();
			sceneNode->SetName("Particle Emitter");

			ParticleEmitter3D* newEmitter = new ParticleEmitter3D();

			RenderComponent * renderComponent = new RenderComponent();
			renderComponent->SetRenderObject(newEmitter);
			sceneNode->AddComponent(renderComponent);
            
            newEmitter->Release();

			break;
		}

		case ResourceEditor::NODE_USER_NODE:
        {
			SetHeader(LocalizedString(L"createnode.usernode"));
			sceneNode = new Entity();
			sceneNode->SetName("UserNode");
			sceneNode->AddComponent(new UserComponent());
			break;
        }

		case ResourceEditor::NODE_SWITCH_NODE:
		{
			SetHeader(LocalizedString(L"createnode.switchnode"));
            sceneNode = new Entity();
			sceneNode->SetName("SwitchNode");
            sceneNode->AddComponent(new SwitchComponent());
            
			KeyedArchive *customProperties = sceneNode->GetCustomProperties();
			customProperties->SetBool(Entity::SCENE_NODE_IS_SOLID_PROPERTY_NAME, false);
		}
			break;


		case ResourceEditor::NODE_PARTICLE_EFFECT:
		{
			SetHeader(L"Particle Effect");

			sceneNode = new Entity();
			ParticleEffectComponent* newEffectComponent = new ParticleEffectComponent();
			sceneNode->AddComponent(newEffectComponent);
			sceneNode->SetName("Particle Effect");

			break;
		}

        default:
            break;
    }

	propertyList = PropertyControlCreator::Instance()->CreateControlForNode(sceneNode, propertyRect, true);
    SafeRetain(propertyList);
	AddControl(propertyList);

	SetScene(editorScene);
    
    propertyList->ReadFrom(sceneNode);
}

void CreateNodesDialog::WillDisappear()
{
	RemoveControl(propertyList);
	SafeRelease(propertyList);
}

