/*
 * tracer.h
 *
 *  Created on: Aug 11, 2016
 *      Author: iceman
 */

#ifndef TRACER_H_
#define TRACER_H_

#include <inttypes.h>

#define EVENT_DEV_CONFIGURED 'C'
#define EVENT_DEV_RESET 'r'
#define EVENT_CDC_LINECODE 'L'
#define EVENT_CDC_CONFIGURED 'c'
#define EVENT_CUSTOM_INIT 'I'

#define EVENT_BUFFER_SIZE 64

extern uint8_t eventBuffer[EVENT_BUFFER_SIZE];
extern uint16_t eventBufferIndex;

#define ADD_EVENT(x) {if(eventBufferIndex<EVENT_BUFFER_SIZE) {eventBuffer[eventBufferIndex] = x; eventBufferIndex++;}}



#endif /* TRACER_H_ */
