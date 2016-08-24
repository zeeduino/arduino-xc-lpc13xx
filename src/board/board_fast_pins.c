
#include <inttypes.h>
#include "chip.h"

#include "pins_arduino.h"

// this goes to HAL since internal pin structures are only accessible there
void Board_Fast_Pins_Handle_Init(
		volatile uint32_t **pinSetHandle,
		volatile uint32_t **pinClrHandle,
		volatile uint32_t **pinReadHandle,
		volatile uint32_t **pinToggleHandle,
		volatile uint32_t *pinPositionHandle,
		int arduinoPin)
{
	*pinSetHandle = &((DIG_MAP_PORT(arduinoPin))->SET[APIN_PORT(arduinoPin)]);
	*pinClrHandle = &((DIG_MAP_PORT(arduinoPin))->CLR[APIN_PORT(arduinoPin)]);
	*pinReadHandle = &((DIG_MAP_PORT(arduinoPin))->PIN[APIN_PORT(arduinoPin)]);
	*pinToggleHandle = &((DIG_MAP_PORT(arduinoPin))->NOT[APIN_PORT(arduinoPin)]);

	*pinPositionHandle = (1UL << APIN_PIN(arduinoPin));
}
