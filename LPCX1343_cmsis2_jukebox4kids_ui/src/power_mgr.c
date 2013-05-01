#include "LPC13xx.h"
#include "gpio.h"
#include "power_mgr.h"
#include "logger.h"
#include "math_utils.h"

uint32_t power_mgr_msticks = 0;
uint32_t power_mgr_last_msticks = 0;
int8_t power_mgr_remaining_player_seconds = -1;
int8_t power_mgr_remaining_amp_seconds = -1;


void power_mgr_init() {
   GPIOSetDir( POWER_MGR_PORT, POWER_MGR_PIN_PLAYER, 1);
   GPIOSetDir( POWER_MGR_PORT, POWER_MGR_PIN_AMP, 1);

   // set default value (0 means active due relay module)
   GPIOSetValue( POWER_MGR_PORT, POWER_MGR_PIN_PLAYER, 0);
   GPIOSetValue( POWER_MGR_PORT, POWER_MGR_PIN_AMP, 0);
}

void power_mgr_process(uint32_t msticks) {
	power_mgr_msticks = msticks;
	if (power_mgr_remaining_player_seconds > -1 || power_mgr_remaining_amp_seconds > -1) {
		// time interval is 1 second
		if (math_calc_diff(power_mgr_msticks, power_mgr_last_msticks) > 100) {
			power_mgr_last_msticks = power_mgr_msticks;
			if (power_mgr_remaining_player_seconds > -1) {
				power_mgr_remaining_player_seconds--;
			}
			if (power_mgr_remaining_amp_seconds > -1) {
				power_mgr_remaining_amp_seconds--;
			}
			if (power_mgr_remaining_player_seconds == 0) {
				power_mgr_set_player(0);
				// with no player there is no need for an amp
				power_mgr_set_amp(0);
			}
			if (power_mgr_remaining_amp_seconds == 0) {
				power_mgr_set_amp(0);
			}
		}
	}
}

void power_mgr_set_player(uint8_t enabled) {
	// negated due relay module
	if (enabled) {
		GPIOSetValue( POWER_MGR_PORT, POWER_MGR_PIN_PLAYER, 0);
	}
	else {
		GPIOSetValue( POWER_MGR_PORT, POWER_MGR_PIN_PLAYER, 1);
	}
}

void power_mgr_set_amp(uint8_t enabled) {
	// negated due relay module
	if (enabled) {
		GPIOSetValue( POWER_MGR_PORT, POWER_MGR_PIN_AMP, 0);
	}
	else {
		GPIOSetValue( POWER_MGR_PORT, POWER_MGR_PIN_AMP, 1);
	}
}

void power_mgr_shutdown_player(int8_t seconds) {
	power_mgr_remaining_player_seconds = seconds;
}

void power_mgr_shutdown_amp(int8_t seconds) {
	power_mgr_remaining_amp_seconds = seconds;
}

uint8_t power_mgr_is_shutting_down() {
	return power_mgr_remaining_player_seconds > -1;
}

uint8_t power_mgr_get_player_status() {
	return !GPIOGetValue( POWER_MGR_PORT, POWER_MGR_PIN_PLAYER );
}

uint8_t power_mgr_get_amp_status() {
	return !GPIOGetValue( POWER_MGR_PORT, POWER_MGR_PIN_AMP );
}

int8_t power_mgr_get_remaining_player_seconds() {
	return power_mgr_remaining_player_seconds;
}

int8_t power_mgr_get_remaining_amp_seconds() {
	return power_mgr_remaining_amp_seconds;
}


