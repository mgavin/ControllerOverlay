#pragma comment(lib, "pluginsdk.lib")

#include <format>
#include <fstream>

#include "bakkesmod/plugin/PluginSettingsWindow.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/wrappers/GameWrapper.h"

#include "imgui/imgui.h"

// getting the right analog stick of an xbox controller on windows
#include <Windows.h>
#include <Xinput.h>
#include <limits>

// getting the right analog stick of a ps4(5?) controller on windows
#include <dinput.h>

// TODO: ADD DIFFERENTIATION BETWEEN DINPUT AND XINPUT
// MAYBE COMPLETELY MOVE OVER TO DINPUT? NEED DATA FORMATS :rolling_eyes:

#define RED ImColor(255, 0, 0, 255)
#define BLUE ImColor(0, 0, 255, 255)
#define GREEN ImColor(0, 255, 0, 255)
#define DARKGREEN ImColor(0, 128, 0, 255)
#define BLACK ImColor(0, 0, 0, 255)
#define WHITE ImColor(255, 255, 255, 255)
#define GREY ImColor(170, 170, 170, 255)
#define DARKGREY ImColor(85, 85, 85, 255)
#define YELLOW ImColor(255, 255, 0, 255)
#define PURPLE ImColor(128, 0, 128, 255)

struct Input {
	int					index;
	bool				pressed;
	ImColor			color;
	std::string name;
};

class ControllerOverlay :
		public BakkesMod::Plugin::BakkesModPlugin,
		public BakkesMod::Plugin::PluginWindow,
		public BakkesMod::Plugin::PluginSettingsWindow {
public:
	void onLoad();
	void onUnload();

	void writeCfg();

	void onTick(std::string eventName);

	void				Render();
	void				RenderImGui();
	void				RenderSettings();
	std::string GetMenuName();
	std::string GetMenuTitle();
	std::string GetPluginName();
	void				SetImGuiContext(uintptr_t ctx);
	bool				ShouldBlockInput();
	bool				IsActiveOverlay();
	void				OnOpen();
	void				OnClose();

	bool renderControllerOverlay	= false;
	bool renderSettings						= false;
	bool overlayPositionLocked		= false;
	bool overlayShouldBlockInputs = false;

	// 	std::string configurationFilePath = std::string(BAKKESMOD_DIR) + "/cfg/controlleroverlay.cfg";
	std::string configurationFilePath = gameWrapper->GetBakkesModPath().string() + "/cfg/controlleroverlay.cfg";

	bool	titleBar			 = true;
	bool	showRightStick = true;
	bool	showDPad			 = true;
	float transparency	 = 1.0f;
	int		type					 = 0;
	int		size					 = 0;

	std::map<std::string, Input> inputs;
	ControllerInput							 controllerInput;
	float												 rstickx, rsticky;
	float												 rtrigger, ltrigger;
	float												 lstickx, lsticky;

	// WORKS FOR XBOX CONTROLLER
	XINPUT_STATE xboxControllerState;

	// NEED TO DO FOR PS4 CONTROLLER / DirectInput (instead of XInput)
	HINSTANCE											 pluginInstance;
	std::shared_ptr<IDirectInput8> DIinterface;
};
