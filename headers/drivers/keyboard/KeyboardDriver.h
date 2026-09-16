/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _KEYBOARD_DRIVER_H_
#define _KEYBOARD_DRIVER_H_

#include "gpdriver.h"
#include "drivers/keyboard/KeyboardDescriptors.h"
#include "eventmanager.h"

class KeyboardDriver : public GPDriver {
public:
    virtual void initialize();
    virtual bool process(Gamepad * gamepad);
    virtual void initializeAux() {}
    virtual void processAux() {}
    virtual uint16_t get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen);
    virtual void set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize);
    virtual bool vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request);
    virtual const uint16_t * get_descriptor_string_cb(uint8_t index, uint16_t langid);
    virtual const uint8_t * get_descriptor_device_cb();
    virtual const uint8_t * get_hid_descriptor_report_cb(uint8_t itf) ;
    virtual const uint8_t * get_descriptor_configuration_cb(uint8_t index);
    virtual const uint8_t * get_descriptor_device_qualifier_cb();
    virtual uint16_t GetJoystickMidValue();
    virtual USBListener * get_usb_auth_listener() { return nullptr; }
    void handleEncoder(GPEvent* e); // for Volume - rotary encoder
private:
    void releaseAllKeys(void);
	void pressKey(uint8_t code);
    uint8_t getModifier(uint8_t code);
    uint8_t getMultimedia(uint8_t code);
    void pressMisterMenuShortAction();
    void pressMisterMenuLongAction();
    uint8_t last_report[CFG_TUD_ENDPOINT0_SIZE] = { };
    uint16_t last_report_size;
    KeyboardReport keyboardReport;
    int8_t volumeChange;

    // State for the custom MiSTer short/long-press menu button.
    bool misterMenuWasPressed = false;
    bool misterMenuLongTriggered = false;
    uint8_t misterMenuPulseType = 0; // 0 = none, 1 = short, 2 = long
    uint32_t misterMenuPressStartMs = 0;
    uint32_t misterMenuPulseStartMs = 0;
};

#endif // _KEYBOARD_DRIVER_H_
