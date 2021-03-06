#include "UltralightImpl.h"

// + Ultralight
#include "AppCore/JSHelpers.h"
#include "AppCore/Platform.h"
#include "AppCore/Overlay.h"
#include "AppCore/CAPI.h"

#include "JavaScriptCore/JSRetainPtr.h"
#include "JavaScriptCore/JavaScript.h"

#include "Ultralight/Geometry.h"
#include "Ultralight/Defines.h" 
#include "Ultralight/String.h"
#include "Ultralight/Matrix.h"
#include "Ultralight/Bitmap.h"
#include "Ultralight/Ultralight.h"
// - Ultralight

// + Unigine
#include <UnigineMeshDynamic.h>
#include <UnigineMaterials.h>
#include <UnigineTextures.h>
#include <UnigineRender.h>
#include <UnigineApp.h>
// - Unigine

using namespace Unigine;
using namespace ultralight;
using namespace KeyCodes;

// + UltralightSharp
RefPtr<View> view;
RefPtr<Bitmap> bitmap;
RefPtr<ultralight::Renderer> renderer;
// - UltralightSharp

// + Unigine
static BlobPtr blob;
static TexturePtr texture;
static TexturePtr font_texture;
static MeshDynamicPtr Ultralight_mesh;
static MaterialPtr Ultralight_material;

static int saved_mouse = 0;

GuiPtr gui;
WidgetSpritePtr hud;
TexturePtr my_texture;
TexturePtr my_texture_ds;
WidgetLabelPtr widget_label;

int Ultralight_Gui_Id;
// - Unigine

class MyListener : public LoadListener {
public:
	MyListener() {}
	virtual ~MyListener() {}

	JSValue GetMessage(const JSObject& thisObject, const JSArgs& args) {
		//auto lol = JSStringCreateWithUTF8CString();

		///
		/// Return our message to JavaScript as a JSValue.
		///
		return JSValue("Hello from C++!<br/>Ultralight rocks!");
	}

	///
	/// Inherited from LoadListener, called when the page has finished parsing
	/// the document.
	///
	/// We perform all our JavaScript initialization here.
	///
	virtual void OnDOMReady(ultralight::View* caller,
		uint64_t frame_id,
		bool is_main_frame,
		const ultralight::String& url) override {
		///
		/// Set our View's JSContext as the one to use in subsequent JSHelper calls
		///
		Ref<JSContext> context = caller->LockJSContext();
		SetJSContext(context.get());

		///
		/// Get the global object (this would be the "window" object in JS)
		///
		JSObject global = JSGlobalObject();

		///
		/// Bind MyApp::GetMessage to the JavaScript function named "GetMessage".
		///
		/// You can get/set properties of JSObjects by using the [] operator with
		/// the following types as potential property values:
		///  - JSValue
		///      Represents a JavaScript value, eg String, Object, Function, etc.
		///  - JSCallback 
		///      Typedef of std::function<void(const JSObject&, const JSArgs&)>)
		///  - JSCallbackWithRetval 
		///      Typedef of std::function<JSValue(const JSObject&, const JSArgs&)>)
		///
		/// We use the BindJSCallbackWithRetval macro to bind our C++ class member
		/// function to our JavaScript callback.
		///
		global["GetMessage"] = BindJSCallbackWithRetval(&MyListener::GetMessage);
	}
};

bool mouse_is_over_unigine_widget(WidgetPtr w)
{
	if (w->isHidden())
		return false;

	bool r = ((gui->getMouseX() > 0) && (gui->getMouseY() > 0) && ((gui->getMouseX() < hud->getWidth()) && (gui->getMouseY() < hud->getHeight())));
	{
		hud->setHidden(!r);
		return r;
	}
}

