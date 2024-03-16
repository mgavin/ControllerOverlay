#include "ControllerOverlay.h"
#include <format>

BAKKESMOD_PLUGIN(ControllerOverlay, "Controller Overlay", "1.7.2.mg", 0)

/*
				https://docs.unrealengine.com/udk/Three/KeyBinds.html

				XboxTypeS_LeftX - Left Analogue Stick X Axis
				XboxTypeS_RightX - Left Analogue Stick Y Axis
				XboxTypeS_LeftY - Right Analogue Stick X Axis
				XboxTypeS_RightY - Right Analogue Stick Y Axis
				XboxTypeS_DPad_Left - DPad Left
				XboxTypeS_DPad_Right - DPad Right
				XboxTypeS_DPad_Up - DPad Up
				XboxTypeS_DPad_Down - DPad Down
				XboxTypeS_A - A Button
				XboxTypeS_B - B Button
				XboxTypeS_Y - Y Button
				XboxTypeS_X - X Button
				XboxTypeS_Start - Start Button
				XboxTypeS_Back - Back Button
				XboxTypeS_LeftShoulder - Left Shoulder
				XboxTypeS_RightShoulder - Right Shoulder
				XboxTypeS_LeftTrigger - Left Trigger
				XboxTypeS_RightTrigger - Right Trigger
				XboxTypeS_LeftThumbStick - Left Analogue Stick Button
				XboxTypeS_RightThumbStick - Right Analogue Stick Button

				Gamepad_LeftStick_Left - Left Analogue Stick Left
				Gamepad_LeftStick_Right - Left Analogue Stick Right
				Gamepad_LeftStick_Up - Left Analogue Stick Up
				Gamepad_LeftStick_Down - Left Analogue Stick Down
				Gamepad_RightStick_Left - Right Analogue Stick Left
				Gamepad_RightStick_Right - Right Analogue Stick Right
				Gamepad_RightStick_Up - Right Analogue Stick Up
				Gamepad_RightStick_Down - Right Analogue Stick Down
*/

