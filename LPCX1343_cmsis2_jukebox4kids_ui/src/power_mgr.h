#ifndef __POWER_MGR_H
#define __POWER_MGR_H

#define POWER_MGR_PORT PORT1
#define POWER_MGR_PIN_PLAYER 8
#define POWER_MGR_PIN_AMP 9


void power_mgr_init();
void power_mgr_process(uint32_t msticks);
void power_mgr_set_player(uint8_t enabled);
void power_mgr_set_amp(uint8_t enabled);
void power_mgr_shutdown_player(int8_t seconds);
void power_mgr_shutdown_amp(int8_t seconds);
uint8_t power_mgr_is_shutting_down();
uint8_t power_mgr_get_player_status();
uint8_t power_mgr_get_amp_status();
int8_t power_mgr_get_remaining_player_seconds();
int8_t power_mgr_get_remaining_amp_seconds();

#endif /* end __POWER_MGR_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