const int KeyToInt(unsigned int key)
{
	int ConvertedKey = 0;

	switch (key)
	{
	case App::KEY_ESC:
		ConvertedKey = GK_ESCAPE;
		break;
	case App::KEY_TAB:
		ConvertedKey = GK_TAB;
		break;
	case App::KEY_BACKSPACE:
		ConvertedKey = GK_BACK;
		break;
	case App::KEY_RETURN:
		ConvertedKey = GK_RETURN;
		break;
	case App::KEY_DELETE:
		ConvertedKey = GK_DELETE;
		break;
	case App::KEY_INSERT:
		ConvertedKey = GK_INSERT;
		break;
	case App::KEY_HOME:
		ConvertedKey = GK_HOME;
		break;
	case App::KEY_END:
		ConvertedKey = GK_END;
		break;
	case App::KEY_PGUP:
		ConvertedKey = GK_PRIOR;
		break;
	case App::KEY_PGDOWN:
		ConvertedKey = GK_NEXT;
		break;
	case App::KEY_LEFT:
		ConvertedKey = GK_LEFT;
		break;
	case App::KEY_RIGHT:
		ConvertedKey = GK_RIGHT;
		break;
	case App::KEY_UP:
		ConvertedKey = GK_UP;
		break;
	case App::KEY_DOWN:
		ConvertedKey = GK_DOWN;
		break;
	case App::KEY_SHIFT:
		ConvertedKey = GK_SHIFT;
		break;
	case App::KEY_CTRL:
		ConvertedKey = GK_CONTROL;
		break;
	case App::KEY_CMD:
		ConvertedKey = GK_LWIN;
		break;
	case App::KEY_SCROLL:
		ConvertedKey = GK_SCROLL;
		break;
	case App::KEY_CAPS:
		ConvertedKey = GK_CAPITAL;
		break;
	case App::KEY_NUM:
		ConvertedKey = GK_NUMLOCK;
		break;
	case App::KEY_F1:
		ConvertedKey = GK_F1;
		break;
	case App::KEY_F2:
		ConvertedKey = GK_F2;
		break;
	case App::KEY_F3:
		ConvertedKey = GK_F3;
		break;
	case App::KEY_F4:
		ConvertedKey = GK_F4;
		break;
	case App::KEY_F5:
		ConvertedKey = GK_F5;
		break;
	case App::KEY_F6:
		ConvertedKey = GK_F6;
		break;
	case App::KEY_F7:
		ConvertedKey = GK_F7;
		break;
	case App::KEY_F8:
		ConvertedKey = GK_F8;
		break;
	case App::KEY_F9:
		ConvertedKey = GK_F9;
		break;
	case App::KEY_F10:
		ConvertedKey = GK_F10;
		break;
	case App::KEY_F11:
		ConvertedKey = GK_F11;
		break;
	case App::KEY_F12:
		ConvertedKey = GK_F12;
		break;
	case App::NUM_KEYS:
		ConvertedKey = GK_NUMPAD0;
		break;
	}

	return ConvertedKey;
}

void HandleMouse()
{
	MouseEvent evt;
	evt.type = MouseEvent::kType_MouseMoved;
	evt.x = App::getMouseX();
	evt.y = App::getMouseY();
	evt.button = MouseEvent::kButton_None;

	view->FireMouseEvent(evt);
}

static int on_button_pressed(int button)
{
	MouseEvent evt;
	evt.type = MouseEvent::kType_MouseDown;
	evt.x = App::getMouseX();
	evt.y = App::getMouseY();

	switch (button)
	{
	case App::BUTTON_LEFT:
		evt.button = MouseEvent::kButton_Left;
		break;
	case App::BUTTON_RIGHT:
		evt.button = MouseEvent::kButton_Left;
		break;
	case App::BUTTON_MIDDLE:
		evt.button = MouseEvent::kButton_Middle;
		break;
	}

	if (mouse_is_over_unigine_widget(gui->getChild(Ultralight_Gui_Id)))
	{
		saved_mouse = App::getMouseButton();
		App::setMouseButton(0);
		Gui::get()->setMouseButton(0);
	}

	view->FireMouseEvent(evt);

	return 0;
}

static int on_button_released(int button)
{
	MouseEvent evt;
	evt.type = MouseEvent::kType_MouseUp;
	evt.x = App::getMouseX();
	evt.y = App::getMouseY();

	switch (button)
	{
	case App::BUTTON_LEFT:
		evt.button = MouseEvent::kButton_Left;
		break;
	case App::BUTTON_RIGHT:
		evt.button = MouseEvent::kButton_Right;
		break;
	case App::BUTTON_MIDDLE:
		evt.button = MouseEvent::kButton_Middle;
		break;
	}

	if (mouse_is_over_unigine_widget(gui->getChild(Ultralight_Gui_Id)))
	{
		Gui::get()->setMouseButton(saved_mouse);
	}


	view->FireMouseEvent(evt);

	return 0;
}