void ControllerOverlay::onLoad() {
	// because you may make the unfortunate mistake of trying to change these
	// cvars from the console so if they exist, pretend they don't, and rewrite
	// accordingly.
	cvarManager->removeCvar("controllerTitleBar");
	cvarManager->removeCvar("controllerTransparency");
	cvarManager->removeCvar("controllerType");
	cvarManager->removeCvar("controllerSize");
	cvarManager->removeCvar("controllerLockOverlayPosition");

	gameWrapper->SetTimeout(
		[this](GameWrapper * gameWrapper) { cvarManager->executeCommand("togglemenu " + GetMenuName()); }, 1);

	gameWrapper->SetTimeout(
		[this](GameWrapper * gameWrapper) {
			// roundabout way to get the instance of the currently loaded DLL
			PluginManagerWrapper pmw		 = gameWrapper->GetPluginManager();
			auto								 plugins = pmw.GetLoadedPlugins();
			auto								 plugin	 = std::find_if(std::begin(*plugins), std::end(*plugins), [this](auto plug) {
				 if (std::strcmp(plug->_details->pluginName, "Controller Overlay") == 0)
					 return 1;
				 return 0;
			 });
			if (plugin != std::end(*plugins)) {
				pluginInstance = (*plugin)->_instance;
				// cvarManager->log("Controller Overlay InstanceID: " +
				// std::to_string((int)pluginInstance));
			} else {	// is there a way to crash gracefully out of this?
				cvarManager->log("Controller Overlay instance id not captured.");
				return;
			}

			// instantiate DirectInput device
			HRESULT hr;
			hr = DirectInput8Create(pluginInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void **)&DIinterface, NULL);
			if (hr != DI_OK) {
				cvarManager->log("DirectInput instantiation failed.");
				return;
			}
		},
		1);

	cvarManager->registerCvar("controllerTitleBar", "1").addOnValueChanged([this](std::string old, CVarWrapper now) {
		titleBar = (now.getStringValue() == "1");

		writeCfg();
	});

	cvarManager->registerCvar("controllerTransparency", "1.0")
		.addOnValueChanged([this](std::string old, CVarWrapper now) {
			transparency = now.getFloatValue();

			writeCfg();
		});

	cvarManager->registerCvar("controllerLockOverlayPosition", "false")
		.addOnValueChanged([this](std::string old, CVarWrapper now) {
			overlayPositionLocked = now.getBoolValue();
			writeCfg();
		});

	cvarManager->registerCvar("controllerType", "xbox").addOnValueChanged([this](std::string old, CVarWrapper now) {
		if (now.getStringValue() == "ps4") {
			type = 1;

			inputs["XboxTypeS_DPad_Left"]				= {0, false, WHITE, "DLEFT"};
			inputs["XboxTypeS_DPad_Right"]			= {0, false, WHITE, "DRIGHT"};
			inputs["XboxTypeS_DPad_Up"]					= {0, false, WHITE, "DUP"};
			inputs["XboxTypeS_DPad_Down"]				= {0, false, WHITE, "DDOWN"};
			inputs["XboxTypeS_A"]								= {0, false, BLUE, "-"};
			inputs["XboxTypeS_B"]								= {0, false, RED, "-"};
			inputs["XboxTypeS_X"]								= {0, false, PURPLE, "-"};
			inputs["XboxTypeS_Y"]								= {0, false, DARKGREEN, "-"};
			inputs["XboxTypeS_LeftShoulder"]		= {0, false, WHITE, "L1"};
			inputs["XboxTypeS_RightShoulder"]		= {0, false, WHITE, "R1"};
			inputs["XboxTypeS_LeftTrigger"]			= {0, false, WHITE, "L2"};
			inputs["XboxTypeS_RightTrigger"]		= {0, false, WHITE, "R2"};
			inputs["XboxTypeS_LeftThumbStick"]	= {0, false, GREY, "L3"};
			inputs["XboxTypeS_RightThumbStick"] = {0, false, GREY, "R3"};
		}

		else {
			type = 0;

			inputs["XboxTypeS_DPad_Left"]				= {0, false, WHITE, "DLEFT"};
			inputs["XboxTypeS_DPad_Right"]			= {0, false, WHITE, "DRIGHT"};
			inputs["XboxTypeS_DPad_Up"]					= {0, false, WHITE, "DUP"};
			inputs["XboxTypeS_DPad_Down"]				= {0, false, WHITE, "DDOWN"};
			inputs["XboxTypeS_A"]								= {0, false, GREEN, "A"};
			inputs["XboxTypeS_B"]								= {0, false, RED, "B"};
			inputs["XboxTypeS_X"]								= {0, false, BLUE, "X"};
			inputs["XboxTypeS_Y"]								= {0, false, YELLOW, "Y"};
			inputs["XboxTypeS_LeftShoulder"]		= {0, false, WHITE, "LB"};
			inputs["XboxTypeS_RightShoulder"]		= {0, false, WHITE, "RB"};
			inputs["XboxTypeS_LeftTrigger"]			= {0, false, WHITE, "LT"};
			inputs["XboxTypeS_RightTrigger"]		= {0, false, WHITE, "RT"};
			inputs["XboxTypeS_LeftThumbStick"]	= {0, false, GREY, "LS"};
			inputs["XboxTypeS_RightThumbStick"] = {0, false, GREY, "RS"};
		}

		for (const std::pair<const std::string, Input> & input : inputs) {
			cvarManager->registerCvar(input.first, input.first).addOnValueChanged([this](std::string old, CVarWrapper now) {
				inputs[now.getStringValue()].index = gameWrapper->GetFNameIndexByString(now.getStringValue());
			});

			cvarManager->getCvar(input.first).notify();
		}

		writeCfg();
	});

	cvarManager->registerCvar("controllerSize", "0").addOnValueChanged([this](std::string old, CVarWrapper now) {
		if (now.getIntValue() == 0 || now.getIntValue() == 1) {
			size = now.getIntValue();
		}

		else {
			size = 0;
		}

		writeCfg();
	});

	if (std::ifstream(configurationFilePath)) {
		cvarManager->loadCfg(configurationFilePath);
	}

	else {
		cvarManager->getCvar("controllerType").notify();
	}

	gameWrapper->HookEvent("Function Engine.GameViewportClient.Tick",
												 bind(&ControllerOverlay::onTick, this, std::placeholders::_1));
}

