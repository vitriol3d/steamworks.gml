/// steam_controller.cpp
#include "steam_glue.h"

#pragma region Controller meta
steam_gml_map<ControllerHandle_t> steam_controller_controllers;
steam_gml_namedmap<ControllerActionSetHandle_t> steam_controller_actionsets;
steam_gml_namedmap<ControllerAnalogActionHandle_t> steam_controller_analog;
steam_gml_namedmap<ControllerDigitalActionHandle_t> steam_controller_digital;
ControllerHandle_t steam_controller_handles[STEAM_CONTROLLER_MAX_COUNT];
EControllerActionOrigin steam_controller_origins[STEAM_CONTROLLER_MAX_ORIGINS];
//
void steam_controller_reset_impl() {
	steam_controller_controllers.clear();
	steam_controller_actionsets.clear();
	steam_controller_analog.clear();
	steam_controller_digital.clear();
}
dllx double steam_controller_reset() {
	steam_controller_reset_impl();
	return 1;
}
/// Should be called on game start if you use Steam Controller
dllx double steam_controller_init() {
	return SteamController() && SteamController()->Init();
}
/// Can be called prior to other controller functions for lower latency. Called by steam_gml_update.
dllx double steam_controller_update() {
	if (SteamController()) {
		SteamController()->RunFrame();
		return 1;
	} else return 0;
}
dllx double steam_controller_get_max_count_raw() {
	return STEAM_CONTROLLER_MAX_COUNT;
}
dllx double steam_controller_get_ids_raw(char* cbuf) {
	if (!SteamController()) return 0;
	SteamController()->RunFrame();
	int found = SteamController()->GetConnectedControllers(steam_controller_handles);
	if (found <= 0) return found;
	buffer buf(cbuf);
	for (int i = 0; i < found; i++) {
		buf.write<int32>(steam_controller_controllers.add(steam_controller_handles[i]));
	}
	return found;
}
ControllerHandle_t steam_controller_find(double id) {
	if (id == -3/* all */) return STEAM_CONTROLLER_HANDLE_ALL_CONTROLLERS;
	ControllerHandle_t q; return steam_controller_controllers.get(id, &q) ? q : 0;
}
#pragma endregion

#pragma region ActionSet
/// Assigns and returns ActionSet ID for given name, -1 if actionset is not found
dllx double steam_controller_get_actionset_id(char* actionSetName) {
	if (!SteamController()) return -1;
	int i; if (steam_controller_actionsets.find_name(actionSetName, &i)) return i;
	ControllerActionSetHandle_t r = SteamController()->GetActionSetHandle(actionSetName);
	if (r != 0) {
		return steam_controller_actionsets.set(actionSetName, r);
	} else return steam_controller_actionsets.set_noone(actionSetName);
}
/// Returns action set ID (generated by steam_controller_get_actionset_id), as per GetCurrentActionSet
dllx double steam_controller_get_actionset(double controller) {
	if (!SteamController()) return -1;
	ControllerHandle_t q; if (!steam_controller_controllers.get(controller, &q)) return -1;
	ControllerActionSetHandle_t r = SteamController()->GetCurrentActionSet(q);
	int i; return steam_controller_actionsets.find_value(r, &i) ? i : -1;
}
/// Changes controller action set, as per ActivateActionSet
dllx double steam_controller_set_actionset(double controller, double actionset_id) {
	if (!SteamController()) return false;
	ControllerHandle_t q; if (!steam_controller_controllers.get(controller, &q)) return false;
	ControllerActionSetHandle_t v; if (!steam_controller_actionsets.get(actionset_id, &v)) return false;
	SteamController()->ActivateActionSet(q, v);
	return true;
}
#pragma endregion