static int on_key_pressed(unsigned int key)
{
	if (key == App::KEY_ESC)
		hud->setHidden(gui->isHidden());

	// Synthesize a key press event for the 'Right Arrow' key
	KeyEvent evt;
	evt.type = KeyEvent::kType_RawKeyDown;
	evt.virtual_key_code = KeyToInt(key);
	evt.native_key_code = KeyToInt(key);
	evt.modifiers = 0;

	// You'll need to generate a key identifier from the virtual key code
	// when synthesizing events. This function is provided in KeyEvent.h
	GetKeyIdentifierFromVirtualKeyCode(evt.virtual_key_code, evt.key_identifier);

	view->FireKeyEvent(evt);

	return 0;
}

static int on_key_released(unsigned int key)
{
	// Synthesize a key press event for the 'Right Arrow' key
	KeyEvent evt;
	evt.type = KeyEvent::kType_KeyUp;
	evt.virtual_key_code = KeyToInt(key);//KeyCodes::GK_TAB;
	evt.native_key_code = KeyToInt(key);
	evt.modifiers = 0;

	// You'll need to generate a key identifier from the virtual key code
	// when synthesizing events. This function is provided in KeyEvent.h
	GetKeyIdentifierFromVirtualKeyCode(evt.virtual_key_code, evt.key_identifier);

	view->FireKeyEvent(evt);

	return 0;
}

static int on_unicode_key_pressed(unsigned int key)
{
		if (key < App::KEY_ESC || key >= App::NUM_KEYS)
		{
			char ch = static_cast<char>(key);
			ultralight::String ConvertedKey(&ch, 1);

			// Synthesize an  event for text generated from pressing the 'A' key
			KeyEvent evt;
			evt.type = KeyEvent::kType_Char;
			evt.text = ConvertedKey;
			evt.unmodified_text = ConvertedKey; // If not available, set to same as evt.text

			view->FireKeyEvent(evt);
		}
	return 0;
}

void UltralightImpl::Init() {
	Config config;

	App::setKeyPressFunc(on_key_pressed);
	App::setKeyReleaseFunc(on_key_released);
	App::setButtonPressFunc(on_button_pressed);
	App::setButtonReleaseFunc(on_button_released);
	App::setKeyPressUnicodeFunc(on_unicode_key_pressed);

	InitPlatform();
	CreateRenderer();
	CreateView();
	
	///
	/// We need to tell config where our resources are so it can 
	/// load our bundled SSL certificates to make HTTPS requests.
	///
	config.resource_path = "./resources/";


	///
	/// The GPU renderer should be disabled to render Views to a 
	/// pixel-buffer (Surface).
	///
	config.use_gpu_renderer = false;

	///
	/// You can set a custom DPI scale here. Default is 1.0 (100%)
	///
	config.device_scale = 1.0;

	///
	/// Pass our configuration to the Platform singleton so that
	/// the library can use it.
	///
	Platform::instance().set_config(config);

	gui = Gui::get();
	texture = Texture::create();
	blob = Blob::create();

	//Platform::instance().set_gpu_driver(custom_gpu_driver_instance);

	createHUDWidgetSprite();
}

const char* Test()
{
	return R"(
<!DOCTYPE html>
<html>
<head>
<style>
body {
  background-color: transparent;
}
</style>
</head>
<body>

<h1>The background-color Property</h1>

<p>The background color can be specified with a color name.</p>

</body>
</html>

)";
}