void ControllerOverlay::onUnload() {
	writeCfg();

	if (renderControllerOverlay) {
		cvarManager->executeCommand("togglemenu " + GetMenuName());
	}
}

void ControllerOverlay::writeCfg() {
	std::ofstream configurationFile;

	configurationFile.open(configurationFilePath);

	configurationFile << "controllerTitleBar \"" + std::to_string(titleBar) + "\"";
	configurationFile << "\n";
	configurationFile << "controllerTransparency \"" + std::to_string(transparency) + "\"";
	configurationFile << "\n";

	if (type == 1) {
		configurationFile << "controllerType \"ps4\"";
	}

	else {
		configurationFile << "controllerType \"xbox\"";
	}

	configurationFile << "\n";
	configurationFile << "controllerSize \"" + std::to_string(size) + "\"";
	configurationFile << "\n";
	configurationFile << "controllerLockOverlayPosition \"" + std::to_string(overlayPositionLocked) + "\"";

	configurationFile.close();
}

void ControllerOverlay::onTick(std::string eventName) {
	// if (!gameWrapper->IsInCustomTraining()) {
	// if (gameWrapper->IsInGame() || gameWrapper->IsInOnlineGame()) {
	{
		{
			for (const std::pair<const std::string, Input> & input : inputs) {
				if (input.second.index > 0) {
					inputs[input.first].pressed = gameWrapper->IsKeyPressed(input.second.index);
				}
			}

			// grab coords of right analog stick
			if (type == 0) {	// XBOX
				const int PLAYER_NUMBER = 0;
				std::memset(&xboxControllerState, 0, sizeof(XINPUT_STATE));
				DWORD result = XInputGetState(PLAYER_NUMBER, &xboxControllerState);
				if (result == ERROR_DEVICE_NOT_CONNECTED) {	 // controller isn't connected
					rstickx = rsticky = 0.0f;
					rtrigger = ltrigger = 1.0f;
				} else {
					rstickx = static_cast<float>(xboxControllerState.Gamepad.sThumbRX) / SHRT_MAX;
					rsticky = -1.0f * static_cast<float>(xboxControllerState.Gamepad.sThumbRY) / SHRT_MAX;

					rtrigger = static_cast<float>(xboxControllerState.Gamepad.bRightTrigger) / 255.0f;
					ltrigger = static_cast<float>(xboxControllerState.Gamepad.bLeftTrigger) / 255.0f;
				}
			} else if (type == 1) {	 // ps4
			}

			CarWrapper car = gameWrapper->GetLocalCar();

			if (!car.IsNull()) {
				controllerInput = car.GetInput();
				lstickx					= controllerInput.Steer;
				lsticky					= controllerInput.Pitch;
			} else {
				lstickx = static_cast<float>(xboxControllerState.Gamepad.sThumbLX) / SHRT_MAX;
				lsticky = -1.0f * static_cast<float>(xboxControllerState.Gamepad.sThumbLY) / SHRT_MAX;
			}
		}
	}
}

void ControllerOverlay::Render() {
	if (!renderControllerOverlay) {
		cvarManager->executeCommand("togglemenu " + GetMenuName());

		return;
	}
	ControllerOverlay::RenderImGui();
	/*if (!gameWrapper->IsInCustomTraining()) {
		if (gameWrapper->IsInOnlineGame()) {
			ServerWrapper server = gameWrapper->GetOnlineGame();

			if (!server.IsNull()) {
				if (!server.GetbMatchEnded()) {
					ControllerOverlay::RenderImGui();
				}
			}
		}

		else if (gameWrapper->IsInGame()) {
			ServerWrapper server = gameWrapper->GetGameEventAsServer();

			if (!server.IsNull()) {
				if (!server.GetbMatchEnded()) {
					ControllerOverlay::RenderImGui();
				}
			}
		}
	}*/
}

