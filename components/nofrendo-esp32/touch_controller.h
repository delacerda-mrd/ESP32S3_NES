// FT6336G capacitive touch driver, translated into the same active-low 16-bit
// input word the emulator's osd_getinput() ev[] table expects (see video_audio.c):
// bit0=Select, bit3=Start, bit4=Up, bit5=Right, bit6=Down, bit7=Left, bit13=A, bit14=B.
#ifndef _TOUCH_CONTROLLER_H_
#define _TOUCH_CONTROLLER_H_

#ifdef __cplusplus
extern "C" {
#endif

void touchInit(void);
int touchReadInput(void);

#ifdef __cplusplus
}
#endif

#endif