const char* Sample1() {
	return R"(
    <html>
      <head>
        <style type="text/css">
          body {
            margin: 0;
            padding: 0;
            overflow: hidden;
            color: black;
            font-family: Arial;
            background: linear-gradient(-45deg, #acb4ff, #f5d4e2);
            display: flex;
            justify-content: center;
            align-items: center;
          }
          div {
            width: 350px;
            height: 350px;
            text-align: center;
            border-radius: 25px;
            background: linear-gradient(-45deg, #e5eaf9, #f9eaf6);
            box-shadow: 0 7px 18px -6px #8f8ae1;
          }
          h1 {
            padding: 1em;
          }
          p {
            background: white;
            padding: 2em;
            margin: 40px;
            border-radius: 25px;
          }
        </style>
      </head>
      <body>
        <div>
          <h1>Hello World!</h1>
          <p>Welcome to Ultralight!</p>

        </div>

      </body>
    </html>
    )";
}

const char* Sample2() {
	return R"(
<html>
  <head>
    <style type="text/css">
    * { -webkit-user-select: none; }
    body { 
      overflow: hidden;
      margin: 0;
      padding: 0;
      background-color: #e0e3ed;
      background: linear-gradient(-45deg, #e0e3ed, #f7f9fc);
      width: 900px;
      height: 600px;
      font-family: -apple-system, 'Segoe UI', Ubuntu, Arial, sans-serif;
    }
    h2, h3 {
      margin: 0;
    }
    div {
      padding: 35px;
      margin: 10px;
      height: 510px;
      width: 360px;
    }
    p, li { 
      font-size: 1em;
    }
    #leftPane {
      float: left;
      color: #858998;
      padding: 85px 65px;
      height: 410px;
      width: 300px;
    }
    #leftPane p {
      color: #858998;
    }
    #rightPane {
      border-radius: 15px;
      background-color: white;
      float: right;
      color: #22283d;
      box-shadow: 0 7px 24px -6px #aaacb8;
    }
    #rightPane li, #rightPane p {
      color: #7e7f8e;
      font-size: 0.9em;
    }
    #rightPane li {
      list-style-type: none;
      padding: 0.6em 0;
      border-radius: 20px;
      margin: 0;
      padding-left: 1em;
      cursor: pointer;
    }
    #rightPane li:hover {
      background-color: #f4f6fb;
    }
    li:before {
      content: '';
      display:inline-block; 
      height: 18; 
      width: 18;
      margin-bottom: -5px; 
      margin-right: 1em;
      background-image: url("data:image/svg+xml;utf8,<svg xmlns=\
'http://www.w3.org/2000/svg' width='18' height='18' viewBox='-2 -2 27 27'>\
<path stroke='%23dbe2e7' stroke-width='2' fill='white' d='M12 0c-6.627 0-12 \
5.373-12 12s5.373 12 12 12 12-5.373 12-12-5.373-12-12-12z'/></svg>");
    }
    li.checked:before {
      background-image: url("data:image/svg+xml;utf8,<svg xmlns=\
'http://www.w3.org/2000/svg' width='18' height='18' viewBox='0 0 24 24'><path \
fill='%2334d7d6' d='M12 0c-6.627 0-12 5.373-12 12s5.373 12 12 12 12-5.373 \
12-12-5.373-12-12-12zm-1.25 17.292l-4.5-4.364 1.857-1.858 2.643 2.506 \
5.643-5.784 1.857 1.857-7.5 7.643z'/></svg>");
    }
    #rightPane h5 {
      border-bottom: 1px solid #eceef0;
      padding-bottom: 9px;
      margin-bottom: 1em;
      margin-top: 3em;
    }
    #rightPane h5 {
      padding-left: 1em;
    }
    #rightPane ul {
      padding-left: 0;
    }
    </style>
    <script>
      window.onload = function() {
        var listItems = document.getElementsByTagName('li');
        for(var i = 0; i < listItems.length; i++) {
          listItems[i].onclick = function() {
            this.classList.toggle('checked');
          }
        }
      }
  </script>
  </head>
  <body>
    <div id="leftPane">
      <h2>My Planner App</h2>
      <p>Welcome to Ultralight Tutorial 2!</p>
    </div>
    <div id="rightPane">
      <h3>Upcoming Tasks</h3>
      <p>Click a task to mark it as completed.</p>
      <h5>Today</h5>
      <ul>
        <li class="checked">Create layout for initial mockup</li>
        <li class="checked">Select icons for mobile interface</li>
        <li class="checked">Discussions regarding new sorting algorithm</li>
        <li class="checked">Call with automotive clients</li>
        <li>Create quote for the Tesla remodel</li>
      </ul>
      <h5>Upcoming</h5>
      <ul>
        <li>Plan itinerary for conference</li>
        <li>Discuss desktop workflow optimizations</li>
        <li>Performance improvement analysis</li>
      </ul>
    </div>
  </body>
</html>
)";
}