void ControllerOverlay::RenderImGui() {
	if (renderSettings) {
		ImGui::SetNextWindowPos(ImVec2(128, 256), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(0, 0));

		ImGui::Begin((GetMenuTitle() + " - Settings").c_str(),
								 &renderSettings,
								 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

		ImGui::Checkbox("Titlebar", &titleBar);
		ImGui::Checkbox("Show Right Stick", &showRightStick);
		ImGui::Checkbox("Show DPad", &showDPad);
		ImGui::Checkbox("Lock Overlay Position", &overlayPositionLocked);
		ImGui::SliderFloat("Transparency", &transparency, 0.f, 1.f, "%.2f");
		const char * types[] = {"Xbox", "PS4"};
		ImGui::Combo("Type", &type, types, IM_ARRAYSIZE(types));
		const char * sizes[] = {"Small @1x", "Large @2x"};
		ImGui::Combo("Size", &size, sizes, IM_ARRAYSIZE(sizes));
		ImGui::End();
	}

	float scale = (size == 1 ? 2.0f : 1.0f);

	ImGui::SetNextWindowBgAlpha(transparency);

	ImGui::SetNextWindowPos(ImVec2(128, 128), ImGuiCond_FirstUseEver);

	float windowHeight = 156 * scale;

	if (!titleBar) {
		windowHeight -= ImGui::GetFrameHeight();
	}

	ImVec2 windowSize = ImVec2(216 * scale, windowHeight);

	if (size == 1) {
		windowSize.x -= 16;
		windowSize.y -= 32;
	}

	ImGui::SetNextWindowSize(windowSize);
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
	if (overlayPositionLocked) {
		windowFlags |= ImGuiWindowFlags_NoMove;
	}

	if (!titleBar) {
		windowFlags = windowFlags | ImGuiWindowFlags_NoTitleBar;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);

	ImGui::Begin(GetMenuTitle().c_str(), &renderControllerOverlay, windowFlags);

	ImVec2 cursorPosition = ImGui::GetCursorPos();

	if (size == 1) {
		ImGuiIO io = ImGui::GetIO();

		if (io.Fonts->Fonts.size() >= 1) {
			ImFont * font = io.Fonts->Fonts[1];

			ImGui::PushFont(font);
		}
	}

	ImDrawList * drawList = ImGui::GetWindowDrawList();

	ImVec2 p = ImGui::GetCursorScreenPos();

	p.x += 12 * scale;

	float buttonWidth = 48 * scale, buttonHeight = 16 * scale;

	/* DRAW TRIGGERS */
	ImVec2 buttonLTPosition = ImVec2(p.x, p.y);								 // Left Trigger / L2
	ImVec2 buttonRTPosition = ImVec2(p.x + 128 * scale, p.y);	 // Right Trigger / R2

	if (inputs["XboxTypeS_LeftTrigger"].pressed) {
		drawList->AddRect(buttonLTPosition,
											ImVec2(buttonLTPosition.x + buttonWidth, buttonLTPosition.y + buttonHeight),
											WHITE,
											8,
											ImDrawCornerFlags_TopLeft,
											2 * scale);
		int rgb = static_cast<int>(255.0f * ltrigger);
		drawList->AddRectFilled(buttonLTPosition,
														ImVec2(buttonLTPosition.x + buttonWidth, buttonLTPosition.y + buttonHeight),
														ImColor(rgb, rgb, rgb, 255),
														8,
														ImDrawCornerFlags_TopLeft);
		int rgb_txt = 255 - rgb;
		drawList->AddText(ImVec2(buttonLTPosition.x + 18 * scale, buttonLTPosition.y + 1 * scale),
											ImColor(rgb_txt, rgb_txt, rgb_txt, 255),
											inputs["XboxTypeS_LeftTrigger"].name.c_str());
	}

	else {
		drawList->AddRect(buttonLTPosition,
											ImVec2(buttonLTPosition.x + buttonWidth, buttonLTPosition.y + buttonHeight),
											WHITE,
											8,
											ImDrawCornerFlags_TopLeft,
											2 * scale);
		drawList->AddText(ImVec2(buttonLTPosition.x + 18 * scale, buttonLTPosition.y + 1 * scale),
											WHITE,
											inputs["XboxTypeS_LeftTrigger"].name.c_str());
	}

	if (inputs["XboxTypeS_RightTrigger"].pressed) {
		drawList->AddRect(buttonRTPosition,
											ImVec2(buttonRTPosition.x + buttonWidth, buttonRTPosition.y + buttonHeight),
											WHITE,
											8,
											ImDrawCornerFlags_TopRight,
											2 * scale);
		int rgb = static_cast<int>(255.0f * rtrigger);
		drawList->AddRectFilled(buttonRTPosition,
														ImVec2(buttonRTPosition.x + buttonWidth, buttonRTPosition.y + buttonHeight),
														ImColor(rgb, rgb, rgb, 255),
														8,
														ImDrawCornerFlags_TopRight);
		int rgb_txt = 255 - rgb;
		drawList->AddText(ImVec2(buttonRTPosition.x + 18 * scale, buttonRTPosition.y + 1 * scale),
											ImColor(rgb_txt, rgb_txt, rgb_txt, 255),
											inputs["XboxTypeS_RightTrigger"].name.c_str());
	}

	else {
		drawList->AddRect(buttonRTPosition,
											ImVec2(buttonRTPosition.x + buttonWidth, buttonRTPosition.y + buttonHeight),
											WHITE,
											8,
											ImDrawCornerFlags_TopRight,
											2 * scale);
		drawList->AddText(ImVec2(buttonRTPosition.x + 18 * scale, buttonRTPosition.y + 1 * scale),
											WHITE,
											inputs["XboxTypeS_RightTrigger"].name.c_str());
	}
	/* END DRAW TRIGGERS */

	p.y += buttonHeight + 4 * scale;

	/* DRAW BUMPERS */
	ImVec2 buttonLBPosition = ImVec2(p.x, p.y);								 // Left Bumper / L1
	ImVec2 buttonRBPosition = ImVec2(p.x + 128 * scale, p.y);	 // Right Bumper / R1

	if (inputs["XboxTypeS_LeftShoulder"].pressed) {
		drawList->AddRectFilled(
			buttonLBPosition, ImVec2(buttonLBPosition.x + buttonWidth, buttonLBPosition.y + buttonHeight), WHITE);
		drawList->AddText(ImVec2(buttonLBPosition.x + 18 * scale, buttonLBPosition.y + 1 * scale),
											BLACK,
											inputs["XboxTypeS_LeftShoulder"].name.c_str());
	}

	else {
		drawList->AddRect(buttonLBPosition,
											ImVec2(buttonLBPosition.x + buttonWidth, buttonLBPosition.y + buttonHeight),
											WHITE,
											0,
											0,
											2 * scale);
		drawList->AddText(ImVec2(buttonLBPosition.x + 18 * scale, buttonLBPosition.y + 1 * scale),
											WHITE,
											inputs["XboxTypeS_LeftShoulder"].name.c_str());
	}

	if (inputs["XboxTypeS_RightShoulder"].pressed) {
		drawList->AddRectFilled(
			buttonRBPosition, ImVec2(buttonRBPosition.x + buttonWidth, buttonRBPosition.y + buttonHeight), WHITE);
		drawList->AddText(ImVec2(buttonRBPosition.x + 18 * scale, buttonRBPosition.y + 1 * scale),
											BLACK,
											inputs["XboxTypeS_RightShoulder"].name.c_str());
	}

	else {
		drawList->AddRect(buttonRBPosition,
											ImVec2(buttonRBPosition.x + buttonWidth, buttonRBPosition.y + buttonHeight),
											WHITE,
											0,
											0,
											2 * scale);
		drawList->AddText(ImVec2(buttonRBPosition.x + 18 * scale, buttonRBPosition.y + 1 * scale),
											WHITE,
											inputs["XboxTypeS_RightShoulder"].name.c_str());
	}
	/* END DRAW BUMPERS */
	/* DRAW DPAD */
	if (showDPad) {
		ImVec2 dpadCenter = ImVec2(p.x + 88 * scale, p.y - 4 * scale);
		drawList->AddCircle(dpadCenter, 3.0, GREY, 32, 2 * scale);

		// up
		const ImVec2 up[] = {
			ImVec2(dpadCenter.x - 5 * scale, dpadCenter.y - 10 * scale),
			ImVec2(dpadCenter.x - 5 * scale, dpadCenter.y - 20 * scale),
			ImVec2(dpadCenter.x + 5 * scale, dpadCenter.y - 20 * scale),
			ImVec2(dpadCenter.x + 5 * scale, dpadCenter.y - 10 * scale),
			ImVec2(dpadCenter.x, dpadCenter.y - 4 * scale),
		};

		if (inputs["XboxTypeS_DPad_Up"].pressed) {
			drawList->AddConvexPolyFilled(up, 5, WHITE);
		} else {
			drawList->AddPolyline(up, 5, WHITE, true, 3.0f);
		}

		// right
		const ImVec2 right[] = {
			ImVec2(dpadCenter.x + 10 * scale, dpadCenter.y - 5 * scale),
			ImVec2(dpadCenter.x + 20 * scale, dpadCenter.y - 5 * scale),
			ImVec2(dpadCenter.x + 20 * scale, dpadCenter.y + 5 * scale),
			ImVec2(dpadCenter.x + 10 * scale, dpadCenter.y + 5 * scale),
			ImVec2(dpadCenter.x + 4 * scale, dpadCenter.y),
		};

		if (inputs["XboxTypeS_DPad_Right"].pressed) {
			drawList->AddConvexPolyFilled(right, 5, WHITE);
		} else {
			drawList->AddPolyline(right, 5, WHITE, true, 3.0f);
		}

		// down
		const ImVec2 down[] = {
			ImVec2(dpadCenter.x - 5 * scale, dpadCenter.y + 10 * scale),
			ImVec2(dpadCenter.x - 5 * scale, dpadCenter.y + 20 * scale),
			ImVec2(dpadCenter.x + 5 * scale, dpadCenter.y + 20 * scale),
			ImVec2(dpadCenter.x + 5 * scale, dpadCenter.y + 10 * scale),
			ImVec2(dpadCenter.x, dpadCenter.y + 4 * scale),
		};

		if (inputs["XboxTypeS_DPad_Down"].pressed) {
			drawList->AddConvexPolyFilled(down, 5, WHITE);
		} else {
			drawList->AddPolyline(down, 5, WHITE, true, 3.0f);
		}

		// left
		const ImVec2 left[] = {
			ImVec2(dpadCenter.x - 10 * scale, dpadCenter.y - 5 * scale),
			ImVec2(dpadCenter.x - 20 * scale, dpadCenter.y - 5 * scale),
			ImVec2(dpadCenter.x - 20 * scale, dpadCenter.y + 5 * scale),
			ImVec2(dpadCenter.x - 10 * scale, dpadCenter.y + 5 * scale),
			ImVec2(dpadCenter.x - 4 * scale, dpadCenter.y),
		};

		if (inputs["XboxTypeS_DPad_Left"].pressed) {
			drawList->AddConvexPolyFilled(left, 5, WHITE);
		} else {
			drawList->AddPolyline(left, 5, WHITE, true, 3.0f);
		}
	}
	/* END DRAW DPAD */

	p.y += buttonHeight + 16 * scale;

	p.x -= 8 * scale;

	float	 leftStickRadius = 32 * scale;
	ImVec2 leftStickCenter = ImVec2(p.x + leftStickRadius, p.y + leftStickRadius);

	drawList->AddCircle(leftStickCenter, 24 * scale, WHITE, 32, 2 * scale);

	drawList->AddCircleFilled(
		ImVec2(leftStickCenter.x + (lstickx * 8 * scale), leftStickCenter.y + (lsticky * 8 * scale)),
		20 * scale,
		(inputs["XboxTypeS_LeftThumbStick"].pressed ? GREY : WHITE),
		32);
	drawList->AddCircleFilled(
		ImVec2(leftStickCenter.x + (lstickx * 8 * scale), leftStickCenter.y + (lsticky * 8 * scale)),
		16 * scale,
		(inputs["XboxTypeS_LeftThumbStick"].pressed ? DARKGREY : GREY),
		32);

	// set up position of right analog stick
	float	 rightStickRadius = 32 * scale;
	ImVec2 rightStickCenter = ImVec2(leftStickCenter.x + 56 * scale, leftStickCenter.y);

	// set up position of buttons
	float	 buttonRadius	 = 12 * scale;
	ImVec2 buttonsCenter = ImVec2(leftStickCenter.x + 128 * scale, leftStickCenter.y);

	std::map<std::string, ImVec2> buttonPositions;
	std::map<std::string, ImVec2> buttonTextPositions;

	if (type == 0) {	// xbox
		// draw right analog stick
		drawList->AddCircle(rightStickCenter, 24 * scale, WHITE, 32, 2 * scale);

		drawList->AddCircleFilled(
			ImVec2(rightStickCenter.x + (rstickx * 8 * scale), rightStickCenter.y + (rsticky * 8 * scale)),
			20 * scale,
			(inputs["XboxTypeS_RightThumbStick"].pressed ? GREY : WHITE),
			32);
		drawList->AddCircleFilled(
			ImVec2(rightStickCenter.x + (rstickx * 8 * scale), rightStickCenter.y + (rsticky * 8 * scale)),
			16 * scale,
			(inputs["XboxTypeS_RightThumbStick"].pressed ? DARKGREY : GREY),
			32);

		// draw buttons
		buttonPositions["XboxTypeS_A"] = ImVec2(buttonsCenter.x, buttonsCenter.y + buttonRadius * 2);
		buttonPositions["XboxTypeS_B"] = ImVec2(buttonsCenter.x + buttonRadius * 2, buttonsCenter.y);
		buttonPositions["XboxTypeS_X"] = ImVec2(buttonsCenter.x - buttonRadius * 2, buttonsCenter.y);
		buttonPositions["XboxTypeS_Y"] = ImVec2(buttonsCenter.x, buttonsCenter.y - buttonRadius * 2);

		buttonTextPositions["XboxTypeS_A"] = ImVec2(3 * scale + buttonPositions["XboxTypeS_A"].x - buttonRadius * 0.5f,
																								buttonPositions["XboxTypeS_A"].y - buttonRadius * 0.5f - 1);
		buttonTextPositions["XboxTypeS_B"] = ImVec2(3 * scale + buttonPositions["XboxTypeS_B"].x - buttonRadius * 0.5f,
																								buttonPositions["XboxTypeS_B"].y - buttonRadius * 0.5f - 1);
		buttonTextPositions["XboxTypeS_X"] = ImVec2(3 * scale + buttonPositions["XboxTypeS_X"].x - buttonRadius * 0.5f,
																								buttonPositions["XboxTypeS_X"].y - buttonRadius * 0.5f - 1);
		buttonTextPositions["XboxTypeS_Y"] = ImVec2(3 * scale + buttonPositions["XboxTypeS_Y"].x - buttonRadius * 0.5f,
																								buttonPositions["XboxTypeS_Y"].y - buttonRadius * 0.5f - 1);

		for (std::pair<std::string, ImVec2> buttonPosition : buttonPositions) {
			if (inputs[buttonPosition.first].pressed) {
				drawList->AddCircleFilled(buttonPosition.second, buttonRadius, inputs[buttonPosition.first].color, 32);
				drawList->AddText(buttonTextPositions[buttonPosition.first], BLACK, inputs[buttonPosition.first].name.c_str());
			}

			else {
				drawList->AddCircle(buttonPosition.second, buttonRadius, inputs[buttonPosition.first].color, 32, 2 * scale);
				drawList->AddText(buttonTextPositions[buttonPosition.first],
													inputs[buttonPosition.first].color,
													inputs[buttonPosition.first].name.c_str());
			}
		}
	} else if (type == 1) {	 // ps4

		// draw buttons
		buttonPositions["XboxTypeS_A"] = ImVec2(buttonsCenter.x, buttonsCenter.y + buttonRadius * 2);
		buttonPositions["XboxTypeS_B"] = ImVec2(buttonsCenter.x + buttonRadius * 2, buttonsCenter.y);
		buttonPositions["XboxTypeS_X"] = ImVec2(buttonsCenter.x - buttonRadius * 2, buttonsCenter.y);
		buttonPositions["XboxTypeS_Y"] = ImVec2(buttonsCenter.x, buttonsCenter.y - buttonRadius * 2);

		for (std::pair<std::string, ImVec2> buttonPosition : buttonPositions) {
			if (inputs[buttonPosition.first].pressed) {
				drawList->AddCircleFilled(buttonPosition.second, buttonRadius, WHITE, 32);
			}

			else {
				drawList->AddCircle(buttonPosition.second, buttonRadius, WHITE, 32, 2 * scale);
			}

			if (buttonPosition.first == "XboxTypeS_A") {
				drawList->AddLine(ImVec2(buttonPosition.second.x - 5 * scale, buttonPosition.second.y - 5 * scale),
													ImVec2(buttonPosition.second.x + 5 * scale, buttonPosition.second.y + 5 * scale),
													inputs[buttonPosition.first].color,
													2 * scale);
				drawList->AddLine(ImVec2(buttonPosition.second.x - 5 * scale, buttonPosition.second.y + 5 * scale),
													ImVec2(buttonPosition.second.x + 5 * scale, buttonPosition.second.y - 5 * scale),
													inputs[buttonPosition.first].color,
													2 * scale);
			}

			else if (buttonPosition.first == "XboxTypeS_B") {
				drawList->AddCircle(
					buttonPosition.second, buttonRadius - 6 * scale, inputs[buttonPosition.first].color, 16, 2 * scale);
			}

			else if (buttonPosition.first == "XboxTypeS_X") {
				drawList->AddQuad(ImVec2(buttonPosition.second.x - 5 * scale, buttonPosition.second.y - 5 * scale),
													ImVec2(buttonPosition.second.x + 5 * scale, buttonPosition.second.y - 5 * scale),
													ImVec2(buttonPosition.second.x + 5 * scale, buttonPosition.second.y + 5 * scale),
													ImVec2(buttonPosition.second.x - 5 * scale, buttonPosition.second.y + 5 * scale),
													inputs[buttonPosition.first].color,
													2 * scale);
			}

			else if (buttonPosition.first == "XboxTypeS_Y") {
				drawList->AddTriangle(ImVec2(buttonPosition.second.x, buttonPosition.second.y - 5 * scale),
															ImVec2(buttonPosition.second.x + 5 * scale, buttonPosition.second.y + 4 * scale),
															ImVec2(buttonPosition.second.x - 5 * scale, buttonPosition.second.y + 4 * scale),
															inputs[buttonPosition.first].color,
															2 * scale);
			}
		}
	}

	ImGui::SetCursorPos(
		ImVec2((size == 0 ? cursorPosition.x + 73 : cursorPosition.x + 156), cursorPosition.y + 8 * scale));

	if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
		if (ImGui::Button("Settings")) {
			renderSettings = !renderSettings;
		}
	}

	/* ADD TEXT FOR COORDS OF LEFT AND RIGHT STICK */
	drawList->AddText(ImGui::GetFont(),
										9 * scale,
										ImVec2{leftStickCenter.x - leftStickRadius + (6 * scale),
													 leftStickCenter.y + leftStickRadius - (scale == 2 ? 10 : 2)},
										WHITE,
										std::format("{: 0.3f} {: 0.3f}", lstickx, lsticky).c_str());
	drawList->AddText(ImGui::GetFont(),
										9 * scale,
										ImVec2{rightStickCenter.x - rightStickRadius + (6 * scale),
													 rightStickCenter.y + rightStickRadius - (scale == 2 ? 10 : 2)},
										WHITE,
										std::format("{: 0.3f} {: 0.3f}", rstickx, rsticky).c_str());
	/* FINISH ADDING TEXT FOR COORDS OF LEFT AND RIGHT STICK */

	if (size == 1) {
		ImGui::PopFont();
	}
	ImGui::PopStyleVar();

	ImGui::End();
}

std::string ControllerOverlay::GetMenuName() {
	return "controlleroverlay";
}

std::string ControllerOverlay::GetMenuTitle() {
	return "Controller Overlay";
}

void ControllerOverlay::SetImGuiContext(uintptr_t ctx) {
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext *>(ctx));
}

bool ControllerOverlay::ShouldBlockInput() {
	return false;
}

bool ControllerOverlay::IsActiveOverlay() {
	return false;
}

void ControllerOverlay::OnOpen() {
	renderControllerOverlay = true;
}

void ControllerOverlay::OnClose() {
	renderControllerOverlay = false;
}
