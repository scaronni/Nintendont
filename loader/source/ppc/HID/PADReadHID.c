#include "../../../../common/include/CommonConfig.h"
#include "../global.h"
#include "HID.h"
#include "hidmem.h"

static u32 stubsize = 0x1800;
static vu32 *stubdest = (u32*)0xC1330000;
static vu32 *stubsrc = (u32*)0xD3011810;
static vu16* const _dspReg = (u16*)0xCC005000;
static vu32* reset = (u32*)0xC0002F54;
static vu32* HIDPad = (u32*)0xD3002700;
u32 regs[29];
const s8 DEADZONE = 0x1A;
void _start()
{
	asm volatile(
		"mflr %r5\n"
		"lis %r6, regs@h\n"
		"ori %r6, %r6, regs@l\n"
		"stw %r0, 0(%r6)\n"
		"stw %r1, 4(%r6)\n"
		"stw %r2, 8(%r6)\n"
		"stw %r3, 12(%r6)\n"
		"stw %r4, 16(%r6)\n"
		"stw %r5, 20(%r6)\n"
		"stw %r9, 24(%r6)\n"
		"stw %r10, 28(%r6)\n"
		"stw %r11, 32(%r6)\n"
		"stw %r12, 36(%r6)\n"
		"stw %r13, 40(%r6)\n"
		"stw %r14, 44(%r6)\n"
		"stw %r15, 48(%r6)\n"
		"stw %r16, 52(%r6)\n"
		"stw %r17, 56(%r6)\n"
		"stw %r18, 60(%r6)\n"
		"stw %r19, 64(%r6)\n"
		"stw %r20, 68(%r6)\n"
		"stw %r21, 72(%r6)\n"
		"stw %r22, 76(%r6)\n"
		"stw %r23, 80(%r6)\n"
		"stw %r24, 84(%r6)\n"
		"stw %r25, 88(%r6)\n"
		"stw %r26, 92(%r6)\n"
		"stw %r27, 96(%r6)\n"
		"stw %r28, 100(%r6)\n"
		"stw %r29, 104(%r6)\n"
		"stw %r30, 108(%r6)\n"
		"stw %r31, 112(%r6)\n"
	);

	if(HID_CTRL->Power.Mask &&	//shutdown if power configured and all power buttons pressed
	  ((HID_Packet[HID_CTRL->Power.Offset] & HID_CTRL->Power.Mask) == HID_CTRL->Power.Mask))
	{
		/* stop audio dma */
		_dspReg[27] = (_dspReg[27]&~0x8000);
		/* reset status 1 */
		*reset = 0x1DEA;
		while(*reset == 0x1DEA) ;
		/* load in stub */
		u32 a = (u32)stubdest;
		u32 end = (u32)(stubdest + stubsize);
		for ( ; a < end; a += 32)
		{
			u8 b;
			for(b = 0; b < 4; ++b)
				*stubdest++ = *stubsrc++;
			__asm("dcbi 0,%0 ; sync ; icbi 0,%0" : : "b"(a));
		}
		__asm(
			"sync ; isync\n"
			"lis %r3, 0x8133\n"
			"mtlr %r3\n"
			"blr\n"
		);
	}
	u32 chan = *HIDPad;
	PADStatus *Pad = (PADStatus*)(0x93002800); //PadBuff
	Pad[chan].err = 0;

	/* first buttons */
	u16 button = 0;
	if(HID_CTRL->DPAD == 0)
	{
		if( HID_Packet[HID_CTRL->Left.Offset] & HID_CTRL->Left.Mask )
			button |= PAD_BUTTON_LEFT;

		if( HID_Packet[HID_CTRL->Right.Offset] & HID_CTRL->Right.Mask )
			button |= PAD_BUTTON_RIGHT;

		if( HID_Packet[HID_CTRL->Down.Offset] & HID_CTRL->Down.Mask )
			button |= PAD_BUTTON_DOWN;

		if( HID_Packet[HID_CTRL->Up.Offset] & HID_CTRL->Up.Mask )
			button |= PAD_BUTTON_UP;
	}
	else
	{
		if((HID_Packet[HID_CTRL->Up.Offset] == HID_CTRL->Up.Mask)||(HID_Packet[HID_CTRL->UpLeft.Offset] == HID_CTRL->UpLeft.Mask)||(HID_Packet[HID_CTRL->RightUp.Offset]	== HID_CTRL->RightUp.Mask))
			button |= PAD_BUTTON_UP;

		if(	(HID_Packet[HID_CTRL->Right.Offset] == HID_CTRL->Right.Mask)||(HID_Packet[HID_CTRL->DownRight.Offset] == HID_CTRL->DownRight.Mask)||(HID_Packet[HID_CTRL->RightUp.Offset] == HID_CTRL->RightUp.Mask))
			button |= PAD_BUTTON_RIGHT;

		if((HID_Packet[HID_CTRL->Down.Offset] == HID_CTRL->Down.Mask) ||(HID_Packet[HID_CTRL->DownRight.Offset] == HID_CTRL->DownRight.Mask) ||(HID_Packet[HID_CTRL->DownLeft.Offset] == HID_CTRL->DownLeft.Mask))
			button |= PAD_BUTTON_DOWN;

		if((HID_Packet[HID_CTRL->Left.Offset] == HID_CTRL->Left.Mask) || (HID_Packet[HID_CTRL->DownLeft.Offset] == HID_CTRL->DownLeft.Mask) || (HID_Packet[HID_CTRL->UpLeft.Offset] == HID_CTRL->UpLeft.Mask) )
			button |= PAD_BUTTON_LEFT;
	}
	if(HID_Packet[HID_CTRL->A.Offset] & HID_CTRL->A.Mask)
		button |= PAD_BUTTON_A;
	if(HID_Packet[HID_CTRL->B.Offset] & HID_CTRL->B.Mask)
		button |= PAD_BUTTON_B;
	if(HID_Packet[HID_CTRL->X.Offset] & HID_CTRL->X.Mask)
		button |= PAD_BUTTON_X;
	if(HID_Packet[HID_CTRL->Y.Offset] & HID_CTRL->Y.Mask)
		button |= PAD_BUTTON_Y;
	if(HID_Packet[HID_CTRL->Z.Offset] & HID_CTRL->Z.Mask)
		button |= PAD_TRIGGER_Z;
	if(HID_Packet[HID_CTRL->L.Offset] & HID_CTRL->L.Mask)
		button |= PAD_TRIGGER_L;
	if(HID_Packet[HID_CTRL->R.Offset] & HID_CTRL->R.Mask)
		button |= PAD_TRIGGER_R;
	if(HID_Packet[HID_CTRL->S.Offset] & HID_CTRL->S.Mask)
		button |= PAD_BUTTON_START;
	Pad[chan].button = button;

	/* then analog sticks */
	s8 stickX, stickY, substickX, substickY;
	if ((HID_CTRL->VID == 0x044F) && (HID_CTRL->PID == 0xB303))	//Logitech Thrustmaster Firestorm Dual Analog 2
	{
		stickX		= HID_Packet[HID_CTRL->StickX.Offset];			//raw 80 81...FF 00 ... 7E 7F (left...center...right)
		stickY		= -1 - HID_Packet[HID_CTRL->StickY.Offset];		//raw 80 81...FF 00 ... 7E 7F (up...center...down)
		substickX	= HID_Packet[HID_CTRL->CStickX.Offset];			//raw 80 81...FF 00 ... 7E 7F (left...center...right)
		substickY	= 127 - HID_Packet[HID_CTRL->CStickY.Offset];	//raw 00 01...7F 80 ... FE FF (up...center...down)
	}
	else
	if ((HID_CTRL->VID == 0x0926) && (HID_CTRL->PID == 0x2526))	//Mayflash 3 in 1 Magic Joy Box 
	{
		stickX		= HID_Packet[HID_CTRL->StickX.Offset] - 128;	//raw 1A 1B...80 81 ... E4 E5 (left...center...right)
		stickY		= 127 - HID_Packet[HID_CTRL->StickY.Offset];	//raw 0E 0F...7E 7F ... E4 E5 (up...center...down)
		if (HID_Packet[HID_CTRL->CStickX.Offset] >= 0)
			substickX	= (HID_Packet[HID_CTRL->CStickX.Offset] * 2) - 128;	//raw 90 91 10 11...41 42...68 69 EA EB (left...center...right) the 90 91 EA EB are hard right and left almost to the point of breaking
		else if (HID_Packet[HID_CTRL->CStickX.Offset] < 0xD0)
			substickX	= 0xFE;
		else
			substickX	= 0;
		substickY	= 127 - ((HID_Packet[HID_CTRL->CStickY.Offset] - 128) * 4);	//raw 88 89...9E 9F A0 A1 ... BA BB (up...center...down)
	}
	else	//standard sticks
	{
		stickX		= HID_Packet[HID_CTRL->StickX.Offset] - 128;
		stickY		= 127 - HID_Packet[HID_CTRL->StickY.Offset];
		substickX	= HID_Packet[HID_CTRL->CStickX.Offset] - 128;
		substickY	= 127 - HID_Packet[HID_CTRL->CStickY.Offset];
	}

	s8 tmp_stick = 0;
	if(stickX > HID_CTRL->StickX.DeadZone && stickX > 0)
		tmp_stick = (double)(stickX - HID_CTRL->StickX.DeadZone) * HID_CTRL->StickX.Radius / 1000;
	else if(stickX < -HID_CTRL->StickX.DeadZone && stickX < 0)
		tmp_stick = (double)(stickX + HID_CTRL->StickX.DeadZone) * HID_CTRL->StickX.Radius / 1000;
	Pad[chan].stickX = tmp_stick;

	tmp_stick = 0;
	if(stickY > HID_CTRL->StickY.DeadZone && stickY > 0)
		tmp_stick = (double)(stickY - HID_CTRL->StickY.DeadZone) * HID_CTRL->StickY.Radius / 1000;
	else if(stickY < -HID_CTRL->StickY.DeadZone && stickY < 0)
		tmp_stick = (double)(stickY + HID_CTRL->StickY.DeadZone) * HID_CTRL->StickY.Radius / 1000;
	Pad[chan].stickY = tmp_stick;

	tmp_stick = 0;
	if(substickX > HID_CTRL->CStickX.DeadZone && substickX > 0)
		tmp_stick = (double)(substickX - HID_CTRL->CStickX.DeadZone) * HID_CTRL->CStickX.Radius / 1000;
	else if(substickX < -HID_CTRL->CStickX.DeadZone && substickX < 0)
		tmp_stick = (double)(substickX + HID_CTRL->CStickX.DeadZone) * HID_CTRL->CStickX.Radius / 1000;
	Pad[chan].substickX = tmp_stick;

	tmp_stick = 0;
	if(substickY > HID_CTRL->CStickY.DeadZone && substickY > 0)
		tmp_stick = (double)(substickY - HID_CTRL->CStickY.DeadZone) * HID_CTRL->CStickY.Radius / 1000;
	else if(substickY < -HID_CTRL->CStickY.DeadZone && substickY < 0)
		tmp_stick = (double)(substickY + HID_CTRL->CStickY.DeadZone) * HID_CTRL->CStickY.Radius / 1000;
	Pad[chan].substickY = tmp_stick;
/*
	Pad[chan].stickX = stickX;
	Pad[chan].stickY = stickY;
	Pad[chan].substickX = substickX;
	Pad[chan].substickY = substickY;
*/
	/* then triggers */
	if( HID_CTRL->DigitalLR )
	{	/* digital triggers, not much to do */
		if(Pad[chan].button & PAD_TRIGGER_L)
			Pad[chan].triggerLeft = 255;
		else
			Pad[chan].triggerLeft = 0;
		if(Pad[chan].button & PAD_TRIGGER_R)
			Pad[chan].triggerRight = 255;
		else
			Pad[chan].triggerRight = 0;
	}
	else
	{	/* much to do with analog */
		u8 tmp_triggerL = 0;
		u8 tmp_triggerR = 0;
		if ((HID_CTRL->VID == 0x0926) && (HID_CTRL->PID == 0x2526))	//Mayflash 3 in 1 Magic Joy Box 
		{
			tmp_triggerL =  HID_Packet[HID_CTRL->LAnalog] & 0xF0;	//high nibble raw 1x 2x ... Dx Ex 
			tmp_triggerR = (HID_Packet[HID_CTRL->RAnalog] & 0x0F) * 16 ;	//low nibble raw x1 x2 ...xD xE
			if(Pad[chan].button & PAD_TRIGGER_L)
				tmp_triggerL = 255;
			if(Pad[chan].button & PAD_TRIGGER_R)
				tmp_triggerR = 255;
		}
		else	//standard analog triggers
		{
			tmp_triggerL = HID_Packet[HID_CTRL->LAnalog];
			tmp_triggerR = HID_Packet[HID_CTRL->RAnalog];
		}
		/* Calculate left trigger with deadzone */
		if(tmp_triggerL > DEADZONE)
			Pad[chan].triggerLeft = (tmp_triggerL - DEADZONE) * 1.11f;
		else
			Pad[chan].triggerLeft = 0;
		/* Calculate right trigger with deadzone */
		if(tmp_triggerR > DEADZONE)
			Pad[chan].triggerRight = (tmp_triggerR - DEADZONE) * 1.11f;
		else
			Pad[chan].triggerRight = 0;
	}

	asm volatile(
		"lis %r6, regs@h\n"
		"ori %r6, %r6, regs@l\n"
		"lwz %r0, 0(%r6)\n"
		"lwz %r1, 4(%r6)\n"
		"lwz %r2, 8(%r6)\n"
		"lwz %r3, 12(%r6)\n"
		"lwz %r4, 16(%r6)\n"
		"lwz %r5, 20(%r6)\n"
		"lwz %r9, 24(%r6)\n"
		"lwz %r10, 28(%r6)\n"
		"lwz %r11, 32(%r6)\n"
		"lwz %r12, 36(%r6)\n"
		"lwz %r13, 40(%r6)\n"
		"lwz %r14, 44(%r6)\n"
		"lwz %r15, 48(%r6)\n"
		"lwz %r16, 52(%r6)\n"
		"lwz %r17, 56(%r6)\n"
		"lwz %r18, 60(%r6)\n"
		"lwz %r19, 64(%r6)\n"
		"lwz %r20, 68(%r6)\n"
		"lwz %r21, 72(%r6)\n"
		"lwz %r22, 76(%r6)\n"
		"lwz %r23, 80(%r6)\n"
		"lwz %r24, 84(%r6)\n"
		"lwz %r25, 88(%r6)\n"
		"lwz %r26, 92(%r6)\n"
		"lwz %r27, 96(%r6)\n"
		"lwz %r28, 100(%r6)\n"
		"lwz %r29, 104(%r6)\n"
		"lwz %r30, 108(%r6)\n"
		"lwz %r31, 112(%r6)\n"
		"mtlr %r5\n"
	);
}