const char* Sample4() {
	return R"(
<html>
  <head>
    <style type="text/css">
      * { -webkit-user-select: none; }
      body { 
        font-family: -apple-system, 'Segoe UI', Ubuntu, Arial, sans-serif; 
        text-align: center;
        background: linear-gradient(#FFF, #DDD);
        padding: 2em;
      }
      body.rainbow {
        background: linear-gradient(90deg, #ff2363, #fff175, #68ff9d, 
                                           #45dce0, #6c6eff, #9e23ff, #ff3091);
        background-size: 1000% 1000%;
        animation: ScrollGradient 10s ease infinite;
      }
      @keyframes ScrollGradient {
        0%   { background-position:0% 50%; }
        50%  { background-position:100% 50%; }
        100% { background-position:0% 50%; }
      }
      #message {
        padding-top: 2em;
        color: white;
        font-weight: bold;
        font-size: 24px;
        text-shadow: 1px 1px rgba(0, 0, 0, 0.4);
      }
    </style>
    <script type="text/javascript">
    function HandleButton(evt) {
      // Call our C++ callback 'GetMessage'
      var message = GetMessage();
      
      // Display the result in our 'message' div element and apply the
      // rainbow effect to our document's body.
      document.getElementById('message').innerHTML = message;
      document.body.classList.add('rainbow');
    }
    </script>
  </head>
  <body>
    <button onclick="HandleButton(event);">Get the Secret Message!</button>
    <div id="message"></div>
  </body>
</html>
    )";
}

///
/// This is called continuously from the app's main run loop. You should
/// update any app logic inside this callback.
///
void OnUpdate(void* user_data) {
	/// We don't use this in this tutorial, just here for example.
}

///
/// This is called whenever the window resizes. Width and height are in
/// DPI-independent logical coordinates (not pixels).
///
void OnResize(void* user_data, unsigned int width, unsigned int height) {
	//ulOverlayResize(overlay, width, height);
}

void UltralightImpl::InitPlatform() {
	/// 
	/// Use the OS's native font loader
	///
	Platform::instance().set_font_loader(GetPlatformFontLoader());

	///
	/// Use the OS's native file loader, with a base directory of "."
	/// All file:/// URLs will load relative to this base directory.
	///
	Platform::instance().set_file_system(GetPlatformFileSystem("./resources/"));

	///
	/// Use the default logger (writes to a log file)
	///
	Platform::instance().set_logger(GetDefaultLogger("ultralight.log"));
}

bool UltralightImpl::CreateRenderer() {
	///
	/// Create our Renderer (call this only once per application).
	/// 
	/// The Renderer singleton maintains the lifetime of the library
	/// and is required before creating any Views.
	///
	/// You should set up the Platform handlers before this.
	///
	renderer = ultralight::Renderer::Create();
	if (renderer.get() == nullptr)
	{
		return false;
	}

	return true;
}

static void create_imgui_mesh()
{
	Ultralight_mesh = MeshDynamic::create(MeshDynamic::DYNAMIC_ALL);

	MeshDynamic::Attribute attributes[3]{};
	attributes[0].offset = 0;
	attributes[0].size = 2;
	attributes[0].type = MeshDynamic::TYPE_FLOAT;
	attributes[1].offset = 8;
	attributes[1].size = 2;
	attributes[1].type = MeshDynamic::TYPE_FLOAT;
	attributes[2].offset = 16;
	attributes[2].size = 4;
	attributes[2].type = MeshDynamic::TYPE_UCHAR;
	Ultralight_mesh->setVertexFormat(attributes, 3);

	//assert(imgui_mesh->getVertexSize() == sizeof(ImDrawVert) && "Vertex size of MeshDynamic is not equal to size of ImDrawVert");
}

static void create_imgui_material()
{
	Ultralight_material = Materials::findMaterial("ultralight")->inherit();
}

