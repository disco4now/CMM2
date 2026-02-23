/*
 * keyboard.h
 *
 *  Created on: 2 Jul 2018
 *      Author: Peter
 */

#ifndef KEYBOARD_H_
#define KEYBOARD_H_

typedef enum {
   HID_DEMO_IDLE = 0,
   HID_DEMO_WAIT,
   HID_DEMO_START,
   HID_DEMO_MOUSE,
   HID_DEMO_KEYBOARD,
   HID_DEMO_REENUMERATE,
 }HID_Demo_State;

 typedef enum {
   HID_MOUSE_IDLE = 0,
   HID_MOUSE_WAIT,
   HID_MOUSE_START,
 }HID_mouse_State;

 typedef enum {
   HID_KEYBOARD_IDLE = 0,
   HID_KEYBOARD_WAIT,
   HID_KEYBOARD_START,
 }HID_keyboard_State;

 typedef struct _DemoStateMachine {
   __IO HID_Demo_State     state;
   __IO HID_mouse_State    mouse_state;
   __IO HID_keyboard_State keyboard_state;
   __IO uint8_t            select;
   __IO uint8_t            lock;
 }HID_DEMO_StateMachine;


// void Error_Handler(void);
 /* Private variables --------------------------------------------------------- */
extern HID_DEMO_StateMachine hid_demo;
extern uint8_t *DEMO_KEYBOARD_menu[];
extern uint8_t prev_select;
extern uint32_t hid_demo_ready;
extern ApplicationTypeDef Appli_state ;
extern USBH_HandleTypeDef hUsbHostFS;
extern void HID_SelectItem(uint8_t ** menu, uint8_t item);
extern void HID_KeyboardMenuProcess(void);
extern void USR_KEYBRD_ProcessData(uint8_t data);
extern void HID_MenuInit(void);
extern void HID_MenuProcess(void);
extern const int UKkeyValue[];
extern const int USkeyValue[];
extern const int *keylayout;
extern uint8_t APP_MapKeyToUsage(HID_KEYBD_Info_TypeDef *info, int keyno);
extern HID_KEYBD_Info_TypeDef last_k_info;
extern HID_KEYBD_Info_TypeDef *k_pinfo;
extern int KeyDown[7];
extern void CheckKeyboard(void);
extern void clearrepeat(void);
extern int caps_lock;
extern int num_lock;
extern int scroll_lock;
extern int keyselect;

#endif /* KEYBOARD_H_ */
