 @*****************************************************************************
 @ nv12_rgb.S : ARM NEONv1 NV12 to RGB chroma conversion
 @*****************************************************************************
 @ Copyright (C) 2011 Sébastien Toque
 @                    Rémi Denis-Courmont
 @
 @ This program is free software; you can redistribute it and/or modify it
 @ under the terms of the GNU Lesser General Public License as published by
 @ the Free Software Foundation; either version 2.1 of the License, or
 @ (at your option) any later version.
 @
 @ This program is distributed in the hope that it will be useful,
 @ but WITHOUT ANY WARRANTY; without even the implied warranty of
 @ MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 @ GNU Lesser General Public License for more details.
 @
 @ You should have received a copy of the GNU Lesser General Public License
 @ along with this program; if not, write to the Free Software Foundation,
 @ Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 @****************************************************************************/

	.syntax unified
	.fpu neon
	.text

/* ARM */
#define O1	r0
#define O2	r1
#define WIDTH	r2
#define HEIGHT	r3
#define Y1	r4
#define Y2	r5
#define U	r6
#define V	r7
#define YPITCH	r8
#define OPAD	r10
#define YPAD	r11
#define COUNT	ip
#define OPITCH	lr

/* NEON */
#define coefY	D0
#define coefRV	D1
#define coefGU	D2
#define coefGV	D3
#define coefBU	D4
#define Rc	Q3
#define Gc	Q4
#define Bc	Q5

#define u	D24
#define v	D25
#define y1	D28
#define y2	D29

#define chro_r	Q6
#define chro_g	Q7
#define chro_b	Q8
#define red		Q9
#define green	Q10
#define blue	Q11
#define lumi	Q15

#define red1	D24
#define green1	D25
#define blue1	D26
#define alpha1	D27
#define red2	D28
#define green2	D29
#define blue2	D30
#define alpha2	D31

coefficients:
    .short  -15872
    .short    4992
    .short  -18432

	.align 2
	.global nv12_rgb_neon
	.type	nv12_rgb_neon, %function
nv12_rgb_neon:
	push		{r4-r8,r10-r11,lr}
	vpush		{q4-q7}

	/* load arguments */
	ldmia		r0,	{O1, OPITCH}
	ldmia		r1,	{Y1, U, V, YPITCH}

	/* round the width to be a multiple of 16 */
	ands		OPAD, WIDTH, #15
	sub			WIDTH, WIDTH, OPAD
	addne		WIDTH, WIDTH, #16

	/* init constants (scale value by 64) */
	vmov.u8		coefY, #74
	vmov.u8		coefRV, #115
	vmov.u8		coefGU, #14
	vmov.u8		coefGV, #34
	vmov.u8		coefBU, #135
	adr			OPAD, coefficients
	vld1.s16	{d6[], d7[]}, [OPAD]!
	vld1.s16	{d8[], d9[]}, [OPAD]!
	vld1.s16	{d10[], d11[]}, [OPAD]!
	vmov.u8		alpha1, #255

	/* init padding */
	cmp			HEIGHT,	#0
	sub			OPAD,	OPITCH,	WIDTH, lsl #2
	sub			YPAD,	YPITCH,	WIDTH

loop_row:
	movsgt	COUNT,	WIDTH
	add		O2,	O1,	OPITCH
	add		Y2,	Y1,	YPITCH
	/* exit if all rows have been processed */
	vpople	{q4-q7}
	pople	{r4-r8,r10-r11,pc}

loop_col:

	/* Common U & V */

	vld2.u8	{u,v}, [U,:128]!

	vmull.u8	chro_r, v, coefRV
	vmull.u8	chro_g, u, coefGU
	vmlal.u8	chro_g, v, coefGV
	vmull.u8	chro_b, u, coefBU

	vadd.s16	chro_r, Rc, chro_r
	vsub.s16	chro_g, Gc, chro_g
	vadd.s16	chro_b, Bc, chro_b

	pld	[U]

	/* Y Top Row */
	vld2.u8	{y1,y2}, [Y1,:128]!

	/* y1 : chrominance + luminance, then clamp (divide by 64) */
	vmull.u8	lumi, y1, coefY
	vqadd.s16	red, lumi, chro_r
	vqadd.s16	green, lumi, chro_g
	vqadd.s16	blue, lumi, chro_b
	vqrshrun.s16	red1, red, #6
	vqrshrun.s16	green1, green, #6
	vqrshrun.s16	blue1, blue, #6

	/* y2 : chrominance + luminance, then clamp (divide by 64) */
	vmull.u8	lumi, y2, coefY
	vqadd.s16	red, lumi, chro_r
	vqadd.s16	green, lumi, chro_g
	vqadd.s16	blue, lumi, chro_b
	vqrshrun.s16	red2, red, #6
	vqrshrun.s16	green2, green, #6
	vqrshrun.s16	blue2, blue, #6

	pld	[Y1]

	vmov.u8	alpha2, #255
	vzip.u8	red1, red2
	vzip.u8	green1, green2
	vzip.u8	blue1, blue2

	vst4.u8		{red1,green1,blue1,alpha1}, [O1,:128]!
	vst4.u8		{red2,green2,blue2,alpha2}, [O1,:128]!

	/* Y Bottom Row */
	vld2.u8	{y1,y2}, [Y2,:128]!

	/* y1 : chrominance + luminance, then clamp (divide by 64) */
	vmull.u8	lumi, y1, coefY
	vqadd.s16	red, lumi, chro_r
	vqadd.s16	green, lumi, chro_g
	vqadd.s16	blue, lumi, chro_b
	vqrshrun.s16	red1, red, #6
	vqrshrun.s16	green1, green, #6
	vqrshrun.s16	blue1, blue, #6

	/* y2 : chrominance + luminance, then clamp (divide by 64) */
	vmull.u8	lumi, y2, coefY
	vqadd.s16	red, lumi, chro_r
	vqadd.s16	green, lumi, chro_g
	vqadd.s16	blue, lumi, chro_b
	vqrshrun.s16	red2, red, #6
	vqrshrun.s16	green2, green, #6
	vqrshrun.s16	blue2, blue, #6

	pld	[Y2]

	vmov.u8	alpha2, #255
	vzip.u8	red1, red2
	vzip.u8	green1, green2
	vzip.u8	blue1, blue2

	vst4.u8		{red1,green1,blue1,alpha1}, [O2,:128]!
	vst4.u8		{red2,green2,blue2,alpha2}, [O2,:128]!

	/* next columns (x16) */
	subs	COUNT,	COUNT,	#16
	bgt		loop_col

	/* next rows (x2) */
	subs	HEIGHT,	#2
	add		O1,	O2,	OPAD
	add		Y1,	Y2,	YPAD
	add		U,	U,	YPAD
	b		loop_row