#pragma region DigitalAction
///
dllx double steam_controller_get_digital_id(char* digitalActionName) {
	if (!SteamController()) return -1;
	int i; if (steam_controller_actionsets.find_name(digitalActionName, &i)) return i;
	ControllerDigitalActionHandle_t r = SteamController()->GetDigitalActionHandle(digitalActionName);
	if (r != 0) {
		return steam_controller_digital.set(digitalActionName, r);
	} else return steam_controller_digital.set_noone(digitalActionName);
}
/// Retreives digital action state (true/false)
dllx double steam_controller_get_digital_value(double controller, double digital_id) {
	if (!SteamController()) return 0;
	ControllerHandle_t q = steam_controller_find(controller); if (q == 0) return 0;
	ControllerDigitalActionHandle_t t; if (!steam_controller_digital.get(digital_id, &t)) return 0;
	ControllerDigitalActionData_t d = SteamController()->GetDigitalActionData(q, t);
	return d.bState;
}
/// Returns whether the given digital action is currently active (true/false)
dllx double steam_controller_get_digital_status(double controller, double digital_id) {
	if (!SteamController()) return 0;
	ControllerHandle_t ctl = steam_controller_find(controller); if (ctl == 0) return 0;
	ControllerDigitalActionHandle_t act; if (!steam_controller_digital.get(digital_id, &act)) return 0;
	ControllerDigitalActionData_t d = SteamController()->GetDigitalActionData(ctl, act);
	return d.bActive;
}
dllx double steam_controller_get_digital_origins_raw(
	double controller, double actionset_id, double digital_id, char* out
) {
	if (!SteamController()) return 0;
	ControllerHandle_t ctl = steam_controller_find(controller); if (ctl == 0) return 0;
	ControllerActionSetHandle_t set;
	if (!steam_controller_actionsets.get(actionset_id, &set)) return 0;
	ControllerDigitalActionHandle_t act = 4;
	if (!steam_controller_digital.get(digital_id, &act)) return 0;
	int found = SteamController()->GetDigitalActionOrigins(ctl, set, act, steam_controller_origins);
	buffer buf(out);
	for (int i = 0; i < found; i++) {
		buf.write<int32>(steam_controller_origins[i]);
	}
	return found;
}
#pragma endregion

#pragma region Analog action
///
dllx double steam_controller_get_analog_id(char* analogActionName) {
	if (!SteamController()) return -1;
	int i; if (steam_controller_actionsets.find_name(analogActionName, &i)) return i;
	ControllerDigitalActionHandle_t r = SteamController()->GetAnalogActionHandle(analogActionName);
	if (r != 0) {
		return steam_controller_analog.set(analogActionName, r);
	} else return steam_controller_analog.set_noone(analogActionName);
}
dllx double steam_controller_get_analog_data(double controller, double analog_id, double data_id) {
	if (!SteamController()) return 0;
	ControllerHandle_t q = steam_controller_find(controller); if (q == 0) return -1;
	ControllerAnalogActionHandle_t t; if (!steam_controller_analog.get(analog_id, &t)) return -2;
	ControllerAnalogActionData_t d = SteamController()->GetAnalogActionData(q, t);
	switch ((int)data_id) {
		case 1: return (int)d.eMode;
		case 2: return d.x;
		case 3: return d.y;
		default: return d.bActive;
	}
}
dllx double steam_controller_get_analog_origins_raw(
	double controller, double actionset_id, double digital_id, char* out
) {
	if (!SteamController()) return 0;
	ControllerHandle_t ctl = steam_controller_find(controller); if (ctl == 0) return 0;
	ControllerActionSetHandle_t set;
	if (!steam_controller_actionsets.get(actionset_id, &set)) return 0;
	ControllerDigitalActionHandle_t act = 4;
	if (!steam_controller_analog.get(digital_id, &act)) return 0;
	int found = SteamController()->GetAnalogActionOrigins(ctl, set, act, steam_controller_origins);
	buffer buf(out);
	for (int i = 0; i < found; i++) {
		buf.write<int32>(steam_controller_origins[i]);
	}
	return found;
}
#pragma endregion

#pragma region Origin
dllx double steam_controller_get_max_origins_raw() {
	return STEAM_CONTROLLER_MAX_ORIGINS;
}
#pragma endregion