static void draw_callback()
{
	//if (frame_draw_data == nullptr)
	//	return;

	//auto draw_data = frame_draw_data;
	//frame_draw_data = nullptr;

	//if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f)
	//	return;


	//// Write vertex and index data into dynamic mesh
	//imgui_mesh->clearVertex();
	//imgui_mesh->clearIndices();
	//imgui_mesh->allocateVertex(draw_data->TotalVtxCount);
	//imgui_mesh->allocateIndices(draw_data->TotalIdxCount);
	//for (int i = 0; i < draw_data->CmdListsCount; ++i)
	//{
	//	const ImDrawList* cmd_list = draw_data->CmdLists[i];

	//	imgui_mesh->addVertexArray(cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size);
	//	imgui_mesh->addIndicesArray(cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size);
	//}
	//imgui_mesh->flushVertex();
	//imgui_mesh->flushIndices();


	//auto render_target = Render::getTemporaryRenderTarget();
	//render_target->bindColorTexture(0, Renderer::getTextureColor());

	//// Render state
	//RenderState::saveState();
	//RenderState::clearStates();
	//RenderState::setBlendFunc(RenderState::BLEND_SRC_ALPHA, RenderState::BLEND_ONE_MINUS_SRC_ALPHA, RenderState::BLEND_OP_ADD);
	//RenderState::setPolygonCull(RenderState::CULL_NONE);
	//RenderState::setDepthFunc(RenderState::DEPTH_NONE);
	//RenderState::setViewport(static_cast<int>(draw_data->DisplayPos.x), static_cast<int>(draw_data->DisplayPos.y),
	//	static_cast<int>(draw_data->DisplaySize.x), static_cast<int>(draw_data->DisplaySize.y));

	//// Orthographic projection matrix
	//float left = draw_data->DisplayPos.x;
	//float right = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
	//float top = draw_data->DisplayPos.y;
	//float bottom = draw_data->DisplayPos.y + draw_data->DisplaySize.y;

	//Math::Mat4 proj;
	//proj.m00 = 2.0f / (right - left);
	//proj.m03 = (right + left) / (left - right);
	//proj.m11 = 2.0f / (top - bottom);
	//proj.m13 = (top + bottom) / (bottom - top);
	//proj.m22 = 0.5f;
	//proj.m23 = 0.5f;
	//proj.m33 = 1.0f;

	//Renderer::setProjection(proj);
	//auto shader = imgui_material->fetchShader("imgui");
	//auto pass = imgui_material->getRenderPass("imgui");
	//Renderer::setShaderParameters(pass, shader, imgui_material, false);


	//imgui_mesh->bind();
	//render_target->enable();
	//{
	//	int global_idx_offset = 0;
	//	int global_vtx_offset = 0;
	//	ImVec2 clip_off = draw_data->DisplayPos;
	//	// Draw command lists
	//	for (int i = 0; i < draw_data->CmdListsCount; ++i)
	//	{
	//		const ImDrawList* cmd_list = draw_data->CmdLists[i];
	//		for (int j = 0; j < cmd_list->CmdBuffer.Size; ++j)
	//		{
	//			const ImDrawCmd* cmd = &cmd_list->CmdBuffer[j];

	//			if (cmd->UserCallback != nullptr)
	//			{
	//				if (cmd->UserCallback != ImDrawCallback_ResetRenderState)
	//					cmd->UserCallback(cmd_list, cmd);
	//			}
	//			else
	//			{
	//				float width = (cmd->ClipRect.z - cmd->ClipRect.x) / draw_data->DisplaySize.x;
	//				float height = (cmd->ClipRect.w - cmd->ClipRect.y) / draw_data->DisplaySize.y;
	//				float x = (cmd->ClipRect.x - clip_off.x) / draw_data->DisplaySize.x;
	//				float y = 1.0f - height - (cmd->ClipRect.y - clip_off.y) / draw_data->DisplaySize.y;

	//				RenderState::setScissorTest(x, y, width, height);
	//				RenderState::flushStates();

	//				auto texture = TexturePtr(static_cast<Texture*>(cmd->TextureId));
	//				imgui_material->setTexture("imgui_texture", texture);

	//				imgui_mesh->renderInstancedSurface(MeshDynamic::MODE_TRIANGLES,
	//					cmd->VtxOffset + global_vtx_offset,
	//					cmd->IdxOffset + global_idx_offset,
	//					cmd->IdxOffset + global_idx_offset + cmd->ElemCount, 1);
	//			}
	//		}
	//		global_vtx_offset += cmd_list->VtxBuffer.Size;
	//		global_idx_offset += cmd_list->IdxBuffer.Size;

	//	}
	//}
	//render_target->disable();
	//imgui_mesh->unbind();

	//RenderState::restoreState();

	//render_target->unbindColorTexture(0);
	//Render::releaseTemporaryRenderTarget(render_target);
}

static void* draw_callback_handle;

