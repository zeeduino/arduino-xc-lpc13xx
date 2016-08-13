/*
 * app_config.h
 *
 *  Created on: Aug 7, 2016
 *      Author: iceman
 */

#ifndef USB_DEVICE_CONFIG_H_
#define USB_DEVICE_CONFIG_H_


#define CONFIG_USB_DEVICE_STRING_MANUFACTURER       "Ravendyne Inc"
#define CONFIG_USB_DEVICE_STRING_PRODUCT            "Test USB device"
#define CONFIG_USB_DEVICE_VENDORID                  (0x1FC9)

#define CONFIG_USB_DEVICE_CDC
//#define CONFIG_USB_DEVICE_MSC
#define CONFIG_USB_DEVICE_CUSTOM_CLASS

//#define CONFIG_USB_DEVICE_HID_KEYBOARD
//#define CONFIG_USB_DEVICE_HID_MOUSE
//#define CONFIG_USB_DEVICE_HID_GENERIC
//#define CONFIG_USB_DEVICE_HID_GENERIC_REPORT_SIZE (64)


      #if (defined(CONFIG_USB_DEVICE_CDC)       || defined(CONFIG_USB_DEVICE_HID_KEYBOARD) || \
           defined(CONFIG_USB_DEVICE_HID_MOUSE) || defined(CONFIG_USB_DEVICE_HID_GENERIC)  || \
           defined(CONFIG_USB_DEVICE_MSC)       || defined(CONFIG_USB_DEVICE_CUSTOM_CLASS))
        #define CONFIG_USB_DEVICE
        #if defined(CONFIG_USB_DEVICE_HID_KEYBOARD) || defined(CONFIG_USB_DEVICE_HID_MOUSE) || defined(CONFIG_USB_DEVICE_HID_GENERIC)
          #define CONFIG_USB_DEVICE_HID
          #if defined(CONFIG_USB_DEVICE_HID_GENERIC) && (CONFIG_USB_DEVICE_HID_GENERIC_REPORT_SIZE > 64)
            #error "CONFIG_USB_DEVICE_HID_GENERIC_REPORT_SIZE exceeds the maximum value of 64 bytes (based on USB specs 2.0 for 'Full Speed Interrupt Endpoint Size')"
          #endif
        #endif
      #endif



#endif /* USB_DEVICE_CONFIG_H_ */
