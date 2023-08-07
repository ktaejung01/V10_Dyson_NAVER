/*
 * bms.h
 *
 * Created: 16/07/2023 20:00:06
 *  Author: David Pye
 */ 


#ifndef BMS_H_
#define BMS_H_

#include "asf.h"

#include "board.h"
#include "bq7693.h"
#include "serial.h"
#include "leds.h"

void pins_init(void);

void bms_init(void);
void bms_mainloop(void);

void bms_handle_idle(void);
void bms_handle_sleep(void);

bool bms_is_pack_full(void);
bool bms_is_safe_to_discharge(void);
bool bms_is_safe_to_charge(void);

enum BMS_STATE {
	BMS_IDLE,
	BMS_CHARGER_CONNECTED,
	BMS_CHARGING,
	BMS_CHARGING_FAULT,
	BMS_CHARGER_CONNECTED_NOT_CHARGING,
	BMS_CHARGER_UNPLUGGED,
	BMS_TRIGGER_PULLED,
	BMS_DISCHARGING,
	BMS_DISCHARGE_FAULT,
	BMS_SLEEP
};

enum BMS_ERROR_CODE {
	BMS_ERR_NONE,			//All good!
	BMS_ERR_PACK_OVERTEMP,	//Pack thermistor reading exceeded MAX_PACK_TEMPERATURE - default 60'C
	BMS_ERR_SHORTCIRCUIT,	//BMS detected short circuit
	BMS_ERR_OVERCURRENT,	//BMS detected overcurrent fault
	BMS_ERR_UNDERVOLTAGE,	//BMS detected undervoltage state
	BMS_ERR_OVERVOLTAGE,	//BMS detected overvoltage state
	BMS_ERR_I2C_FAIL,		//Unable to talk to the BQ7693 IC
};

#endif /* BMS_H_ */