int UltralightImpl::CreateView() 
{
	///
	/// Create an HTML view, 500 by 500 pixels large.
	///
	//view = renderer->CreateView(App::getWidth(), App::getHeight(), true, nullptr);
	view = renderer->CreateView(500, 500, true, nullptr);

	if (renderer.get() == nullptr)
		return 0;

	///
	/// Load a raw string of HTML.
	///
	//view->LoadURL("file:///Test.html");
	//view->LoadHTML(Test());
	//view->LoadHTML(Sample1());
	//view->LoadHTML(Sample2());
	view->LoadHTML(Sample4());
	//view->LoadURL("file:///sample5.html");
	//view->LoadURL("file:///sample6.html");

	///
	/// Notify the View it has input focus (updates appearance).
	///
	view->Focus();

	///
	/// Get the pixel-buffer Surface for a View.
	///
	Surface* surface = view->surface();

	///
	/// Cast it to a BitmapSurface.
	///
	BitmapSurface* bitmap_surface = (BitmapSurface*)surface;

	///
	/// Get the underlying bitmap.
	///
	RefPtr<Bitmap> bitmap = bitmap_surface->bitmap();

	view->set_load_listener(new MyListener());

	return 1;
}

void UltralightImpl::UpdateLogic() {
	///
	/// Give the library a chance to handle any pending tasks and timers.
	///
	///
	renderer->Update();
}

void CopyBitmapToTexture(RefPtr<Bitmap> bitmap) {

	bitmap->SwapRedBlueChannels();

	void* pixels = bitmap->LockPixels();

	uint32_t width = bitmap->width();
	uint32_t height = bitmap->height();
	uint32_t stride = bitmap->row_bytes();

	UltralightImpl::CreateTexture(hud, pixels, width, height, stride);

	bitmap->UnlockPixels();
	bitmap->SwapRedBlueChannels();
}

void UltralightImpl::RenderOneFrame() {
	///
	/// Render all active Views (this updates the Surface for each View).
	///
	renderer->Render();

	HandleMouse();

	///
	/// Psuedo-code to loop through all active Views.
	///
	//for (auto view : view_list) {
		///
		/// Get the Surface as a BitmapSurface (the default implementation).
		///
		BitmapSurface* surface = (BitmapSurface*)(view->surface());

		///
		/// Check if our Surface is dirty (pixels have changed).
		///
		if (!surface->dirty_bounds().IsEmpty()) {
	
			/// Psuedo-code to upload Surface's bitmap to GPU texture.
			///
			CopyBitmapToTexture(surface->bitmap());
			///
			/// Clear the dirty bounds.
			///
			surface->ClearDirtyBounds();
		}
	//}
}

void UltralightImpl::CreateTexture(Unigine::WidgetSpritePtr sprite,void* pixels, uint32_t width, uint32_t height, uint32_t stride)
{ 
	auto ConvertedPixels = static_cast<unsigned char*>(pixels);
	blob->setData(ConvertedPixels,stride);

	texture->setBlob(blob);
	blob->setData(nullptr, 0);

	sprite->setRender(texture);
}

void UltralightImpl::createHUDWidgetSprite()
{
	auto width = 500;//App::getWidth();
	auto height = 500;//App::getHeight();

	hud = WidgetSprite::create(gui);
	hud->setPosition(0, 0);
	hud->setWidth(width);
	hud->setHeight(height);
	hud->setLayerBlendFunc(0, Gui::BLEND_ONE, Gui::BLEND_ONE_MINUS_SRC_ALPHA);

	texture->create2D(width, height, Texture::FORMAT_RGBA8, Texture::DEFAULT_FLAGS);

	gui->addChild(hud, Gui::ALIGN_OVERLAP);

	Ultralight_Gui_Id = gui->getNumChildren() - 1;
}

void UltralightImpl::shutdown()
{
	Ultralight_material->deleteLater();
	Ultralight_mesh->deleteLater();
	font_texture->deleteLater();

	Render::removeCallback(Render::CALLBACK_END_SCREEN, draw_callback_handle);

	renderer = nullptr;

	App::setKeyPressFunc(nullptr);
	App::setKeyReleaseFunc(nullptr);
	App::setButtonPressFunc(nullptr);
	App::setButtonReleaseFunc(nullptr);
	App::setKeyPressUnicodeFunc(nullptr);
}