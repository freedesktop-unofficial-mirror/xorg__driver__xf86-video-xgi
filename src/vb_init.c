/* Copyright (C) 2003-2006 by XGI Technology, Taiwan.
 *
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation on the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL XGI AND/OR
 *  ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "osdef.h"
#include "vgatypes.h"


#ifdef LINUX_KERNEL
#include <linux/version.h>
#include <linux/types.h>
#include <linux/delay.h> /* udelay */
#include "XGIfb.h"
#endif

#include "vb_def.h"
#include "vb_struct.h"
#include "vb_util.h"
#include "vb_setmode.h"
#include "vb_init.h"
#include "vb_ext.h"

#ifdef LINUX_XF86
#include "xf86.h"
#include "xf86PciInfo.h"
#include "xgi.h"
#include "xgi_regs.h"
#endif

#ifdef LINUX_KERNEL
#include <asm/io.h>
#include <linux/types.h>
#endif




static UCHAR XGINew_ChannelAB;
static UCHAR XGINew_DataBusWidth;

USHORT XGINew_DRAMType[17][5]={{0x0C,0x0A,0x02,0x40,0x39},{0x0D,0x0A,0x01,0x40,0x48},
                     {0x0C,0x09,0x02,0x20,0x35},{0x0D,0x09,0x01,0x20,0x44},
                     {0x0C,0x08,0x02,0x10,0x31},{0x0D,0x08,0x01,0x10,0x40},
                     {0x0C,0x0A,0x01,0x20,0x34},{0x0C,0x09,0x01,0x08,0x32},
                     {0x0B,0x08,0x02,0x08,0x21},{0x0C,0x08,0x01,0x08,0x30},
                     {0x0A,0x08,0x02,0x04,0x11},{0x0B,0x0A,0x01,0x10,0x28},
                     {0x09,0x08,0x02,0x02,0x01},{0x0B,0x09,0x01,0x08,0x24},
                     {0x0B,0x08,0x01,0x04,0x20},{0x0A,0x08,0x01,0x02,0x10},
                     {0x09,0x08,0x01,0x01,0x00}};

static const USHORT XGINew_SDRDRAM_TYPE[13][5]=
{
    { 2,12, 9,64,0x35},
    { 1,13, 9,64,0x44},
    { 2,12, 8,32,0x31},
    { 2,11, 9,32,0x25},
    { 1,12, 9,32,0x34},
    { 1,13, 8,32,0x40},
    { 2,11, 8,16,0x21},
    { 1,12, 8,16,0x30},
    { 1,11, 9,16,0x24},
    { 1,11, 8, 8,0x20},
    { 2, 9, 8, 4,0x01},
    { 1,10, 8, 4,0x10},
    { 1, 9, 8, 2,0x00}
};

static const USHORT XGINew_DDRDRAM_TYPE[4][5]=
{
    { 2,12, 9,64,0x35},
    { 2,12, 8,32,0x31},
    { 2,11, 8,16,0x21},
    { 2, 9, 8, 4,0x01}
};

static const USHORT XGINew_DDRDRAM_TYPE340[4][5]=
{
    { 2,13, 9,64,0x45},
    { 2,12, 9,32,0x35},
    { 2,12, 8,16,0x31},
    { 2,11, 8, 8,0x21}
};

static void XGINew_SetDRAMSize_340(PXGI_HW_DEVICE_INFO, PVB_DEVICE_INFO);
static void XGINew_SetDRAMSize_XG45(PXGI_HW_DEVICE_INFO, PVB_DEVICE_INFO);
static void XGINew_SetMemoryClock(PXGI_HW_DEVICE_INFO, PVB_DEVICE_INFO);
static void XGINew_SetDRAMModeRegister340(PXGI_HW_DEVICE_INFO);
static void XGINew_SetDRAMDefaultRegister340(PXGI_HW_DEVICE_INFO, USHORT,
    PVB_DEVICE_INFO);
static void XGINew_SetDRAMDefaultRegisterXG45(PXGI_HW_DEVICE_INFO, USHORT, 
    PVB_DEVICE_INFO);
static UCHAR XGINew_Get340DRAMType(PXGI_HW_DEVICE_INFO, PVB_DEVICE_INFO);

static int XGINew_SetDDRChannel(int index, UCHAR ChannelNo,
    UCHAR XGINew_ChannelAB, const USHORT DRAMTYPE_TABLE[][5],
    PVB_DEVICE_INFO pVBInfo);

static void XGINew_SetDRAMSizingType(int index ,
    const USHORT DRAMTYPE_TABLE[][5], PVB_DEVICE_INFO pVBInfo);
static USHORT XGINew_SetDRAMSizeReg(int index,
    const USHORT DRAMTYPE_TABLE[][5], PVB_DEVICE_INFO pVBInfo);

static int XGINew_SetRank(int index, UCHAR RankNo, UCHAR XGINew_ChannelAB,
    const USHORT DRAMTYPE_TABLE[][5], PVB_DEVICE_INFO pVBInfo);

static int XGINew_CheckRanks(int RankNo, int index,
    const USHORT DRAMTYPE_TABLE[][5], PVB_DEVICE_INFO pVBInfo);
static int XGINew_CheckRank(int RankNo, int index,
    const USHORT DRAMTYPE_TABLE[][5], PVB_DEVICE_INFO pVBInfo);
static int XGINew_CheckDDRRank(int RankNo, int index,
    const USHORT DRAMTYPE_TABLE[][5], PVB_DEVICE_INFO pVBInfo);
static int XGINew_CheckDDRRanks(int RankNo, int index,
    const USHORT DRAMTYPE_TABLE[][5], PVB_DEVICE_INFO pVBInfo);

static int XGINew_CheckBanks(int index, const USHORT DRAMTYPE_TABLE[][5],
    PVB_DEVICE_INFO pVBInfo);
static int XGINew_CheckColumn(int index, const USHORT DRAMTYPE_TABLE[][5],
    PVB_DEVICE_INFO pVBInfo);

static int XGINew_DDRSizing340(PXGI_HW_DEVICE_INFO, PVB_DEVICE_INFO);
static int XGINew_DDRSizingXG45(PXGI_HW_DEVICE_INFO, PVB_DEVICE_INFO);
static int XGINew_SDRSizing(PVB_DEVICE_INFO);
static int XGINew_DDRSizing(PVB_DEVICE_INFO);

static void XGINew_DDR_MRS(PVB_DEVICE_INFO pVBInfo);
static void XGINew_SDR_MRS(PVB_DEVICE_INFO pVBInfo);
static void XGINew_DDR1x_MRS_340(PXGI_HW_DEVICE_INFO HwDeviceExtension,
    USHORT P3c4, PVB_DEVICE_INFO pVBInfo);
static void XGINew_DDR2x_MRS_340(PXGI_HW_DEVICE_INFO HwDeviceExtension,
    USHORT P3c4, PVB_DEVICE_INFO pVBInfo);
static void XGINew_DDR2_MRS_340(PXGI_HW_DEVICE_INFO HwDeviceExtension,
    USHORT P3c4, PVB_DEVICE_INFO pVBInfo);
static void XGINew_DDR1x_DefaultRegister(PXGI_HW_DEVICE_INFO HwDeviceExtension,
    USHORT Port, PVB_DEVICE_INFO pVBInfo);
static void XGINew_DDR2x_DefaultRegister(PXGI_HW_DEVICE_INFO HwDeviceExtension,
    USHORT Port, PVB_DEVICE_INFO pVBInfo);
static void XGINew_DDR2_DefaultRegister(PXGI_HW_DEVICE_INFO HwDeviceExtension,
    USHORT Port, PVB_DEVICE_INFO pVBInfo);

static void XGINew_DisableChannelInterleaving(int index, 
    const USHORT XGINew_DDRDRAM_TYPE[][5], PVB_DEVICE_INFO pVBInfo);

static void DualChipInit(PXGI_HW_DEVICE_INFO, PVB_DEVICE_INFO);

static void XGINew_DisableRefresh(PXGI_HW_DEVICE_INFO ,PVB_DEVICE_INFO);
static void XGINew_EnableRefresh(PXGI_HW_DEVICE_INFO, PVB_DEVICE_INFO);

static void XGINew_Delay15us(ULONG);
static void SetPowerConsume(PXGI_HW_DEVICE_INFO, USHORT);
static void XGINew_DDR1x_MRS_XG20(USHORT, PVB_DEVICE_INFO);
static void XGINew_SetDRAMModeRegister_XG20(PXGI_HW_DEVICE_INFO);
static void XGINew_ChkSenseStatus(PXGI_HW_DEVICE_INFO, PVB_DEVICE_INFO);

static int XGINew_ReadWriteRest( USHORT StopAddr, USHORT StartAddr, 
    PVB_DEVICE_INFO pVBInfo);
static int XGI45New_ReadWriteRest(USHORT StopAddr, USHORT StartAddr,
    PVB_DEVICE_INFO pVBInfo);
static UCHAR XGINew_CheckFrequence(PVB_DEVICE_INFO pVBInfo);
static void XGINew_CheckChannel(PXGI_HW_DEVICE_INFO HwDeviceExtension,
				PVB_DEVICE_INFO pVBInfo);

static int XGINew_RAMType;                  /*int      ModeIDOffset,StandTable,CRT1Table,ScreenOffset,REFIndex;*/
static ULONG UNIROM;			  /* UNIROM */


#ifdef LINUX_KERNEL
void DelayUS(ULONG MicroSeconds)
{
	udelay(MicroSeconds);
}
#endif

/* --------------------------------------------------------------------- */
/* Function : XGIInitNew */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
BOOLEAN XGIInitNew( PXGI_HW_DEVICE_INFO HwDeviceExtension )
{

    VB_DEVICE_INFO VBINF;
    PVB_DEVICE_INFO pVBInfo = &VBINF;
#ifndef LINUX_XF86
    USHORT Mclockdata[ 30 ] , Eclockdata[ 30 ] ;
    UCHAR  j , SR11 , SR17 = 0 , SR18 = 0 , SR19 = 0 ;
    UCHAR  CR37 = 0 , CR38 = 0 , CR79 = 0 , CR7A = 0 ,
           CR7B = 0 , CR36 = 0 , CR78 = 0 , CR3C = 0 ,
           CR3D = 0 , CR3E = 0 , CR3F = 0 , CR35 = 0 ;
#endif
    UCHAR   i , temp = 0 , temp1 ,
            VBIOSVersion[ 5 ] ;
    ULONG   base,ChipsetID,VendorID,GraphicVendorID;
    PUCHAR  volatile pVideoMemory;

    /* ULONG j, k ; */

    PXGI_DSReg pSR ;

    ULONG Temp ;


    pVBInfo->ROMAddr = HwDeviceExtension->pjVirtualRomBase ;
    pVBInfo->FBAddr = HwDeviceExtension->pjVideoMemoryAddress ;
    pVBInfo->BaseAddr = ( USHORT )HwDeviceExtension->pjIOAddress ;
    pVideoMemory = ( PUCHAR )pVBInfo->ROMAddr;
    
    pVBInfo->IF_DEF_LCDA = 1 ;
    pVBInfo->IF_DEF_VideoCapture = 0 ;
    pVBInfo->IF_DEF_ScaleLCD = 0 ;
    pVBInfo->IF_DEF_OEMUtil = 0 ;
    pVBInfo->IF_DEF_PWD = 0 ;


    if ( HwDeviceExtension->jChipType == XG20 )			/* kuku 2004/06/25 */
    {
    	pVBInfo->IF_DEF_YPbPr = 0 ;
        pVBInfo->IF_DEF_HiVision = 0 ;
        pVBInfo->IF_DEF_CRT2Monitor = 0 ;
    }
    else if ( HwDeviceExtension->jChipType >= XG40 )
    {
        pVBInfo->IF_DEF_YPbPr = 1 ;
        pVBInfo->IF_DEF_HiVision = 1 ;
        pVBInfo->IF_DEF_CRT2Monitor = 1 ;
    }
    else
    {
        pVBInfo->IF_DEF_YPbPr = 1 ;
        pVBInfo->IF_DEF_HiVision = 1 ;
        pVBInfo->IF_DEF_CRT2Monitor = 0 ;
    }


    Newdebugcode( 0x99 ) ;

   /* if ( pVBInfo->ROMAddr == 0 ) */
   /* return( FALSE ) ; */

    if ( pVBInfo->FBAddr == 0 )
        return( FALSE ) ;

    if ( pVBInfo->BaseAddr == 0 )
        return( FALSE ) ;

    XGI_SetRegByte((XGIIOADDRESS) ( USHORT )( pVBInfo->BaseAddr + 0x12 ) , 0x67 ) ;	/* 3c2 <- 67 ,ynlai */

    pVBInfo->ISXPDOS = 0 ;



    if ( !HwDeviceExtension->bIntegratedMMEnabled )
        return( FALSE ) ;	/* alan */



    XGI_MemoryCopy( VBIOSVersion , HwDeviceExtension->szVBIOSVer , 4 ) ;

    VBIOSVersion[ 4 ] = 0x0 ;

    /* 09/07/99 modify by domao */
    
    
    pVBInfo->P3c4 = pVBInfo->BaseAddr + 0x14 ;
    pVBInfo->P3d4 = pVBInfo->BaseAddr + 0x24 ;
    pVBInfo->P3c0 = pVBInfo->BaseAddr + 0x10 ;
    pVBInfo->P3ce = pVBInfo->BaseAddr + 0x1e ;
    pVBInfo->P3c2 = pVBInfo->BaseAddr + 0x12 ;
    pVBInfo->P3ca = pVBInfo->BaseAddr + 0x1a ;
    pVBInfo->P3c6 = pVBInfo->BaseAddr + 0x16 ;
    pVBInfo->P3c7 = pVBInfo->BaseAddr + 0x17 ;
    pVBInfo->P3c8 = pVBInfo->BaseAddr + 0x18 ;
    pVBInfo->P3c9 = pVBInfo->BaseAddr + 0x19 ;
    pVBInfo->P3da = pVBInfo->BaseAddr + 0x2A ;
    pVBInfo->Part0Port = pVBInfo->BaseAddr + XGI_CRT2_PORT_00 ;
    pVBInfo->Part1Port = pVBInfo->BaseAddr + XGI_CRT2_PORT_04 ;
    pVBInfo->Part2Port = pVBInfo->BaseAddr + XGI_CRT2_PORT_10 ;
    pVBInfo->Part3Port = pVBInfo->BaseAddr + XGI_CRT2_PORT_12 ;
    pVBInfo->Part4Port = pVBInfo->BaseAddr + XGI_CRT2_PORT_14 ;
    pVBInfo->Part5Port = pVBInfo->BaseAddr + XGI_CRT2_PORT_14 + 2 ;

    if ( HwDeviceExtension->jChipType != XG20 )			/* kuku 2004/06/25 */
    XGI_GetVBType( pVBInfo ) ;         /* Run XGI_GetVBType before InitTo330Pointer */

    InitTo330Pointer( HwDeviceExtension->jChipType,  pVBInfo ) ;

    /* ReadVBIOSData */
    ReadVBIOSTablData( HwDeviceExtension->jChipType , pVBInfo) ;

    /* 1.Openkey */
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x05 , 0x86 ) ;



    /* 2.Reset Extended register */

    for( i = 0x06 ; i < 0x20 ; i++ )
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , i , 0 ) ;

    for( i = 0x21 ; i <= 0x27 ; i++ )
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , i , 0 ) ;

    /* for( i = 0x06 ; i <= 0x27 ; i++ ) */
    /* XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , i , 0 ) ; */


    if(( HwDeviceExtension->jChipType == XG20 ) || ( HwDeviceExtension->jChipType >= XG40))
    {
        for( i = 0x31 ; i <= 0x3B ; i++ )
            XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , i , 0 ) ;
    }
    else
    {
        for( i = 0x31 ; i <= 0x3D ; i++ )
            XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , i , 0 ) ;
    }

    if ( HwDeviceExtension->jChipType == XG42 )			/* [Hsuan] 2004/08/20 Auto over driver for XG42 */
      XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x3B , 0xC0 ) ;

    /* for( i = 0x30 ; i <= 0x3F ; i++ ) */
    /* XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , i , 0 ) ; */

    for( i = 0x79 ; i <= 0x7C ; i++ )
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , i , 0 ) ;		/* shampoo 0208 */



    if ( HwDeviceExtension->jChipType == XG20 )
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0x97 , *pVBInfo->pXGINew_CR97 ) ;

    /* 3.SetMemoryClock */
    if ( !( *pVBInfo->pSoftSetting & SoftDRAMType ) )
    {
        if ( HwDeviceExtension->jChipType == XG20 )
        {
            temp = ( UCHAR )XGINew_GetReg1( pVBInfo->P3d4 , 0x97 ) ;
        }
        else if (HwDeviceExtension->jChipType == XG45)
        {
            temp = 0x02 ;
        }
        else
        {
            temp = ( UCHAR )XGINew_GetReg1( pVBInfo->P3c4 , 0x3A ) ;
        }
    }


    if ( HwDeviceExtension->jChipType == XG20 )
    	XGINew_RAMType = temp & 0x01 ;
    else
    {
        XGINew_RAMType = temp & 0x03 ;	/* alan */
    }

    /* Get DRAM type */
    if ( HwDeviceExtension->jChipType == XG45 )
    { }
    else if ( HwDeviceExtension->jChipType >= XG40 )
        XGINew_RAMType = ( int )XGINew_Get340DRAMType( HwDeviceExtension , pVBInfo) ;

    if ( UNIROM == 1 ) XGINew_RAMType = 0;


    if ( HwDeviceExtension->jChipType < XG40 )
        XGINew_SetMemoryClock( HwDeviceExtension , pVBInfo ) ;

    /* 4.SetDefExt1Regs begin */
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x07 , *pVBInfo->pSR07 ) ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x11 , 0x0F ) ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x1F , *pVBInfo->pSR1F ) ;
    /* XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x20 , 0x20 ) ; */
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x20 , 0xA0 ) ;	/* alan, 2001/6/26 Frame buffer can read/write SR20 */


    /* SR11 = 0x0F ; */
    /* XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x11 , SR11 ) ; */


    if ( (HwDeviceExtension->jChipType != XG20) || (HwDeviceExtension->jChipType != XG45) )		/* kuku 2004/06/25 */
    {
    /* Set AGP Rate */
    temp1 = XGINew_GetReg1( pVBInfo->P3c4 , 0x3B ) ;
    temp1 &= 0x02 ;
    if ( temp1 == 0x02 )
    {
        XGI_SetRegLong((XGIIOADDRESS) 0xcf8 , 0x80000000 ) ;
        ChipsetID = XGINew_GetReg3( 0x0cfc ) ;
        XGI_SetRegLong((XGIIOADDRESS) 0xcf8 , 0x8000002C ) ;
        VendorID = XGINew_GetReg3( 0x0cfc ) ;
        VendorID &= 0x0000FFFF ;
        XGI_SetRegLong((XGIIOADDRESS) 0xcf8 , 0x8001002C ) ;
        GraphicVendorID = XGINew_GetReg3( 0x0cfc ) ;
        GraphicVendorID &= 0x0000FFFF;

        if ( ChipsetID == 0x7301039 )
            XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0x5F , 0x09 ) ;

        ChipsetID &= 0x0000FFFF ;

        if ( ( ChipsetID == 0x700E ) || ( ChipsetID == 0x1022 ) || ( ChipsetID == 0x1106 ) || ( ChipsetID == 0x10DE ) )
        {
            if ( ChipsetID == 0x1106 )
            {
                if ( ( VendorID == 0x1019 ) && ( GraphicVendorID == 0x1019 ) )
                    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0x5F , 0x0D ) ;
                else
                    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0x5F , 0x0B ) ;
            }
            else
                XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0x5F , 0x0B ) ;
        }
    }

    if ( HwDeviceExtension->jChipType >= XG40 )
    {
        /* Set AGP customize registers (in SetDefAGPRegs) Start */
        for( i = 0x47 ; i <= 0x4C ; i++ )
            XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , i , pVBInfo->AGPReg[ i - 0x47 ] ) ;

        for( i = 0x70 ; i <= 0x71 ; i++ )
            XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , i , pVBInfo->AGPReg[ 6 + i - 0x70 ] ) ;

        for( i = 0x74 ; i <= 0x77 ; i++ )
            XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , i , pVBInfo->AGPReg[ 8 + i - 0x74 ] ) ;
        /* Set AGP customize registers (in SetDefAGPRegs) End */
        /*[Hsuan]2004/12/14 AGP Input Delay Adjustment on 850 */
        XGI_SetRegLong((XGIIOADDRESS) 0xcf8 , 0x80000000 ) ;		
        ChipsetID = XGINew_GetReg3( 0x0cfc ) ;
        if ( ChipsetID == 0x25308086 )
            XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0x77 , 0xF0 ) ;
            
        HwDeviceExtension->pQueryVGAConfigSpace( HwDeviceExtension , 0x50 , 0 , &Temp ) ;	/* Get */
        Temp >>= 20 ;
        Temp &= 0xF ;

        if ( Temp == 1 )
            XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0x48 , 0x20 ) ;	/* CR48 */
    }

    if ( HwDeviceExtension->jChipType < XG40 )
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0x49 , pVBInfo->CR49[ 0 ] ) ;
    }	/* != XG20 */

    /* Set PCI */
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x23 , *pVBInfo->pSR23 ) ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x24 , *pVBInfo->pSR24 ) ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x25 , pVBInfo->SR25[ 0 ] ) ;

    if ( HwDeviceExtension->jChipType != XG20 )		/* kuku 2004/06/25 */
    {
    /* Set VB */
    XGI_UnLockCRT2( HwDeviceExtension, pVBInfo) ;
    XGINew_SetRegANDOR( pVBInfo->Part0Port , 0x3F , 0xEF , 0x00 ) ;	/* alan, disable VideoCapture */
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port , 0x00 , 0x00 ) ;
    temp1 = ( UCHAR )XGINew_GetReg1( pVBInfo->P3d4 , 0x7B ) ;		/* chk if BCLK>=100MHz */
    temp = ( UCHAR )( ( temp1 >> 4 ) & 0x0F ) ;


        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port , 0x02 , ( *pVBInfo->pCRT2Data_1_2 ) ) ;
    

    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port , 0x2E , 0x08 ) ;	/* use VB */
    } /* != XG20 */


    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x27 , 0x1F ) ;

    if ( ( HwDeviceExtension->jChipType == XG42 ) && XGINew_Get340DRAMType( HwDeviceExtension , pVBInfo) != 0 )	/* Not DDR */
    {
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x31 , ( *pVBInfo->pSR31 & 0x3F ) | 0x40 ) ;
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x32 , ( *pVBInfo->pSR32 & 0xFC ) | 0x01 ) ;
    }
    else
    {
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x31 , *pVBInfo->pSR31 ) ;
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x32 , *pVBInfo->pSR32 ) ;
    }
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x33 , *pVBInfo->pSR33 ) ;



    if ( HwDeviceExtension->jChipType >= XG40 )
      SetPowerConsume ( HwDeviceExtension , pVBInfo->P3c4);

    if ( HwDeviceExtension->jChipType != XG20 )		/* kuku 2004/06/25 */
    {
    if ( XGI_BridgeIsOn( pVBInfo ) == 1 )
    {
        {
            XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port , 0x00 , 0x1C ) ;
            XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port , 0x0D , *pVBInfo->pCRT2Data_4_D ) ;
            XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port , 0x0E , *pVBInfo->pCRT2Data_4_E ) ;
            XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port , 0x10 , *pVBInfo->pCRT2Data_4_10 ) ;
            XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port , 0x0F , 0x3F ) ;
        }

        XGI_LockCRT2( HwDeviceExtension, pVBInfo ) ;
    }
    }	/* != XG20 */

    if ( HwDeviceExtension->jChipType < XG40 )
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0x83 , 0x00 ) ;



    if ( HwDeviceExtension->jChipType >= XG40 )
    {
    	if (HwDeviceExtension->jChipType == XG45)
            XGINew_SetDRAMDefaultRegisterXG45( HwDeviceExtension ,  pVBInfo->P3d4,  pVBInfo ) ;
        else
            XGINew_SetDRAMDefaultRegister340( HwDeviceExtension ,  pVBInfo->P3d4,  pVBInfo ) ;    
        
        if ( HwDeviceExtension->bSkipDramSizing == TRUE )
        {
            pSR = HwDeviceExtension->pSR ;
            if ( pSR!=NULL )
            {
                while( pSR->jIdx != 0xFF )
                {
                    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , pSR->jIdx , pSR->jVal ) ;
                    pSR++ ;
                }
            }
            /* XGINew_SetDRAMModeRegister340( pVBInfo ) ; */
        }   	/* SkipDramSizing */
        else
        {
/*            if ( HwDeviceExtension->jChipType == XG20 )
            {
            	XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x13 , pVBInfo->SR15[0][XGINew_RAMType] ) ;
                XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , pVBInfo->SR15[1][XGINew_RAMType] ) ;
                XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x20 , 0x20 ) ;
            }
            else*/
            if ( HwDeviceExtension->jChipType == XG45 )
                XGINew_SetDRAMSize_XG45( HwDeviceExtension , pVBInfo) ;
            else
                XGINew_SetDRAMSize_340( HwDeviceExtension , pVBInfo) ;
        }
    }		/* XG40 */

 
 

    /* SetDefExt2Regs begin */
/*
    AGP = 1 ;
    temp =( UCHAR )XGINew_GetReg1( pVBInfo->P3c4 , 0x3A ) ;
    temp &= 0x30 ;
    if ( temp == 0x30 )
        AGP = 0 ;

    if ( AGP == 0 )
        *pVBInfo->pSR21 &= 0xEF ;

    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x21 , *pVBInfo->pSR21 ) ;
    if ( AGP == 1 )
        *pVBInfo->pSR22 &= 0x20 ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x22 , *pVBInfo->pSR22 ) ;
*/

    base = 0x80000000 ;
    OutPortLong( 0xcf8 , base ) ;
    Temp = ( InPortLong( 0xcfc ) & 0xFFFF ) ;
    if ( Temp == 0x1039 )
    {
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x22 , ( UCHAR )( ( *pVBInfo->pSR22 ) & 0xFE ) ) ;
    }
    else
    {
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x22 , *pVBInfo->pSR22 ) ;
    }

    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x21 , *pVBInfo->pSR21 ) ;

    if ( HwDeviceExtension->jChipType == XG40 )	/* Initialize seconary chip */
    {
        if ( CheckDualChip(pVBInfo) )
            DualChipInit( HwDeviceExtension , pVBInfo) ;
        /* SetDefExt2Regs end */
    }

    if ( HwDeviceExtension->bSkipSense == FALSE )
    {
        XGI_SenseCRT1(pVBInfo) ;
        /* XGINew_DetectMonitor( HwDeviceExtension ) ; */
        XGI_GetSenseStatus( HwDeviceExtension , pVBInfo ) ;	/* sense CRT2 */
    }

    XGINew_ChkSenseStatus ( HwDeviceExtension , pVBInfo ) ;
    XGINew_SetModeScratch ( HwDeviceExtension , pVBInfo ) ;

    Newdebugcode( 0x88 ) ;

    /* Johnson@062403. To save time for power management. */
    /* DelayMS(1000); */
    /* ~Johnson@062403. */
    /* XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0x32 , 0x28 ) ; //0207 temp */
    /* XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0x36 , 0x02 ) ; //0207 temp */

    return( TRUE ) ;
} /* end of init */



/* --------------------------------------------------------------------- */
/* Function : DualChipInit */
/* Input : */
/* Output : */
/* Description : Initialize the secondary chip. */
/* --------------------------------------------------------------------- */
void DualChipInit( PXGI_HW_DEVICE_INFO HwDeviceExtension ,PVB_DEVICE_INFO pVBInfo)
{
#ifdef LINUX_XF86
    USHORT  BaseAddr2nd = (USHORT)(ULONG)HwDeviceExtension->pj2ndIOAddress ;
#else
    USHORT  BaseAddr2nd = (USHORT)HwDeviceExtension->pj2ndIOAddress ;
#endif
    USHORT  XGINew_P3C3 = pVBInfo->BaseAddr + VIDEO_SUBSYSTEM_ENABLE_PORT ;
    USHORT  XGINew_P3CC = pVBInfo->BaseAddr + MISC_OUTPUT_REG_READ_PORT ;
    USHORT  XGINew_2ndP3C3 = BaseAddr2nd + VIDEO_SUBSYSTEM_ENABLE_PORT ;
    USHORT  XGINew_2ndP3D4 = BaseAddr2nd + CRTC_ADDRESS_PORT_COLOR ;
    USHORT  XGINew_2ndP3C4 = BaseAddr2nd + SEQ_ADDRESS_PORT ;
    USHORT  XGINew_2ndP3C2 = BaseAddr2nd + MISC_OUTPUT_REG_WRITE_PORT ;
    ULONG   Temp ;
    UCHAR   tempal , i ;

    pVBInfo->ROMAddr     = HwDeviceExtension->pjVirtualRomBase ;
    pVBInfo->BaseAddr    = (USHORT)HwDeviceExtension->pjIOAddress ;
    /* Programming Congiguration Space in Secondary Chip */
    /* set CRA1 D[6] = 1 */
    XGINew_SetRegANDOR( pVBInfo->P3d4 , 0xA1 , 0xBF , 0x40 ) ;

    /* Write 2nd Chip Configuration Info into Configuration Space */
    /* Command CNFG04 */
    HwDeviceExtension->pQueryVGAConfigSpace( HwDeviceExtension , PCI_COMMAND , 0 , &Temp ) ; /* Get */
    HwDeviceExtension->pQueryVGAConfigSpace( HwDeviceExtension , PCI_COMMAND + 0x80 , 1 , &Temp ) ; /* Set */
    /* Latency Timer CNFG0C */
    HwDeviceExtension->pQueryVGAConfigSpace( HwDeviceExtension , 0x0c , 0 , &Temp ) ; /* Get */
    HwDeviceExtension->pQueryVGAConfigSpace( HwDeviceExtension , 0x0c + 0x80 , 1 , &Temp ) ; /* Set */
    /* Linear space */
    HwDeviceExtension->pQueryVGAConfigSpace( HwDeviceExtension , 0x10 , 0 , &Temp ) ; /* Get */
    HwDeviceExtension->pQueryVGAConfigSpace( HwDeviceExtension , 0x10 + 0x80 , 1 , &Temp ) ; /* Set */
    /* MMIO space */
    HwDeviceExtension->pQueryVGAConfigSpace( HwDeviceExtension , 0x14 , 0 , &Temp ) ; /* Get */
    Temp += 0x40000;
    HwDeviceExtension->pQueryVGAConfigSpace( HwDeviceExtension , 0x14 + 0x80 , 1 , &Temp ) ; /* Set */
    /* Relocated IO space */
    HwDeviceExtension->pQueryVGAConfigSpace( HwDeviceExtension , 0x18 , 0 , &Temp ) ; /* Get */
    Temp += 0x80;
    HwDeviceExtension->pQueryVGAConfigSpace( HwDeviceExtension , 0x18 + 0x80 , 1 , &Temp ) ; /* Set */
    /* Miscellaneous reg(input port 3cch,output port 3c2h) */
    tempal = ( UCHAR )XGINew_GetReg2( XGINew_P3CC ) ;	/* 3cc */
    XGI_SetRegByte((XGIIOADDRESS) XGINew_2ndP3C2 , tempal ) ;
    /* VGA enable reg(port 3C3h) */
    tempal = ( UCHAR )XGINew_GetReg2( XGINew_P3C3 ) ;	/* 3c3 */
    XGI_SetRegByte((XGIIOADDRESS) XGINew_2ndP3C3 , tempal ) ;
    SetPowerConsume ( HwDeviceExtension , XGINew_2ndP3D4);
    /* ----- CRA0=42, CRA1=81, CRA2=60, CRA3=20, CRA4=50, CRA5=40, CRA8=88 -----// */
    /* ----- CRA9=10, CRAA=80, CRAB=01, CRAC=F1, CRAE=80, CRAF=45, CRB7=24 -----// */
    /* primary chip */
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0xA0 , 0x72 ) ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0xA1 , 0x81 ) ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0xA2 , 0x60 ) ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0xA3 , 0x20 ) ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0xA4 , 0x50 ) ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0xA5 , 0x40 ) ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0xA8 , 0x88 ) ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0xA9 , 0x10 ) ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0xAA , 0x80 ) ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0xAB , 0x01 ) ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0xAC , 0xF1 ) ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0xAE , 0x80 ) ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0xAF , 0x45 ) ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0xB7 , 0x24 ) ;

    /* secondary chip */
    XGI_SetReg((XGIIOADDRESS) XGINew_2ndP3D4 , 0xA0 , 0x72 ) ;
    XGI_SetReg((XGIIOADDRESS) XGINew_2ndP3D4 , 0xA1 , 0x81 ) ;
    XGI_SetReg((XGIIOADDRESS) XGINew_2ndP3D4 , 0xA2 , 0x60 ) ;
    XGI_SetReg((XGIIOADDRESS) XGINew_2ndP3D4 , 0xA3 , 0x20 ) ;
    XGI_SetReg((XGIIOADDRESS) XGINew_2ndP3D4 , 0xA4 , 0x50 ) ;
    XGI_SetReg((XGIIOADDRESS) XGINew_2ndP3D4 , 0xA5 , 0x40 ) ;
    XGI_SetReg((XGIIOADDRESS) XGINew_2ndP3D4 , 0xA8 , 0x88 ) ;
    XGI_SetReg((XGIIOADDRESS) XGINew_2ndP3D4 , 0xA9 , 0x10 ) ;
    XGI_SetReg((XGIIOADDRESS) XGINew_2ndP3D4 , 0xAA , 0x80 ) ;
    XGI_SetReg((XGIIOADDRESS) XGINew_2ndP3D4 , 0xAB , 0x01 ) ;
    XGI_SetReg((XGIIOADDRESS) XGINew_2ndP3D4 , 0xAC , 0xF1 ) ;
    XGI_SetReg((XGIIOADDRESS) XGINew_2ndP3D4 , 0xAE , 0x80 ) ;
    XGI_SetReg((XGIIOADDRESS) XGINew_2ndP3D4 , 0xAF , 0x45 ) ;
    XGI_SetReg((XGIIOADDRESS) XGINew_2ndP3D4 , 0xB7 , 0x24 ) ;

    /* 06/20/2003 [christine] CRT threshold setting request */
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0x78 , 0x40 ) ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0x79 , 0x0C ) ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0x7A , 0x34 ) ;

    /* OpenKey in 2nd chip */
    XGI_SetReg((XGIIOADDRESS) XGINew_2ndP3C4 , 0x05 , 0x86 ) ;

    /* Set PCI registers */
    tempal = (UCHAR)XGINew_GetReg1( pVBInfo->P3c4 , 0x06 ) ;
    XGI_SetReg((XGIIOADDRESS) XGINew_2ndP3C4 , 0x06 , tempal ) ;

    for( i = 0x20 ; i <= 0x25 ; i++ )
    {
        tempal = ( UCHAR )XGINew_GetReg1( pVBInfo->P3c4 , i ) ;
        XGI_SetReg((XGIIOADDRESS) XGINew_2ndP3C4 , i , tempal ) ;
    }
    for(i = 0x31; i <= 0x32; i++ )
    {
        tempal = ( UCHAR )XGINew_GetReg1( pVBInfo->P3c4 , i ) ;
        XGI_SetReg((XGIIOADDRESS) XGINew_2ndP3C4 , i , tempal ) ;
    }
    XGINew_SetDRAMDefaultRegister340( HwDeviceExtension , XGINew_2ndP3D4 , pVBInfo) ;

    for(i = 0x13; i <= 0x14; i++ )
    {
        tempal = ( UCHAR )XGINew_GetReg1( pVBInfo->P3c4 , i ) ;
        XGI_SetReg((XGIIOADDRESS) XGINew_2ndP3C4 , i , tempal ) ;
    }

    /* Close key in 2nd chip */
    XGI_SetReg((XGIIOADDRESS) XGINew_2ndP3C4 , 0x05 , 0x00 ) ;
}




/* ============== alan ====================== */

/* --------------------------------------------------------------------- */
/* Function : XGINew_Get340DRAMType */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
UCHAR XGINew_Get340DRAMType( PXGI_HW_DEVICE_INFO HwDeviceExtension , PVB_DEVICE_INFO pVBInfo)
{
    UCHAR data ;

    if ( HwDeviceExtension->jChipType != XG20 )
    {
        if ( *pVBInfo->pSoftSetting & SoftDRAMType )
        {
            data = *pVBInfo->pSoftSetting & 0x07 ;
            return( data ) ;
        }
        else
        {
            data = XGINew_GetReg1( pVBInfo->P3c4 , 0x39 ) & 0x02 ;

            if ( data == 0 )
                data = ( XGINew_GetReg1( pVBInfo->P3c4 , 0x3A ) & 0x02 ) >> 1 ;

            return( data ) ;
        }
    }
    else
    {
    	data = XGINew_GetReg1( pVBInfo->P3d4 , 0x97 ) & 0x01 ;

    	if ( data == 1 )
            data ++ ;

    	return( data );
    }
}


/* --------------------------------------------------------------------- */
/* Function : XGINew_Delay15us */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
/*
void XGINew_Delay15us(ULONG ulMicrsoSec)
{
}
*/


/* --------------------------------------------------------------------- */
/* Function : XGINew_SDR_MRS */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void XGINew_SDR_MRS(PVB_DEVICE_INFO pVBInfo)
{
    USHORT data ;

    data = XGINew_GetReg1( pVBInfo->P3c4 , 0x16 ) ;
    data &= 0x3F ;          /* SR16 D7=0,D6=0 */
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x16 , data ) ;   /* enable mode register set(MRS) low */
    /* XGINew_Delay15us( 0x100 ) ; */
    data |= 0x80 ;          /* SR16 D7=1,D6=0 */
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x16 , data ) ;   /* enable mode register set(MRS) high */
    /* XGINew_Delay15us( 0x100 ) ; */
}


/* --------------------------------------------------------------------- */
/* Function : XGINew_DDR1x_MRS_340 */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void XGINew_DDR1x_MRS_340(PXGI_HW_DEVICE_INFO HwDeviceExtension, USHORT P3c4,
			  PVB_DEVICE_INFO pVBInfo)
{
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x18 , 0x01 ) ;
    if ( HwDeviceExtension->jChipType == XG42 )		/* XG42 BA0 & BA1  layout change */
        XGI_SetReg((XGIIOADDRESS) P3c4 , 0x19 , 0x40 ) ;
    else
        XGI_SetReg((XGIIOADDRESS) P3c4 , 0x19 , 0x20 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x16 , 0x00 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x16 , 0x80 ) ;

    if ( *pVBInfo->pXGINew_DRAMTypeDefinition != 0x0C )	/* Samsung F Die */
    {
        DelayUS( 3000 ) ;	/* Delay 67 x 3 Delay15us */
        XGI_SetReg((XGIIOADDRESS) P3c4 , 0x18 , 0x00 ) ;
        if ( HwDeviceExtension->jChipType == XG42 )
            XGI_SetReg((XGIIOADDRESS) P3c4 , 0x19 , 0x40 ) ;
        else
            XGI_SetReg((XGIIOADDRESS) P3c4 , 0x19 , 0x20 ) ;
        XGI_SetReg((XGIIOADDRESS) P3c4 , 0x16 , 0x00 ) ;
        XGI_SetReg((XGIIOADDRESS) P3c4 , 0x16 , 0x80 ) ;
    }

    DelayUS( 60 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x18 , pVBInfo->SR15[ 2 ][ XGINew_RAMType ] ) ;	/* SR18 */
    
    if (HwDeviceExtension->jChipType == XG45)
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x19 , 0x01 ) ;				/*TSop DRAM DLL pin jump to A9*/
    else
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x19 , 0x02 ) ;				/*TSop DRAM DLL pin jump to A9*/
    
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x16 , pVBInfo->SR16[ 0 ] ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x16 , pVBInfo->SR16[ 1 ] ) ;
    DelayUS( 1000 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x1B , 0x03 ) ;
    DelayUS( 500 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x18 , pVBInfo->SR15[ 2 ][ XGINew_RAMType ] ) ;	/* SR18 */
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x19 , 0x00 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x16 , pVBInfo->SR16[ 2 ] ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x16 , pVBInfo->SR16[ 3 ] ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x1B , 0x00 ) ;
}


/* --------------------------------------------------------------------- */
/* Function : XGINew_DDR2x_MRS_340 */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void XGINew_DDR2x_MRS_340(PXGI_HW_DEVICE_INFO HwDeviceExtension, USHORT P3c4,
			  PVB_DEVICE_INFO pVBInfo)
{
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x18 , 0x00 ) ;
    if ( HwDeviceExtension->jChipType == XG42 )		/*XG42 BA0 & BA1  layout change*/
        XGI_SetReg((XGIIOADDRESS) P3c4 , 0x19 , 0x40 ) ;
    else
        XGI_SetReg((XGIIOADDRESS) P3c4 , 0x19 , 0x20 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x16 , 0x00 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x16 , 0x80 ) ;
    
    if ( *pVBInfo->pXGINew_DRAMTypeDefinition != 0x0C )	/* Samsung F Die */
    {
        DelayUS( 3000 ) ;	/* Delay 67 x 3 Delay15us */
        XGI_SetReg((XGIIOADDRESS) P3c4 , 0x18 , 0x00 ) ;
        if ( HwDeviceExtension->jChipType == XG42 )
            XGI_SetReg((XGIIOADDRESS) P3c4 , 0x19 , 0x40 ) ;
        else
            XGI_SetReg((XGIIOADDRESS) P3c4 , 0x19 , 0x20 ) ;
        XGI_SetReg((XGIIOADDRESS) P3c4 , 0x16 , 0x00 ) ;
        XGI_SetReg((XGIIOADDRESS) P3c4 , 0x16 , 0x80 ) ;
    }
    
    DelayUS( 60 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x18 , pVBInfo->SR15[ 2 ][ XGINew_RAMType ] ) ;	/* SR18 */
    /* XGI_SetReg((XGIIOADDRESS) P3c4 , 0x18 , 0x31 ) ; */
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x19 , 0x02 ) ;				/*TSop DRAM DLL pin jump to A9*/
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x16 , pVBInfo->SR16[ 0 ] ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x16 , pVBInfo->SR16[ 1 ] ) ;
    DelayUS( 1000 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x1B , 0x03 ) ;
    DelayUS( 500 ) ;
    /* XGI_SetReg((XGIIOADDRESS) P3c4 , 0x18 , 0x31 ) ; */
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x18 , pVBInfo->SR15[ 2 ][ XGINew_RAMType ] ) ;	/* SR18 */
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x19 , 0x00 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x16 , pVBInfo->SR16[ 2 ] ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x16 , pVBInfo->SR16[ 3 ] ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x1B , 0x00 ) ;
}


/* --------------------------------------------------------------------- */
/* Function : XGINew_DDR2_MRS_340 */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void XGINew_DDR2_MRS_340(PXGI_HW_DEVICE_INFO HwDeviceExtension, USHORT P3c4,
			 PVB_DEVICE_INFO pVBInfo)
{
    USHORT P3d4 = P3c4 + 0x10 ;
    UCHAR data ;

    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x28 , 0x64 ) ;	/* SR28 */
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x29 , 0x63 ) ;	/* SR29 */
    DelayUS( 200 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x18 , 0x00 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x19 , 0x20 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x16 , 0x00 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x16 , 0x80 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x18 , 0xC5 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x19 , 0x23 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x16 , 0x00 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x16 , 0x80 ) ;
    DelayUS( 2 ) ;
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x97 , 0x11 ) ;	/* CR97 */

    if( P3c4 != pVBInfo->P3c4 )
    {
        data = XGINew_GetReg1( pVBInfo->P3c4 , 0x28 ) ;
        XGI_SetReg((XGIIOADDRESS) P3c4 , 0x28 , data ) ;	/* SR28 */
        data = XGINew_GetReg1( pVBInfo->P3c4 , 0x29 ) ;
        XGI_SetReg((XGIIOADDRESS) P3c4 , 0x29 , data ) ;	/* SR29 */
        data = XGINew_GetReg1( pVBInfo->P3c4 , 0x2A ) ;
        XGI_SetReg((XGIIOADDRESS) P3c4 , 0x2A , data ) ;	/* SR2A */

        data = XGINew_GetReg1( pVBInfo->P3c4 , 0x2E ) ;
        XGI_SetReg((XGIIOADDRESS) P3c4 , 0x2e , data ) ;	/* SR2E */
        data = XGINew_GetReg1( pVBInfo->P3c4 , 0x2F ) ;
        XGI_SetReg((XGIIOADDRESS) P3c4 , 0x2f , data ) ;	/* SR2F */
        data = XGINew_GetReg1( pVBInfo->P3c4 , 0x30 ) ;
        XGI_SetReg((XGIIOADDRESS) P3c4 , 0x30 , data ) ;	/* SR30 */
    }
    else
        XGINew_SetMemoryClock( HwDeviceExtension , pVBInfo ) ;

    DelayUS( 1000 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x18 , 0xC5 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x19 , 0x23 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x16 , 0x00 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x16 , 0x80 ) ;
    DelayUS( 1 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x1B , 0x04 ) ;	/* SR1B */
    DelayUS( 5) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x1B , 0x00 ) ;	/* SR1B */
    DelayUS( 5 ) ;
    /* XGI_SetReg((XGIIOADDRESS) P3c4 , 0x18 , 0x72 ) ; */
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x18 , pVBInfo->SR15[ 2 ][ XGINew_RAMType ] ) ;	/* SR18 */
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x19 , 0x06 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x16 , 0x05 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x16 , 0x85 ) ;
    DelayUS( 1 ) ;
}


/* --------------------------------------------------------------------- */
/* Function : XGINew_DDR1x_DefaultRegister */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void XGINew_DDR1x_DefaultRegister(PXGI_HW_DEVICE_INFO HwDeviceExtension,
				  USHORT Port, PVB_DEVICE_INFO pVBInfo)
{
    USHORT P3d4 = Port ,
           P3c4 = Port - 0x10 ;
#ifndef LINUX_XF86
    UCHAR  data ;
#endif
    if ( HwDeviceExtension->jChipType == XG20 )
    {
        XGINew_SetMemoryClock( HwDeviceExtension , pVBInfo ) ;
        XGI_SetReg((XGIIOADDRESS) P3d4 , 0x82 , pVBInfo->CR40[ 11 ][ XGINew_RAMType ] ) ;	/* CR82 */
        XGI_SetReg((XGIIOADDRESS) P3d4 , 0x85 , pVBInfo->CR40[ 12 ][ XGINew_RAMType ] ) ;	/* CR85 */
        XGI_SetReg((XGIIOADDRESS) P3d4 , 0x86 , pVBInfo->CR40[ 13 ][ XGINew_RAMType ] ) ;	/* CR86 */

        XGI_SetReg((XGIIOADDRESS) P3d4 , 0x98 , 0x01 ) ;
        XGI_SetReg((XGIIOADDRESS) P3d4 , 0x9A , 0x02 ) ;

        XGINew_DDR1x_MRS_XG20( P3c4 , pVBInfo) ;
    }
    else
    {
        XGINew_SetMemoryClock( HwDeviceExtension , pVBInfo ) ;

        switch( HwDeviceExtension->jChipType )
        {
            case XG41:
            case XG42:
                XGI_SetReg((XGIIOADDRESS) P3d4 , 0x82 , pVBInfo->CR40[ 11 ][ XGINew_RAMType ] ) ;	/* CR82 */
                XGI_SetReg((XGIIOADDRESS) P3d4 , 0x85 , pVBInfo->CR40[ 12 ][ XGINew_RAMType ] ) ;	/* CR85 */
                XGI_SetReg((XGIIOADDRESS) P3d4 , 0x86 , pVBInfo->CR40[ 13 ][ XGINew_RAMType ] ) ;	/* CR86 */
                break ;
            default:
                XGI_SetReg((XGIIOADDRESS) P3d4 , 0x82 , 0x88 ) ;
                XGI_SetReg((XGIIOADDRESS) P3d4 , 0x86 , 0x00 ) ;
                XGINew_GetReg1( P3d4 , 0x86 ) ;				/* Insert read command for delay */
                XGI_SetReg((XGIIOADDRESS) P3d4 , 0x86 , 0x88 ) ;
                XGINew_GetReg1( P3d4 , 0x86 ) ;
                XGI_SetReg((XGIIOADDRESS) P3d4 , 0x86 , pVBInfo->CR40[ 13 ][ XGINew_RAMType ] ) ;
                XGI_SetReg((XGIIOADDRESS) P3d4 , 0x82 , 0x77 ) ;
                XGI_SetReg((XGIIOADDRESS) P3d4 , 0x85 , 0x00 ) ;
                XGINew_GetReg1( P3d4 , 0x85 ) ;				/* Insert read command for delay */
                XGI_SetReg((XGIIOADDRESS) P3d4 , 0x85 , 0x88 ) ;
                XGINew_GetReg1( P3d4 , 0x85 ) ;				/* Insert read command for delay */
                XGI_SetReg((XGIIOADDRESS) P3d4 , 0x85 , pVBInfo->CR40[ 12 ][ XGINew_RAMType ] ) ;	/* CR85 */
                XGI_SetReg((XGIIOADDRESS) P3d4 , 0x82 , pVBInfo->CR40[ 11 ][ XGINew_RAMType ] ) ;	/* CR82 */
                break ;
        }
        if (HwDeviceExtension->jChipType != XG45)
            XGI_SetReg((XGIIOADDRESS) P3d4 , 0x97 , 0x00 ) ;
        XGI_SetReg((XGIIOADDRESS) P3d4 , 0x98 , 0x01 ) ;
        XGI_SetReg((XGIIOADDRESS) P3d4 , 0x9A , 0x02 ) ;
        XGINew_DDR1x_MRS_340( HwDeviceExtension , P3c4 , pVBInfo ) ;
    }
}


/* --------------------------------------------------------------------- */
/* Function : XGINew_DDR2x_DefaultRegister */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void XGINew_DDR2x_DefaultRegister(PXGI_HW_DEVICE_INFO HwDeviceExtension,
				  USHORT Port, PVB_DEVICE_INFO pVBInfo)
{
    USHORT P3d4 = Port ,
           P3c4 = Port - 0x10 ;

#ifndef LINUX_XF86
    UCHAR  data ;
#endif

    XGINew_SetMemoryClock( HwDeviceExtension , pVBInfo ) ;

    /* 20040906 Hsuan modify CR82, CR85, CR86 for XG42 */
    switch( HwDeviceExtension->jChipType )
    {
       case XG41:
       case XG42:
            XGI_SetReg((XGIIOADDRESS) P3d4 , 0x82 , pVBInfo->CR40[ 11 ][ XGINew_RAMType ] ) ;	/* CR82 */
            XGI_SetReg((XGIIOADDRESS) P3d4 , 0x85 , pVBInfo->CR40[ 12 ][ XGINew_RAMType ] ) ;	/* CR85 */
            XGI_SetReg((XGIIOADDRESS) P3d4 , 0x86 , pVBInfo->CR40[ 13 ][ XGINew_RAMType ] ) ;	/* CR86 */
            break ;
       default:
         /* keep following setting sequence, each setting in the same reg insert idle */
         XGI_SetReg((XGIIOADDRESS) P3d4 , 0x82 , 0x88 ) ;
    	 XGI_SetReg((XGIIOADDRESS) P3d4 , 0x86 , 0x00 ) ;
    	 XGINew_GetReg1( P3d4 , 0x86 ) ;				/* Insert read command for delay */
    	 XGI_SetReg((XGIIOADDRESS) P3d4 , 0x86 , 0x88 ) ;
    	 XGI_SetReg((XGIIOADDRESS) P3d4 , 0x82 , 0x77 ) ;
    	 XGI_SetReg((XGIIOADDRESS) P3d4 , 0x85 , 0x00 ) ;
    	 XGINew_GetReg1( P3d4 , 0x85 ) ;				/* Insert read command for delay */
    	 XGI_SetReg((XGIIOADDRESS) P3d4 , 0x85 , 0x88 ) ;
    	 XGINew_GetReg1( P3d4 , 0x85 ) ;				/* Insert read command for delay */
    	 XGI_SetReg((XGIIOADDRESS) P3d4 , 0x85 , pVBInfo->CR40[ 12 ][ XGINew_RAMType ] ) ;	/* CR85 */
    	 XGI_SetReg((XGIIOADDRESS) P3d4 , 0x82 , pVBInfo->CR40[ 11 ][ XGINew_RAMType ] ) ;	/* CR82 */
    }
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x97 , 0x11 ) ;
    if ( HwDeviceExtension->jChipType == XG42 )
    {
      XGI_SetReg((XGIIOADDRESS) P3d4 , 0x98 , 0x01 ) ;
    }
    else
    {
      XGI_SetReg((XGIIOADDRESS) P3d4 , 0x98 , 0x03 ) ;
    }
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x9A , 0x02 ) ;

    XGINew_DDR2x_MRS_340( HwDeviceExtension ,  P3c4 , pVBInfo ) ;
}


/* --------------------------------------------------------------------- */
/* Function : XGINew_DDR2_DefaultRegister */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void XGINew_DDR2_DefaultRegister(PXGI_HW_DEVICE_INFO HwDeviceExtension,
				 USHORT Port, PVB_DEVICE_INFO pVBInfo)
{
    USHORT P3d4 = Port ,
           P3c4 = Port - 0x10 ;

    /* keep following setting sequence, each setting in the same reg insert idle */
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x82 , 0x77 ) ;
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x86 , 0x00 ) ;
    XGINew_GetReg1( P3d4 , 0x86 ) ;				/* Insert read command for delay */
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x86 , 0x88 ) ;
    XGINew_GetReg1( P3d4 , 0x86 ) ;				/* Insert read command for delay */
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x86 , pVBInfo->CR40[ 13 ][ XGINew_RAMType ] ) ;	/* CR86 */
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x82 , 0x77 ) ;
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x85 , 0x00 ) ;
    XGINew_GetReg1( P3d4 , 0x85 ) ;				/* Insert read command for delay */
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x85 , 0x88 ) ;
    XGINew_GetReg1( P3d4 , 0x85 ) ;				/* Insert read command for delay */
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x85 , pVBInfo->CR40[ 12 ][ XGINew_RAMType ] ) ;	/* CR85 */
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x82 , pVBInfo->CR40[ 11 ][ XGINew_RAMType ] ) ;	/* CR82 */

    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x98 , 0x03 ) ;
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x9A , 0x02 ) ;
    XGINew_DDR2_MRS_340( HwDeviceExtension , P3c4, pVBInfo ) ;
}


/* --------------------------------------------------------------------- */
/* Function : XGINew_SetDRAMDefaultRegister340 */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void XGINew_SetDRAMDefaultRegister340( PXGI_HW_DEVICE_INFO HwDeviceExtension ,  USHORT Port , PVB_DEVICE_INFO pVBInfo)
{
    UCHAR temp , temp1 , temp2 , temp3 ,
          i , j , k ;

    USHORT P3d4 = Port ,
           P3c4 = Port - 0x10 ;

    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x6D , pVBInfo->CR40[ 8 ][ XGINew_RAMType ] ) ;
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x68 , pVBInfo->CR40[ 5 ][ XGINew_RAMType ] ) ;
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x69 , pVBInfo->CR40[ 6 ][ XGINew_RAMType ] ) ;
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x6A , pVBInfo->CR40[ 7 ][ XGINew_RAMType ] ) ;

    temp2 = 0 ;
    for( i = 0 ; i < 4 ; i++ )
    {
        temp = pVBInfo->CR6B[ XGINew_RAMType ][ i ] ;        		/* CR6B DQS fine tune delay */
        for( j = 0 ; j < 4 ; j++ )
        {
            temp1 = ( ( temp >> ( 2 * j ) ) & 0x03 ) << 2 ;
            temp2 |= temp1 ;
            XGI_SetReg((XGIIOADDRESS) P3d4 , 0x6B , temp2 ) ;
            XGINew_GetReg1( P3d4 , 0x6B ) ;				/* Insert read command for delay */
            temp2 &= 0xF0 ;
            temp2 += 0x10 ;
        }
    }

    temp2 = 0 ;
    for( i = 0 ; i < 4 ; i++ )
    {
        temp = pVBInfo->CR6E[ XGINew_RAMType ][ i ] ;        		/* CR6E DQM fine tune delay */
        for( j = 0 ; j < 4 ; j++ )
        {
            temp1 = ( ( temp >> ( 2 * j ) ) & 0x03 ) << 2 ;
            temp2 |= temp1 ;
            XGI_SetReg((XGIIOADDRESS) P3d4 , 0x6E , temp2 ) ;
            XGINew_GetReg1( P3d4 , 0x6E ) ;				/* Insert read command for delay */
            temp2 &= 0xF0 ;
            temp2 += 0x10 ;
        }
    }

    temp3 = 0 ;
    for( k = 0 ; k < 4 ; k++ )
    {
        XGINew_SetRegANDOR( P3d4 , 0x6E , 0xFC , temp3 ) ;		/* CR6E_D[1:0] select channel */
        temp2 = 0 ;
        for( i = 0 ; i < 8 ; i++ )
        {
            temp = pVBInfo->CR6F[ XGINew_RAMType ][ 8 * k + i ] ;   	/* CR6F DQ fine tune delay */
            for( j = 0 ; j < 4 ; j++ )
            {
                temp1 = ( temp >> ( 2 * j ) ) & 0x03 ;
                temp2 |= temp1 ;
                XGI_SetReg((XGIIOADDRESS) P3d4 , 0x6F , temp2 ) ;
                XGINew_GetReg1( P3d4 , 0x6F ) ;				/* Insert read command for delay */
                temp2 &= 0xF8 ;
                temp2 += 0x08 ;
            }
        }
        temp3 += 0x01 ;
    }

    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x80 , pVBInfo->CR40[ 9 ][ XGINew_RAMType ] ) ;	/* CR80 */
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x81 , pVBInfo->CR40[ 10 ][ XGINew_RAMType ] ) ;	/* CR81 */

    temp2 = 0x80 ;
    temp = pVBInfo->CR89[ XGINew_RAMType ][ 0 ] ;        		/* CR89 terminator type select */
    for( j = 0 ; j < 4 ; j++ )
    {
        temp1 = ( temp >> ( 2 * j ) ) & 0x03 ;
        temp2 |= temp1 ;
        XGI_SetReg((XGIIOADDRESS) P3d4 , 0x89 , temp2 ) ;
        XGINew_GetReg1( P3d4 , 0x89 ) ;				/* Insert read command for delay */
        temp2 &= 0xF0 ;
        temp2 += 0x10 ;
    }

    temp = pVBInfo->CR89[ XGINew_RAMType ][ 1 ] ;
    temp1 = temp & 0x03 ;
    temp2 |= temp1 ;
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x89 , temp2 ) ;

    temp = pVBInfo->CR40[ 3 ][ XGINew_RAMType ] ;
    temp1 = temp & 0x0F ;
    temp2 = ( temp >> 4 ) & 0x07 ;
    temp3 = temp & 0x80 ;
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x45 , temp1 ) ;	/* CR45 */
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x99 , temp2 ) ;	/* CR99 */
    XGINew_SetRegOR( P3d4 , 0x40 , temp3 ) ;	/* CR40_D[7] */
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x41 , pVBInfo->CR40[ 0 ][ XGINew_RAMType ] ) ;	/* CR41 */

    for( j = 0 ; j <= 6 ; j++ )
        XGI_SetReg((XGIIOADDRESS) P3d4 , ( 0x90 + j ) , pVBInfo->CR40[ 14 + j ][ XGINew_RAMType ] ) ;	/* CR90 - CR96 */

    for( j = 0 ; j <= 2 ; j++ )
        XGI_SetReg((XGIIOADDRESS) P3d4 , ( 0xC3 + j ) , pVBInfo->CR40[ 21 + j ][ XGINew_RAMType ] ) ;	/* CRC3 - CRC5 */

    for( j = 0 ; j < 2 ; j++ )
        XGI_SetReg((XGIIOADDRESS) P3d4 , ( 0x8A + j ) , pVBInfo->CR40[ 1 + j ][ XGINew_RAMType ] ) ;	/* CR8A - CR8B */

    if ( ( HwDeviceExtension->jChipType == XG41 ) || ( HwDeviceExtension->jChipType == XG42 ) )
        XGI_SetReg((XGIIOADDRESS) P3d4 , 0x8C , 0x87 ) ;

    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x59 , pVBInfo->CR40[ 4 ][ XGINew_RAMType ] ) ;	/* CR59 */

    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x83 , 0x09 ) ;	/* CR83 */
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x87 , 0x00 ) ;	/* CR87 */
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0xCF , *pVBInfo->pCRCF ) ;	/* CRCF */
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x1A , 0x87 ) ;	/* SR1A */

    temp = XGINew_Get340DRAMType( HwDeviceExtension, pVBInfo) ;
    if( temp == 0 )
        XGINew_DDR1x_DefaultRegister( HwDeviceExtension, P3d4, pVBInfo ) ;
    else if ( temp == 0x02 )
	XGINew_DDR2x_DefaultRegister( HwDeviceExtension, P3d4, pVBInfo ) ;
    else
   	XGINew_DDR2_DefaultRegister( HwDeviceExtension, P3d4, pVBInfo ) ;

    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x1B , pVBInfo->SR15[ 3 ][ XGINew_RAMType ] ) ;	/* SR1B */
}

/* --------------------------------------------------------------------- */
/* Function : XGINew_SetDRAMDefaultRegisterXG45 */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void XGINew_SetDRAMDefaultRegisterXG45( PXGI_HW_DEVICE_INFO HwDeviceExtension ,  USHORT Port , PVB_DEVICE_INFO pVBInfo)
{
    UCHAR temp , temp1 , temp2 , temp3 ,
          i , j , k ;

    USHORT P3d4 = Port ,
           P3c4 = Port - 0x10 ;

    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x6D , pVBInfo->CR40[ 8 ][ XGINew_RAMType ] ) ;
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x6E , pVBInfo->XG45CR6E[ XGINew_RAMType ] ) ;
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x6F , pVBInfo->XG45CR6F[ XGINew_RAMType ] ) ;
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x68 , pVBInfo->CR40[ 5 ][ XGINew_RAMType ] ) ;
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x69 , pVBInfo->CR40[ 6 ][ XGINew_RAMType ] ) ;
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x6A , pVBInfo->CR40[ 7 ][ XGINew_RAMType ] ) ;

    temp = 0x00 ;
    for ( j = 0 ; j < 24 ; j ++ )
    {
    	XGI_SetReg((XGIIOADDRESS) P3d4 , 0x6B , temp );
        temp += 0x08 ;
    }
    
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x80 , pVBInfo->CR40[ 9 ][ XGINew_RAMType ] ) ;	/* CR80 */
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x81 , pVBInfo->CR40[ 10 ][ XGINew_RAMType ] ) ;	/* CR81 */

    temp2 = 0x80 ;
    temp = pVBInfo->CR89[ XGINew_RAMType ][ 0 ] ;        		/* CR89 terminator type select */
    for( j = 0 ; j < 4 ; j++ )
    {
        temp1 = ( temp >> ( 2 * j ) ) & 0x03 ;
        temp2 |= temp1 ;
        XGI_SetReg((XGIIOADDRESS) P3d4 , 0x89 , temp2 ) ;
        XGINew_GetReg1( P3d4 , 0x89 ) ;				/* Insert read command for delay */
        temp2 &= 0xF0 ;
        temp2 += 0x10 ;
    }

    temp = pVBInfo->CR89[ XGINew_RAMType ][ 1 ] ;
    temp1 = temp & 0x03 ;
    temp2 |= temp1 ;
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x89 , temp2 ) ;

    temp = 0x00 ;
    for ( j = 0 ; j < 3 ; j ++ )
    {
    	XGI_SetReg((XGIIOADDRESS) P3d4 , 0x40 , temp  );
    	temp += 0x40 ;
    }
    
    temp = 0x00 ;
    for ( j = 0 ; j < 24 ; j ++ )
    {
    	XGI_SetReg((XGIIOADDRESS) P3d4 , 0x41 , temp );
        temp += 0x08 ;
    }
    
    temp = 0x00 ;
    for ( j = 0 ; j < 24 ; j ++ )
    {
        XGI_SetReg((XGIIOADDRESS) P3d4 , 0x42 , temp );
        temp += 0x08 ;
    }
    
    for ( k = 0 ; k < 2 ; k ++ )
    {            
        XGINew_SetRegANDOR( P3d4 , 0x43 , ~0x04 , k * 0x04 );
        
        for ( i = 0 ; i < 3 ; i ++ )
        {

    	    XGINew_SetRegANDOR( P3d4 , 0x43 , ~0x03 , i * 0x01 );
    	    
            for ( j = 0 ; j < 32 ; j ++ )
                XGI_SetReg((XGIIOADDRESS) P3d4 , 0x44 , j * 0x08 );
        }    
    }
    
    for ( j = 0 ; j < 3 ; j ++ )            
    	XGI_SetReg((XGIIOADDRESS) P3d4 , 0x45 , j * 0x08 ) ;	/* CR45 */
    
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x97 , 0x84 ) ;
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x98 , 0x01 ) ;
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x99 , 0x22 ) ;
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x9A , 0x02 ) ;
    	
    for( j = 0 ; j <= 6 ; j++ )
        XGI_SetReg((XGIIOADDRESS) P3d4 , ( 0x90 + j ) , pVBInfo->CR40[ 14 + j ][ XGINew_RAMType ] ) ;	/* CR90 - CR96 */

    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x59 , pVBInfo->CR40[ 4 ][ XGINew_RAMType ] ) ;	/* CR59 */
    
    for( j = 0 ; j <= 2 ; j++ )
        XGI_SetReg((XGIIOADDRESS) P3d4 , ( 0xC3 + j ) , pVBInfo->CR40[ 21 + j ][ XGINew_RAMType ] ) ;	/* CRC3 - CRC5 */

    XGI_SetReg((XGIIOADDRESS) P3d4 , 0xC8 , 0x04 ) ;
    
    for( j = 0 ; j < 2 ; j++ )
        XGI_SetReg((XGIIOADDRESS) P3d4 , ( 0x8A + j ) , pVBInfo->CR40[ 1 + j ][ XGINew_RAMType ] ) ;	/* CR8A - CR8B */

    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x8C , 0x40 ) ;
    
    if ( ( HwDeviceExtension->jChipType == XG41 ) || ( HwDeviceExtension->jChipType == XG42 ) )
        XGI_SetReg((XGIIOADDRESS) P3d4 , 0x8C , 0x87 ) ;

    XGI_SetReg((XGIIOADDRESS) P3d4 , 0xCF , *pVBInfo->pCRCF ) ;	/* CRCF */

    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x83 , 0x09 ) ;	/* CR83 */
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x87 , 0x00 ) ;	/* CR87 */
    XGI_SetReg((XGIIOADDRESS) P3d4 , 0x8D , 0x87 ) ;	/* CR8D */
    
    XGINew_DDR1x_DefaultRegister( HwDeviceExtension, P3d4, pVBInfo ) ;
    
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x1A , 0x87 ) ;	/* SR1A */
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x1B , pVBInfo->SR15[ 3 ][ XGINew_RAMType ] ) ;	/* SR1B */
}

/* --------------------------------------------------------------------- */
/* Function : XGINew_DDR_MRS */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void XGINew_DDR_MRS(PVB_DEVICE_INFO pVBInfo)
{
    USHORT data ;

    PUCHAR volatile pVideoMemory = ( PUCHAR )pVBInfo->ROMAddr ;

    /* SR16 <- 1F,DF,2F,AF */
    /* yriver modified SR16 <- 0F,DF,0F,AF */
    /* enable DLL of DDR SD/SGRAM , SR16 D4=1 */
    data = pVideoMemory[ 0xFB ] ;
    /* data = XGINew_GetReg1( pVBInfo->P3c4 , 0x16 ) ; */

    data &= 0x0F ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x16 , data ) ;
    data |= 0xC0 ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x16 , data ) ;
    data &= 0x0F ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x16 , data ) ;
    data |= 0x80 ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x16 , data ) ;
    data &= 0x0F ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x16 , data ) ;
    data |= 0xD0 ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x16 , data ) ;
    data &= 0x0F ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x16 , data ) ;
    data |= 0xA0 ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x16 , data ) ;
/*
   else {
     data &= 0x0F;
     data |= 0x10;
     XGI_SetReg((XGIIOADDRESS)pVBInfo->P3c4,0x16,data);

     if (!(pVBInfo->SR15[1][XGINew_RAMType] & 0x10))
     {
       data &= 0x0F;
     }

     data |= 0xC0;
     XGI_SetReg((XGIIOADDRESS)pVBInfo->P3c4,0x16,data);


     data &= 0x0F;
     data |= 0x20;
     XGI_SetReg((XGIIOADDRESS)pVBInfo->P3c4,0x16,data);
     if (!(pVBInfo->SR15[1][XGINew_RAMType] & 0x10))
     {
       data &= 0x0F;
     }

     data |= 0x80;
     XGI_SetReg((XGIIOADDRESS)pVBInfo->P3c4,0x16,data);
   }
*/
}


/* --------------------------------------------------------------------- */
/* Function : XGINew_SetDRAMSize_340 */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void XGINew_SetDRAMSize_340( PXGI_HW_DEVICE_INFO HwDeviceExtension , PVB_DEVICE_INFO pVBInfo)
{
    USHORT  data ;

    pVBInfo->ROMAddr = HwDeviceExtension->pjVirtualRomBase ;
    pVBInfo->FBAddr = HwDeviceExtension->pjVideoMemoryAddress ;
    XGISetModeNew( HwDeviceExtension , 0x2e ) ;

    data = XGINew_GetReg1( pVBInfo->P3c4 , 0x21 ) ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x21 , ( USHORT )( data & 0xDF ) ) ;	/* disable read cache */

    data = XGINew_GetReg1( pVBInfo->P3c4 , 0x1 ) ;
    data |= 0x20 ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x01 , data ) ;			/* Turn OFF Display */

    XGINew_DDRSizing340( HwDeviceExtension, pVBInfo ) ;

    data=XGINew_GetReg1( pVBInfo->P3c4 , 0x21 ) ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x21 , ( USHORT )( data | 0x20 ) ) ;	/* enable read cache */
}


/*--------------------------------------------------------------------- */
/* Function    : XGINew_SetDRAMSize_XG45 */
/*Input       : */
/*Output      : */
/*Description : */
/*--------------------------------------------------------------------- */
void XGINew_SetDRAMSize_XG45( PXGI_HW_DEVICE_INFO HwDeviceExtension , PVB_DEVICE_INFO pVBInfo)
{
    USHORT  data ;

    pVBInfo->ROMAddr = HwDeviceExtension->pjVirtualRomBase ;
    pVBInfo->FBAddr = HwDeviceExtension->pjVideoMemoryAddress ;
    XGISetModeNew( HwDeviceExtension , 0x2e ) ;

    data = XGINew_GetReg1( pVBInfo->P3c4 , 0x21 ) ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x21 , ( USHORT )( data & 0xDF ) ) ;	/*disable read cache*/  

    data = XGINew_GetReg1( pVBInfo->P3c4 , 0x1 ) ;
    data |= 0x20 ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x01 , data ) ;			/*Turn OFF Display*/

    XGINew_DDRSizingXG45( HwDeviceExtension, pVBInfo ) ;

    data=XGINew_GetReg1( pVBInfo->P3c4 , 0x21 ) ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x21 , ( USHORT )( data | 0x20 ) ) ;	/*enable read cache*/
}


/* --------------------------------------------------------------------- */
/* Function : XGINew_SetDRAMModeRegister340 */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */

void XGINew_SetDRAMModeRegister340( PXGI_HW_DEVICE_INFO HwDeviceExtension )
{
    UCHAR data ;
    VB_DEVICE_INFO VBINF;
    PVB_DEVICE_INFO pVBInfo = &VBINF;
    pVBInfo->ROMAddr = HwDeviceExtension->pjVirtualRomBase ;
    pVBInfo->FBAddr = HwDeviceExtension->pjVideoMemoryAddress ;
    pVBInfo->BaseAddr = ( USHORT )HwDeviceExtension->pjIOAddress ;
    pVBInfo->ISXPDOS = 0 ;

    pVBInfo->P3c4 = pVBInfo->BaseAddr + 0x14 ;
    pVBInfo->P3d4 = pVBInfo->BaseAddr + 0x24 ;
    pVBInfo->P3c0 = pVBInfo->BaseAddr + 0x10 ;
    pVBInfo->P3ce = pVBInfo->BaseAddr + 0x1e ;
    pVBInfo->P3c2 = pVBInfo->BaseAddr + 0x12 ;
    pVBInfo->P3ca = pVBInfo->BaseAddr + 0x1a ;
    pVBInfo->P3c6 = pVBInfo->BaseAddr + 0x16 ;
    pVBInfo->P3c7 = pVBInfo->BaseAddr + 0x17 ;
    pVBInfo->P3c8 = pVBInfo->BaseAddr + 0x18 ;
    pVBInfo->P3c9 = pVBInfo->BaseAddr + 0x19 ;
    pVBInfo->P3da = pVBInfo->BaseAddr + 0x2A ;
    pVBInfo->Part0Port = pVBInfo->BaseAddr + XGI_CRT2_PORT_00 ;
    pVBInfo->Part1Port = pVBInfo->BaseAddr + XGI_CRT2_PORT_04 ;
    pVBInfo->Part2Port = pVBInfo->BaseAddr + XGI_CRT2_PORT_10 ;
    pVBInfo->Part3Port = pVBInfo->BaseAddr + XGI_CRT2_PORT_12 ;
    pVBInfo->Part4Port = pVBInfo->BaseAddr + XGI_CRT2_PORT_14 ;
    pVBInfo->Part5Port = pVBInfo->BaseAddr + XGI_CRT2_PORT_14 + 2 ;

    XGI_GetVBType( pVBInfo ) ;         /* Run XGI_GetVBType before InitTo330Pointer */

    InitTo330Pointer(HwDeviceExtension->jChipType,pVBInfo);

    ReadVBIOSTablData( HwDeviceExtension->jChipType , pVBInfo) ;

    if (HwDeviceExtension->jChipType == XG45)
        XGINew_DDR1x_MRS_340( HwDeviceExtension, pVBInfo->P3c4, pVBInfo ) ;
    else
    {
    if ( XGINew_Get340DRAMType( HwDeviceExtension, pVBInfo) == 0 )
    {
        data = ( XGINew_GetReg1( pVBInfo->P3c4 , 0x39 ) & 0x02 ) >> 1 ;
        if ( data == 0x01 )
            XGINew_DDR2x_MRS_340( HwDeviceExtension, pVBInfo->P3c4, pVBInfo ) ;
        else
            XGINew_DDR1x_MRS_340( HwDeviceExtension, pVBInfo->P3c4, pVBInfo ) ;
    }
    else
        XGINew_DDR2_MRS_340( HwDeviceExtension, pVBInfo->P3c4, pVBInfo);
    }    

    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x1B , 0x03 ) ;
}


/* --------------------------------------------------------------------- */
/* Function : XGINew_DisableRefresh */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void XGINew_DisableRefresh( PXGI_HW_DEVICE_INFO HwDeviceExtension , PVB_DEVICE_INFO pVBInfo)
{
    USHORT  data ;

  
    data = XGINew_GetReg1( pVBInfo->P3c4 , 0x1B ) ;
    data &= 0xF8 ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x1B , data ) ;
  
}


/* --------------------------------------------------------------------- */
/* Function : XGINew_EnableRefresh */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void XGINew_EnableRefresh( PXGI_HW_DEVICE_INFO HwDeviceExtension , PVB_DEVICE_INFO pVBInfo)
{

    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x1B , pVBInfo->SR15[ 3 ][ XGINew_RAMType ] ) ;	/* SR1B */


}


/* --------------------------------------------------------------------- */
/* Function : XGINew_DisableChannelInterleaving */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void XGINew_DisableChannelInterleaving(int index, 
				       const USHORT XGINew_DDRDRAM_TYPE[][5],
				       PVB_DEVICE_INFO pVBInfo)
{
    USHORT data ;

    data = XGINew_GetReg1( pVBInfo->P3c4 , 0x15 ) ;
    data &= 0x1F ;

    switch( XGINew_DDRDRAM_TYPE[ index ][ 3 ] )
    {
        case 64:
            data |= 0 ;
            break ;
        case 32:
            data |= 0x20 ;
            break ;
        case 16:
            data |= 0x40 ;
            break ;
        case 4:
            data |= 0x60 ;
            break ;
        default:
            break ;
    }
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x15 , data ) ;
}


/* --------------------------------------------------------------------- */
/* Function : XGINew_SetDRAMSizingType */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void XGINew_SetDRAMSizingType(int index , const USHORT DRAMTYPE_TABLE[][5],
			      PVB_DEVICE_INFO pVBInfo)
{
    USHORT data ;

    data = DRAMTYPE_TABLE[ index ][ 4 ] ;
    XGINew_SetRegANDOR( pVBInfo->P3c4 , 0x13 , 0x80 , data ) ;
   /* should delay 50 ns */
}


/* --------------------------------------------------------------------- */
/* Function : XGINew_SetRank */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
int XGINew_SetRank(int index, UCHAR RankNo, UCHAR XGINew_ChannelAB,
		   const USHORT DRAMTYPE_TABLE[][5], PVB_DEVICE_INFO pVBInfo)
{
    USHORT data ;
    int RankSize ;

    if ( ( RankNo == 2 ) && ( DRAMTYPE_TABLE[ index ][ 0 ] == 2 ) )
        return 0 ;

    RankSize = DRAMTYPE_TABLE[ index ][ 3 ] / 2 * XGINew_DataBusWidth / 32 ;

    if ( ( RankNo * RankSize ) <= 128 )
    {
        data = 0 ;

        while( ( RankSize >>= 1 ) > 0 )
        {
            data += 0x10 ;
        }
        data |= ( RankNo - 1 ) << 2 ;
        data |= ( XGINew_DataBusWidth / 64 ) & 2 ;
        data |= XGINew_ChannelAB ;
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , data ) ;
        /* should delay */
        XGINew_SDR_MRS( pVBInfo ) ;
        return( 1 ) ;
    }
    else
        return( 0 ) ;
}


/* --------------------------------------------------------------------- */
/* Function : XGINew_SetDDRChannel */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
int XGINew_SetDDRChannel(int index, UCHAR ChannelNo, UCHAR XGINew_ChannelAB,
			 const USHORT DRAMTYPE_TABLE[][5],
			 PVB_DEVICE_INFO pVBInfo)
{
    USHORT  data ;
    int RankSize ;

    RankSize = DRAMTYPE_TABLE[index][3]/2 * XGINew_DataBusWidth/32;
    /* RankSize = DRAMTYPE_TABLE[ index ][ 3 ] ; */
    if ( ChannelNo * RankSize <= 128 )
    {
        data = 0 ;
        while( ( RankSize >>= 1 ) > 0 )
        {
            data += 0x10 ;
        }

        if ( ChannelNo == 2 )
            data |= 0x0C ;

        data |= ( XGINew_DataBusWidth / 32 ) & 2 ;
        data |= XGINew_ChannelAB ;
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , data ) ;
        /* should delay */
        XGINew_DDR_MRS( pVBInfo ) ;
        return( 1 ) ;
    }
    else
        return( 0 ) ;
}


/* --------------------------------------------------------------------- */
/* Function : XGINew_CheckColumn */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
int XGINew_CheckColumn(int index, const USHORT DRAMTYPE_TABLE[][5],
		       PVB_DEVICE_INFO pVBInfo)
{
    int i ;
    ULONG Increment , Position ;

    /* Increment = 1 << ( DRAMTYPE_TABLE[ index ][ 2 ] + XGINew_DataBusWidth / 64 + 1 ) ; */
    Increment = 1 << ( 10 + XGINew_DataBusWidth / 64 ) ;

    for( i = 0 , Position = 0 ; i < 2 ; i++ )
    {
        *( ( PULONG )( pVBInfo->FBAddr + Position ) ) = Position ;
        Position += Increment ;
    }

    for( i = 0 , Position = 0 ; i < 2 ; i++ )
    {
        /* if ( pVBInfo->FBAddr[ Position ] != Position ) */
        if ( ( *( PULONG )( pVBInfo->FBAddr + Position ) ) != Position )
            return( 0 ) ;
        Position += Increment ;
    }
    return( 1 ) ;
}


/* --------------------------------------------------------------------- */
/* Function : XGINew_CheckBanks */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
int XGINew_CheckBanks(int index, const USHORT DRAMTYPE_TABLE[][5],
		      PVB_DEVICE_INFO pVBInfo)
{
    int i ;
    ULONG Increment , Position ;

    Increment = 1 << ( DRAMTYPE_TABLE[ index ][ 2 ] + XGINew_DataBusWidth / 64 + 2 ) ;

    for( i = 0 , Position = 0 ; i < 4 ; i++ )
    {
        /* pVBInfo->FBAddr[ Position ] = Position ; */
        *( ( PULONG )( pVBInfo->FBAddr + Position ) ) = Position ;
        Position += Increment ;
    }

    for( i = 0 , Position = 0 ; i < 4 ; i++ )
    {
        /* if (pVBInfo->FBAddr[ Position ] != Position ) */
        if ( ( *( PULONG )( pVBInfo->FBAddr + Position ) ) != Position )
            return( 0 ) ;
        Position += Increment ;
    }
    return( 1 ) ;
}


/* --------------------------------------------------------------------- */
/* Function : XGINew_CheckRank */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
int XGINew_CheckRank(int RankNo, int index, const USHORT DRAMTYPE_TABLE[][5],
		     PVB_DEVICE_INFO pVBInfo)
{
    int i ;
    ULONG Increment , Position ;

    Increment = 1 << ( DRAMTYPE_TABLE[ index ][ 2 ] + DRAMTYPE_TABLE[ index ][ 1 ] +
                  DRAMTYPE_TABLE[ index ][ 0 ] + XGINew_DataBusWidth / 64 + RankNo ) ;

    for( i = 0 , Position = 0 ; i < 2 ; i++ )
    {
        /* pVBInfo->FBAddr[ Position ] = Position ; */
        /* *( ( PULONG )( pVBInfo->FBAddr ) ) = Position ; */
        *( ( PULONG )( pVBInfo->FBAddr + Position ) ) = Position ;
        Position += Increment ;
    }

    for( i = 0 , Position = 0 ; i < 2 ; i++ )
    {
        /* if ( pVBInfo->FBAddr[ Position ] != Position ) */
        /* if ( ( *( PULONG )( pVBInfo->FBAddr ) ) != Position ) */
        if ( ( *( PULONG )( pVBInfo->FBAddr + Position ) ) != Position )
            return( 0 ) ;
        Position += Increment ;
    }
    return( 1 );
}


/* --------------------------------------------------------------------- */
/* Function : XGINew_CheckDDRRank */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
int XGINew_CheckDDRRank(int RankNo, int index,
			const USHORT DRAMTYPE_TABLE[][5],
			PVB_DEVICE_INFO pVBInfo)
{
    ULONG Increment , Position ;
    USHORT data ;

    Increment = 1 << ( DRAMTYPE_TABLE[ index ][ 2 ] + DRAMTYPE_TABLE[ index ][ 1 ] +
                       DRAMTYPE_TABLE[ index ][ 0 ] + XGINew_DataBusWidth / 64 + RankNo ) ;

    Increment += Increment / 2 ;

    Position = 0;
    *( ( PULONG )( pVBInfo->FBAddr + Position + 0 ) ) = 0x01234567 ;
    *( ( PULONG )( pVBInfo->FBAddr + Position + 1 ) ) = 0x456789AB ;
    *( ( PULONG )( pVBInfo->FBAddr + Position + 2 ) ) = 0x55555555 ;
    *( ( PULONG )( pVBInfo->FBAddr + Position + 3 ) ) = 0x55555555 ;
    *( ( PULONG )( pVBInfo->FBAddr + Position + 4 ) ) = 0xAAAAAAAA ;
    *( ( PULONG )( pVBInfo->FBAddr + Position + 5 ) ) = 0xAAAAAAAA ;

    if ( ( *( PULONG )( pVBInfo->FBAddr + 1 ) ) == 0x456789AB )
        return( 1 ) ;

    if ( ( *( PULONG )( pVBInfo->FBAddr + 0 ) ) == 0x01234567 )
        return( 0 ) ;

    data = XGINew_GetReg1( pVBInfo->P3c4 , 0x14 ) ;
    data &= 0xF3 ;
    data |= 0x0E ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , data ) ;
    data = XGINew_GetReg1( pVBInfo->P3c4 , 0x15 ) ;
    data += 0x20 ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x15 , data ) ;

    return( 1 ) ;
}


/* --------------------------------------------------------------------- */
/* Function : XGINew_CheckRanks */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
int XGINew_CheckRanks(int RankNo, int index, const USHORT DRAMTYPE_TABLE[][5],
		      PVB_DEVICE_INFO pVBInfo)
{
    int r ;

    for( r = RankNo ; r >= 1 ; r-- )
    {
        if ( !XGINew_CheckRank( r , index , DRAMTYPE_TABLE, pVBInfo ) )
            return( 0 ) ;
    }

    if ( !XGINew_CheckBanks( index , DRAMTYPE_TABLE, pVBInfo ) )
        return( 0 ) ;

    if ( !XGINew_CheckColumn( index , DRAMTYPE_TABLE, pVBInfo ) )
        return( 0 ) ;

    return( 1 ) ;
}


/* --------------------------------------------------------------------- */
/* Function : XGINew_CheckDDRRanks */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
int XGINew_CheckDDRRanks(int RankNo, int index,
			 const USHORT DRAMTYPE_TABLE[][5],
			 PVB_DEVICE_INFO pVBInfo)
{
    int r ;

    for( r = RankNo ; r >= 1 ; r-- )
    {
        if ( !XGINew_CheckDDRRank( r , index , DRAMTYPE_TABLE, pVBInfo ) )
            return( 0 ) ;
    }

    if ( !XGINew_CheckBanks( index , DRAMTYPE_TABLE, pVBInfo ) )
        return( 0 ) ;

    if ( !XGINew_CheckColumn( index , DRAMTYPE_TABLE, pVBInfo ) )
        return( 0 ) ;

    return( 1 ) ;
}


/* --------------------------------------------------------------------- */
/* Function : */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
int XGINew_SDRSizing(PVB_DEVICE_INFO pVBInfo)
{
    int    i ;
    UCHAR  j ;

    for( i = 0 ; i < 13 ; i++ )
    {
        XGINew_SetDRAMSizingType( i , XGINew_SDRDRAM_TYPE , pVBInfo) ;

        for( j = 2 ; j > 0 ; j-- )
        {
            if ( !XGINew_SetRank( i , ( UCHAR )j , XGINew_ChannelAB , XGINew_SDRDRAM_TYPE , pVBInfo) )
                continue ;
            else
            {
                if ( XGINew_CheckRanks( j , i , XGINew_SDRDRAM_TYPE, pVBInfo) )
                    return( 1 ) ;
            }
        }
    }
    return( 0 ) ;
}


/* --------------------------------------------------------------------- */
/* Function : XGINew_SetDRAMSizeReg */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
USHORT XGINew_SetDRAMSizeReg(int index, const USHORT DRAMTYPE_TABLE[][5],
			     PVB_DEVICE_INFO pVBInfo)
{
    USHORT data = 0 , memsize = 0 ;
    int RankSize ;
    UCHAR ChannelNo ;

    RankSize = DRAMTYPE_TABLE[ index ][ 3 ] * XGINew_DataBusWidth / 32 ;
    data = XGINew_GetReg1( pVBInfo->P3c4 , 0x13 ) ;
    data &= 0x80 ;

    if ( data == 0x80 )
        RankSize *= 2 ;

    data = 0 ;

    if( XGINew_ChannelAB == 3 )
        ChannelNo = 4 ;
    else
        ChannelNo = XGINew_ChannelAB ;

    if ( ChannelNo * RankSize <= 256 )
    {
        while( ( RankSize >>= 1 ) > 0 )
        {
            data += 0x10 ;
        }

        memsize = data >> 4 ;

        /* [2004/03/25] Vicent, Fix DRAM Sizing Error */
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , ( XGINew_GetReg1( pVBInfo->P3c4 , 0x14 ) & 0x0F ) | ( data & 0xF0 ) ) ;

       /* data |= XGINew_ChannelAB << 2 ; */
       /* data |= ( XGINew_DataBusWidth / 64 ) << 1 ; */
       /* XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , data ) ; */

        /* should delay */
        /* XGINew_SetDRAMModeRegister340( pVBInfo ) ; */
    }
    return( memsize ) ;
}


/* --------------------------------------------------------------------- */
/* Function : XGINew_ReadWriteRest */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
int XGINew_ReadWriteRest( USHORT StopAddr, USHORT StartAddr, 
			  PVB_DEVICE_INFO pVBInfo)
{
    int i ;
    ULONG Position = 0 ;

    *( ( PULONG )( pVBInfo->FBAddr + Position ) ) = Position ;

    for( i = StartAddr ; i <= StopAddr ; i++ )
    {
        Position = 1 << i ;
        *( ( PULONG )( pVBInfo->FBAddr + Position ) ) = Position ;
    }

    DelayUS( 500 ) ;	/* [Vicent] 2004/04/16. Fix #1759 Memory Size error in Multi-Adapter. */

    Position = 0 ;

    if ( ( *( PULONG )( pVBInfo->FBAddr + Position ) ) != Position )
        return( 0 ) ;

    for( i = StartAddr ; i <= StopAddr ; i++ )
    {
        Position = 1 << i ;
        if ( ( *( PULONG )( pVBInfo->FBAddr + Position ) ) != Position )
            return( 0 ) ;
    }
    return( 1 ) ;
}


/*--------------------------------------------------------------------- */
/* Function    : XGI45New_ReadWriteRest */
/* Input       : */
/* Output      : */
/* Description : return 0 : fail, 1 : pass */
/*--------------------------------------------------------------------- */
int XGI45New_ReadWriteRest(USHORT StopAddr, USHORT StartAddr,
			   PVB_DEVICE_INFO pVBInfo)
{
    int i ;
    ULONG Position = 0 ;

    *( ( PULONG )( pVBInfo->FBAddr + Position ) ) = Position ;
    
    for( i = StartAddr ; i <= StopAddr ; i++ )
    {
        Position = 1 << i ;
        *( ( PULONG )( pVBInfo->FBAddr + Position ) ) = Position ;
    }
    
    if ( XGINew_ChannelAB == 4 )
    {
        Position = ( 1 << StopAddr ) + ( 1 << ( StopAddr - 1 ) );
        *( ( PULONG )( pVBInfo->FBAddr + Position ) ) = Position ;
    }	
    
    DelayUS( 500 ) ;	/* [Vicent] 2004/04/16. Fix #1759 Memory Size error in Multi-Adapter. */

    Position = 0 ;
    
    if ( ( *( PULONG )( pVBInfo->FBAddr + Position ) ) != Position )
        return( 0 ) ;
    
    for( i = StartAddr ; i <= StopAddr ; i++ )
    {
        Position = 1 << i ;
        if ( ( *( PULONG )( pVBInfo->FBAddr + Position ) ) != Position )
            return( 0 ) ;
    }
    
    if ( XGINew_ChannelAB == 4 )
    {
        Position = ( 1 << StopAddr ) + ( 1 << ( StopAddr - 1 ) );
        if( ( *( PULONG )( pVBInfo->FBAddr + Position ) ) != Position );
        return( 0 ) ;
    }	
    return( 1 ) ;
}


/* --------------------------------------------------------------------- */
/* Function : XGINew_CheckFrequence */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
UCHAR XGINew_CheckFrequence(PVB_DEVICE_INFO pVBInfo)
{
    UCHAR data ;

    data = XGINew_GetReg1( pVBInfo->P3d4 , 0x97 ) ;

    if ( ( data & 0x10 ) == 0 )
    {
        data = XGINew_GetReg1( pVBInfo->P3c4 , 0x39 ) ;
        data = ( data & 0x02 ) >> 1 ;
        return( data ) ;
    }
    else
        return( data & 0x01 ) ;
}


/* --------------------------------------------------------------------- */
/* Function : XGINew_CheckChannel */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void XGINew_CheckChannel(PXGI_HW_DEVICE_INFO HwDeviceExtension,
			 PVB_DEVICE_INFO pVBInfo)
{
    UCHAR i, data ;

    switch( HwDeviceExtension->jChipType )
    {
      case XG20:
          data = XGINew_GetReg1( pVBInfo->P3d4 , 0x97 ) ;
          data = data & 0x01;
          XGINew_ChannelAB = 1 ;		/* XG20 "JUST" one channel */
          
          if ( data == 0 )  /* Single_32_16 */
          {
              XGINew_DataBusWidth = 32 ;	/* 32 bits */
              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x13 , 0xB1 ) ;  /* 22bit + 2 rank + 32bit */
              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x52 ) ;

              if ( XGINew_ReadWriteRest( 24 , 23 , pVBInfo ) == 1 )
                  return ;

              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x13 , 0x31 ) ;  /* 22bit + 1 rank + 32bit */
              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x42 ) ;

              if ( XGINew_ReadWriteRest( 23 , 23 , pVBInfo ) == 1 )
                  return ;
	      
	      XGINew_DataBusWidth = 16 ;	/* 16 bits */
              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x13 , 0xB1 ) ;  /* 22bit + 2 rank + 16bit */
              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x41 ) ;

              if ( XGINew_ReadWriteRest( 23 , 22 , pVBInfo ) == 1 )
                  return ;
              else
                  XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x13 , 0x31 ) ;

          }
          else  /* Dual_16_8 */
          {					
              XGINew_DataBusWidth = 16 ;	/* 16 bits */
              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x13 , 0xB1 ) ;
              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x41 ) ;

              if ( XGINew_ReadWriteRest( 23 , 22 , pVBInfo ) == 1 )
                  return ;

              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x13 , 0x31 ) ;
              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x31 ) ;

              if ( XGINew_ReadWriteRest( 22 , 22 , pVBInfo ) == 1 )
                  return ;

	      XGINew_DataBusWidth = 8 ;	/* 8 bits */
              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x13 , 0xB1 ) ;
              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x30 ) ;

              if ( XGINew_ReadWriteRest( 22 , 21 , pVBInfo ) == 1 )
                  return ;
              else
                  XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x13 , 0x31 ) ;
          }
          break ;
      
      case XG41:
          if ( XGINew_CheckFrequence(pVBInfo) == 1 )
          {
              XGINew_DataBusWidth = 32 ;	/* 32 bits */
              XGINew_ChannelAB = 3 ;		/* Quad Channel */
              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x13 , 0xA1 ) ;
              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x4C ) ;

              if ( XGINew_ReadWriteRest( 25 , 23 , pVBInfo ) == 1 )
                  return ;

              XGINew_ChannelAB = 2 ;		/* Dual channels */
              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x48 ) ;

              if ( XGINew_ReadWriteRest( 24 , 23 , pVBInfo ) == 1 )
                  return ;

              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x49 ) ;

              if ( XGINew_ReadWriteRest( 24 , 23 , pVBInfo ) == 1 )
                  return ;

              XGINew_ChannelAB = 3 ;
              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x13 , 0x21 ) ;
              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x3C ) ;

              if ( XGINew_ReadWriteRest( 24 , 23 , pVBInfo ) == 1 )
                  return ;

              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x38 ) ;

              if ( XGINew_ReadWriteRest( 8 , 4 , pVBInfo ) == 1 )
                  return ;
              else
                  XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x39 ) ;
          }
          else
          {					/* DDR */
              XGINew_DataBusWidth = 64 ;	/* 64 bits */
              XGINew_ChannelAB = 2 ;		/* Dual channels */
              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x13 , 0xA1 ) ;
              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x5A ) ;

              if ( XGINew_ReadWriteRest( 25 , 24 , pVBInfo ) == 1 )
                  return ;

              XGINew_ChannelAB = 1 ;		/* Single channels */
              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x52 ) ;

              if ( XGINew_ReadWriteRest( 24 , 23 , pVBInfo ) == 1 )
                  return ;

              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x53 ) ;

              if ( XGINew_ReadWriteRest( 24 , 23 , pVBInfo ) == 1 )
                  return ;

              XGINew_ChannelAB = 2 ;		/* Dual channels */
              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x13 , 0x21 ) ;
              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x4A ) ;

              if ( XGINew_ReadWriteRest( 24 , 23 , pVBInfo ) == 1 )
                  return ;

              XGINew_ChannelAB = 1 ;		/* Single channels */
              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x42 ) ;

              if ( XGINew_ReadWriteRest( 8 , 4 , pVBInfo ) == 1 )
                  return ;
              else
                  XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x43 ) ;
          }

          break ;

      case XG42:
/*
      	  XG42 SR14 D[3] Reserve
      	  	    D[2] = 1, Dual Channel
      	  	         = 0, Single Channel

      	  It's Different from Other XG40 Series.
*/
          if ( XGINew_CheckFrequence(pVBInfo) == 1 )	/* DDRII, DDR2x */
          {
              XGINew_DataBusWidth = 32 ;	/* 32 bits */
              XGINew_ChannelAB = 2 ;		/* 2 Channel */
              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x13 , 0xA1 ) ;
              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x44 ) ;

              if ( XGINew_ReadWriteRest( 24 , 23 , pVBInfo ) == 1 )
                  return ;

              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x13 , 0x21 ) ;
              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x34 ) ;
              if ( XGINew_ReadWriteRest( 23 , 22 , pVBInfo ) == 1 )
                  return ;

              XGINew_ChannelAB = 1 ;		/* Single Channel */
              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x13 , 0xA1 ) ;
              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x40 ) ;

              if ( XGINew_ReadWriteRest( 23 , 22 , pVBInfo ) == 1 )
                  return ;
              else
              {
                  XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x13 , 0x21 ) ;
                  XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x30 ) ;
              }
          }
          else
          {					/* DDR */
              XGINew_DataBusWidth = 64 ;	/* 64 bits */
              XGINew_ChannelAB = 1 ;		/* 1 channels */
              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x13 , 0xA1 ) ;
              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x52 ) ;

              if ( XGINew_ReadWriteRest( 24 , 23 , pVBInfo ) == 1 )
                  return ;
              else
              {
                  XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x13 , 0x21 ) ;
                  XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x42 ) ;
              }
          }

          break ;
          
      case XG45: 
      	      
      	   XGINew_DataBusWidth = 64 ;	/* 64 bits */
           XGINew_ChannelAB = 4 ;		/* 3+1 Channel */
           XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x13 , 0x21 ) ;
           XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x4C ) ;
        
           if ( XGI45New_ReadWriteRest( 25 , 24 , pVBInfo ) == 1 )
               return ;
               
           XGINew_ChannelAB = 3 ;		/* 3 Channel */
           XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x13 , 0xA1 ) ;
           XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x58 ) ;
        
           if ( XGI45New_ReadWriteRest( 26 , 24 , pVBInfo ) == 1 )
               return ;    
               
           XGINew_ChannelAB = 2 ;		/* 2 Channel */
           XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x13 , 0xA1 ) ;
           XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x54 ) ;
        
           if ( XGI45New_ReadWriteRest( 25 , 24 , pVBInfo ) == 1 )
               return ; 
               
           XGINew_ChannelAB = 1 ;		/* 1 Channel */
           for ( i = 0; i <= 2; i++)
           {
               XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x13 , 0xA1 ) ;
               XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x50+i ) ;
        
               if ( XGI45New_ReadWriteRest( 24 , 23 , pVBInfo ) == 1 )
                   return ;         
           }
           
           XGINew_ChannelAB = 3 ;		/* 3 Channel */
           XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x13 , 0x21 ) ;
           XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x58 ) ;
        
           if ( XGI45New_ReadWriteRest( 25 , 24 , pVBInfo ) == 1 )
               return ;  
               
           XGINew_ChannelAB = 2 ;		/* 2 Channel */
           XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x13 , 0x21 ) ;
           XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x54 ) ;
        
           if ( XGI45New_ReadWriteRest( 24 , 23 , pVBInfo ) == 1 )
               return ;    
               
           XGINew_ChannelAB = 1 ;		/* 1 Channel */
           for ( i = 0; i <= 2; i++)
           {
               XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x13 , 0x21 ) ;
               XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x50+i ) ;
        
               if ( XGI45New_ReadWriteRest( 23 , 22 , pVBInfo ) == 1 )
                   return ;         
           }               
           break ;
           
      default:	/* XG40 */

          if ( XGINew_CheckFrequence(pVBInfo) == 1 )	/* DDRII */
          {
              XGINew_DataBusWidth = 32 ;	/* 32 bits */
              XGINew_ChannelAB = 3 ;
              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x13 , 0xA1 ) ;
              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x4C ) ;

              if ( XGINew_ReadWriteRest( 25 , 23 , pVBInfo ) == 1 )
                  return ;

              XGINew_ChannelAB = 2 ;		/* 2 channels */
              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x48 ) ;

              if ( XGINew_ReadWriteRest( 24 , 23 , pVBInfo ) == 1 )
                  return ;

              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x13 , 0x21 ) ;
              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x3C ) ;

              if ( XGINew_ReadWriteRest( 24 , 23 , pVBInfo ) == 1 )
                  XGINew_ChannelAB = 3 ;	/* 4 channels */
              else
              {
                  XGINew_ChannelAB = 2 ;	/* 2 channels */
                  XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x38 ) ;
              }
          }
          else
          {					/* DDR */
              XGINew_DataBusWidth = 64 ;	/* 64 bits */
              XGINew_ChannelAB = 2 ;		/* 2 channels */
              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x13 , 0xA1 ) ;
              XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x5A ) ;

              if ( XGINew_ReadWriteRest( 25 , 24 , pVBInfo ) == 1 )
                  return ;
              else
              {
                  XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x13 , 0x21 ) ;
                  XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x14 , 0x4A ) ;
              }
          }
      	  break ;
    }
}


/* --------------------------------------------------------------------- */
/* Function : XGINew_DDRSizing340 */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
int XGINew_DDRSizing340( PXGI_HW_DEVICE_INFO HwDeviceExtension, PVB_DEVICE_INFO pVBInfo)
{
    int i ;
    USHORT memsize , addr ;

    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x15 , 0x00 ) ;	/* noninterleaving */
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x1C , 0x00 ) ;	/* nontiling */
    XGINew_CheckChannel( HwDeviceExtension, pVBInfo ) ;

    for( i = 0 ; i < 4 ; i++ )
    {
        XGINew_SetDRAMSizingType( i , XGINew_DDRDRAM_TYPE340, pVBInfo ) ;
        memsize = XGINew_SetDRAMSizeReg( i , XGINew_DDRDRAM_TYPE340, pVBInfo ) ;
        if ( memsize == 0 )
            continue ;

        addr = memsize + ( XGINew_ChannelAB - 2 ) + 20 ;
        if ( ( HwDeviceExtension->ulVideoMemorySize - 1 ) < ( ULONG )( 1 << addr ) )
            continue ;

        if ( XGINew_ReadWriteRest( addr , 9, pVBInfo ) == 1 )
            return( 1 ) ;
    }
    return( 0 ) ;
}


/*--------------------------------------------------------------------- */
/* Function    : XGINew_DDRSizingXG45 */
/* Input       : */
/* Output      : */
/* Description : */
/*--------------------------------------------------------------------- */
int XGINew_DDRSizingXG45( PXGI_HW_DEVICE_INFO HwDeviceExtension, PVB_DEVICE_INFO pVBInfo)
{
    int i ;
    USHORT memsize , addr ;
    
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x15 , 0x00 ) ;	/* noninterleaving */
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x1C , 0x00 ) ;	/* nontiling */
    XGINew_CheckChannel( HwDeviceExtension, pVBInfo ) ;
    
    for( i = 0 ; i < 4 ; i++ )
    {
        XGINew_SetDRAMSizingType( i , XGINew_DDRDRAM_TYPE340, pVBInfo ) ;
        memsize = XGINew_SetDRAMSizeReg( i , XGINew_DDRDRAM_TYPE340, pVBInfo ) ;
        if ( memsize == 0 )
            continue ;
            
        addr = memsize + ( XGINew_ChannelAB - 2 ) + 20 ;
        if ( ( HwDeviceExtension->ulVideoMemorySize - 1 ) < ( ULONG )( 1 << addr ) )
            continue ;
            
        if ( XGI45New_ReadWriteRest( addr , 9, pVBInfo ) == 1 )
            return( 1 ) ;
    }
    return( 0 ) ;
}


/* --------------------------------------------------------------------- */
/* Function : XGINew_DDRSizing */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
int XGINew_DDRSizing(PVB_DEVICE_INFO pVBInfo)
{
    int    i ;
    UCHAR  j ;

    for( i = 0 ; i < 4 ; i++ )
    {
        XGINew_SetDRAMSizingType( i , XGINew_DDRDRAM_TYPE, pVBInfo ) ;
        XGINew_DisableChannelInterleaving( i , XGINew_DDRDRAM_TYPE , pVBInfo) ;
        for( j = 2 ; j > 0 ; j-- )
        {
            XGINew_SetDDRChannel( i , j , XGINew_ChannelAB , XGINew_DDRDRAM_TYPE , pVBInfo ) ;
            if ( !XGINew_SetRank( i , ( UCHAR )j , XGINew_ChannelAB , XGINew_DDRDRAM_TYPE, pVBInfo ) )
                continue ;
            else
            {
                if ( XGINew_CheckDDRRanks( j , i , XGINew_DDRDRAM_TYPE,  pVBInfo ) )
                return( 1 ) ;
            }
        }
    }
    return( 0 ) ;
}

/* --------------------------------------------------------------------- */
/* Function : XGINew_SetMemoryClock */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void XGINew_SetMemoryClock( PXGI_HW_DEVICE_INFO HwDeviceExtension, PVB_DEVICE_INFO pVBInfo)
{
#ifndef LINUX_XF86
    UCHAR tempal ;
#endif

    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x28 , pVBInfo->MCLKData[ XGINew_RAMType ].SR28 ) ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x29 , pVBInfo->MCLKData[ XGINew_RAMType ].SR29 ) ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x2A , pVBInfo->MCLKData[ XGINew_RAMType ].SR2A ) ;



    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x2E , pVBInfo->ECLKData[ XGINew_RAMType ].SR2E ) ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x2F , pVBInfo->ECLKData[ XGINew_RAMType ].SR2F ) ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x30 , pVBInfo->ECLKData[ XGINew_RAMType ].SR30 ) ;

    /* [Vicent] 2004/07/07, When XG42 ECLK = MCLK = 207MHz, Set SR32 D[1:0] = 10b */
    /* [Hsuan] 2004/08/20, Modify SR32 value, when MCLK=207MHZ, ELCK=250MHz, Set SR32 D[1:0] = 10b */
    if ( HwDeviceExtension->jChipType == XG42 )
    {
      if ( ( pVBInfo->MCLKData[ XGINew_RAMType ].SR28 == 0x1C ) && ( pVBInfo->MCLKData[ XGINew_RAMType ].SR29 == 0x01 )
        && ( ( ( pVBInfo->ECLKData[ XGINew_RAMType ].SR2E == 0x1C ) && ( pVBInfo->ECLKData[ XGINew_RAMType ].SR2F == 0x01 ) )
        || ( ( pVBInfo->ECLKData[ XGINew_RAMType ].SR2E == 0x22 ) && ( pVBInfo->ECLKData[ XGINew_RAMType ].SR2F == 0x01 ) ) ) )
      {
      	XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x32 , ( ( UCHAR )XGINew_GetReg1( pVBInfo->P3c4 , 0x32 ) & 0xFC ) | 0x02 ) ;
      }
    }
}


/* --------------------------------------------------------------------- */
/* input : dx ,valid value : CR or second chip's CR */
/*  */
/* SetPowerConsume : */
/* Description: reduce 40/43 power consumption in first chip or */
/* in second chip, assume CR A1 D[6]="1" in this case */
/* output : none */
/* --------------------------------------------------------------------- */
void SetPowerConsume ( PXGI_HW_DEVICE_INFO HwDeviceExtension , USHORT XGI_P3d4Port )
{
    ULONG   lTemp ;
    UCHAR   bTemp;

    HwDeviceExtension->pQueryVGAConfigSpace( HwDeviceExtension , 0x08 , 0 , &lTemp ) ; /* Get */
    if ((lTemp&0xFF)==0)
    {
        /* set CR58 D[5]=0 D[3]=0 */
        XGINew_SetRegAND( XGI_P3d4Port , 0x58 , 0xD7 ) ;
        bTemp = (UCHAR) XGINew_GetReg1( XGI_P3d4Port , 0xCB ) ;
    	if (bTemp&0x20)
    	{
            if (!(bTemp&0x10))
            {
            	XGINew_SetRegANDOR( XGI_P3d4Port , 0x58 , 0xD7 , 0x20 ) ; /* CR58 D[5]=1 D[3]=0 */
            }
            else
            {
            	XGINew_SetRegANDOR( XGI_P3d4Port , 0x58 , 0xD7 , 0x08 ) ; /* CR58 D[5]=0 D[3]=1 */
            }

    	}

    }
}



#if defined(LINUX_XF86)||defined(LINUX_KERNEL)
void XGINew_InitVBIOSData(PXGI_HW_DEVICE_INFO HwDeviceExtension, PVB_DEVICE_INFO pVBInfo)
{

	/* ULONG ROMAddr = (ULONG)HwDeviceExtension->pjVirtualRomBase; */
    pVBInfo->ROMAddr = HwDeviceExtension->pjVirtualRomBase ;
    pVBInfo->FBAddr = HwDeviceExtension->pjVideoMemoryAddress ;
    pVBInfo->BaseAddr = ( USHORT )HwDeviceExtension->pjIOAddress ;
    pVBInfo->ISXPDOS = 0 ;

    pVBInfo->P3c4 = pVBInfo->BaseAddr + 0x14 ;
    pVBInfo->P3d4 = pVBInfo->BaseAddr + 0x24 ;
    pVBInfo->P3c0 = pVBInfo->BaseAddr + 0x10 ;
    pVBInfo->P3ce = pVBInfo->BaseAddr + 0x1e ;
    pVBInfo->P3c2 = pVBInfo->BaseAddr + 0x12 ;
    pVBInfo->P3ca = pVBInfo->BaseAddr + 0x1a ;
    pVBInfo->P3c6 = pVBInfo->BaseAddr + 0x16 ;
    pVBInfo->P3c7 = pVBInfo->BaseAddr + 0x17 ;
    pVBInfo->P3c8 = pVBInfo->BaseAddr + 0x18 ;
    pVBInfo->P3c9 = pVBInfo->BaseAddr + 0x19 ;
    pVBInfo->P3da = pVBInfo->BaseAddr + 0x2A ;
    pVBInfo->Part0Port = pVBInfo->BaseAddr + XGI_CRT2_PORT_00 ;
    pVBInfo->Part1Port = pVBInfo->BaseAddr + XGI_CRT2_PORT_04 ;
    pVBInfo->Part2Port = pVBInfo->BaseAddr + XGI_CRT2_PORT_10 ;
    pVBInfo->Part3Port = pVBInfo->BaseAddr + XGI_CRT2_PORT_12 ;
    pVBInfo->Part4Port = pVBInfo->BaseAddr + XGI_CRT2_PORT_14 ;
    pVBInfo->Part5Port = pVBInfo->BaseAddr + XGI_CRT2_PORT_14 + 2 ;

    XGI_GetVBType( pVBInfo ) ;         /* Run XGI_GetVBType before InitTo330Pointer */

	switch(HwDeviceExtension->jChipType)
	{
	case XG40:
	case XG41:
	case XG42:
	case XG20:
	default:
		InitTo330Pointer(HwDeviceExtension->jChipType,pVBInfo);
		return ;
	}

}
#endif /* For Linux */

/* --------------------------------------------------------------------- */
/* Function : ReadVBIOSTablData */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void ReadVBIOSTablData( UCHAR ChipType , PVB_DEVICE_INFO pVBInfo)
{
#ifndef LINUX_XF86
    ULONG   ulOffset ;
    UCHAR   temp , index , l ;
#endif
    PUCHAR  volatile pVideoMemory = ( PUCHAR )pVBInfo->ROMAddr ;
    ULONG   i ;
    UCHAR   j , k ;
    ULONG   ii , jj ;

    i = pVideoMemory[ 0x1CF ] | ( pVideoMemory[ 0x1D0 ] << 8 ) ;		/* UniROM */
    if ( i != 0 )
        UNIROM = 1 ;

    ii = 0x90 ;
    for( jj = 0x00 ; jj < 0x08 ; jj++ )
    {
        pVBInfo->MCLKData[ jj ].SR28 = pVideoMemory[ ii ] ;
        pVBInfo->MCLKData[ jj ].SR29 = pVideoMemory[ ii + 1] ;
        pVBInfo->MCLKData[ jj ].SR2A = pVideoMemory[ ii + 2] ;
        pVBInfo->MCLKData[ jj ].CLOCK = pVideoMemory[ ii + 3 ] | ( pVideoMemory[ ii + 4 ] << 8 ) ;
        ii += 0x05 ;
    }

    ii = 0xB8 ;
    for( jj = 0x00 ; jj < 0x08 ; jj++ )
    {
        pVBInfo->ECLKData[ jj ].SR2E = pVideoMemory[ ii ] ;
        pVBInfo->ECLKData[ jj ].SR2F=pVideoMemory[ ii + 1 ] ;
        pVBInfo->ECLKData[ jj ].SR30= pVideoMemory[ ii + 2 ] ;
        pVBInfo->ECLKData[ jj ].CLOCK= pVideoMemory[ ii + 3 ] | ( pVideoMemory[ ii + 4 ] << 8 ) ;
        ii += 0x05 ;
    }

    /* Volari customize data area start */
    /* if ( ChipType == XG40 ) */
    if ( ChipType >= XG40 )
    {
        ii = 0xE0 ;
        for( jj = 0x00 ; jj < 0x03 ; jj++ )
        {
            pVBInfo->SR15[ jj ][ 0 ] = pVideoMemory[ ii ] ;		/* SR13, SR14, and SR18 */
            pVBInfo->SR15[ jj ][ 1 ] = pVideoMemory[ ii + 1 ] ;
            pVBInfo->SR15[ jj ][ 2 ] = pVideoMemory[ ii + 2 ] ;
            pVBInfo->SR15[ jj ][ 3 ] = pVideoMemory[ ii + 3 ] ;
            pVBInfo->SR15[ jj ][ 4 ] = pVideoMemory[ ii + 4 ] ;
            pVBInfo->SR15[ jj ][ 5 ] = pVideoMemory[ ii + 5 ] ;
            pVBInfo->SR15[ jj ][ 6 ] = pVideoMemory[ ii + 6 ] ;
            pVBInfo->SR15[ jj ][ 7 ] = pVideoMemory[ ii + 7 ] ;
            ii += 0x08 ;
        }
        ii = 0x110 ;
        jj = 0x03 ;
        pVBInfo->SR15[ jj ][ 0 ] = pVideoMemory[ ii ] ;		/* SR1B */
        pVBInfo->SR15[ jj ][ 1 ] = pVideoMemory[ ii + 1 ] ;
        pVBInfo->SR15[ jj ][ 2 ] = pVideoMemory[ ii + 2 ] ;
        pVBInfo->SR15[ jj ][ 3 ] = pVideoMemory[ ii + 3 ] ;
        pVBInfo->SR15[ jj ][ 4 ] = pVideoMemory[ ii + 4 ] ;
        pVBInfo->SR15[ jj ][ 5 ] = pVideoMemory[ ii + 5 ] ;
        pVBInfo->SR15[ jj ][ 6 ] = pVideoMemory[ ii + 6 ] ;
        pVBInfo->SR15[ jj ][ 7 ] = pVideoMemory[ ii + 7 ] ;

        *pVBInfo->pSR07 = pVideoMemory[ 0x74 ] ;
        *pVBInfo->pSR1F = pVideoMemory[ 0x75 ] ;
        *pVBInfo->pSR21 = pVideoMemory[ 0x76 ] ;
        *pVBInfo->pSR22 = pVideoMemory[ 0x77 ] ;
        *pVBInfo->pSR23 = pVideoMemory[ 0x78 ] ;
        *pVBInfo->pSR24 = pVideoMemory[ 0x79 ] ;
        pVBInfo->SR25[ 0 ] = pVideoMemory[ 0x7A ] ;
        *pVBInfo->pSR31 = pVideoMemory[ 0x7B ] ;
        *pVBInfo->pSR32 = pVideoMemory[ 0x7C ] ;
        *pVBInfo->pSR33 = pVideoMemory[ 0x7D ] ;
        ii = 0xF8 ;

        for( jj = 0 ; jj < 3 ; jj++ )
        {
            pVBInfo->CR40[ jj ][ 0 ] = pVideoMemory[ ii ] ;
            pVBInfo->CR40[ jj ][ 1 ] = pVideoMemory[ ii + 1 ] ;
            pVBInfo->CR40[ jj ][ 2 ] = pVideoMemory[ ii + 2 ] ;
            pVBInfo->CR40[ jj ][ 3 ] = pVideoMemory[ ii + 3 ] ;
            pVBInfo->CR40[ jj ][ 4 ] = pVideoMemory[ ii + 4 ] ;
            pVBInfo->CR40[ jj ][ 5 ] = pVideoMemory[ ii + 5 ] ;
            pVBInfo->CR40[ jj ][ 6 ] = pVideoMemory[ ii + 6 ] ;
            pVBInfo->CR40[ jj ][ 7 ] = pVideoMemory[ ii + 7 ] ;
            ii += 0x08 ;
        }

        ii = 0x118 ;
        for( j = 3 ; j < 24 ; j++ )
        {
            pVBInfo->CR40[ j ][ 0 ] = pVideoMemory[ ii ] ;
            pVBInfo->CR40[ j ][ 1 ] = pVideoMemory[ ii + 1 ] ;
            pVBInfo->CR40[ j ][ 2 ] = pVideoMemory[ ii + 2 ] ;
            pVBInfo->CR40[ j ][ 3 ] = pVideoMemory[ ii + 3 ] ;
            pVBInfo->CR40[ j ][ 4 ] = pVideoMemory[ ii + 4 ] ;
            pVBInfo->CR40[ j ][ 5 ] = pVideoMemory[ ii + 5 ] ;
            pVBInfo->CR40[ j ][ 6 ] = pVideoMemory[ ii + 6 ] ;
            pVBInfo->CR40[ j ][ 7 ] = pVideoMemory[ ii + 7 ] ;
            ii += 0x08 ;
        }

        i = pVideoMemory[ 0x1C0 ] | ( pVideoMemory[ 0x1C1 ] << 8 ) ;

        for( j = 0 ; j < 8 ; j++ )
        {
            for( k = 0 ; k < 4 ; k++ )
                pVBInfo->CR6B[ j ][ k ] = pVideoMemory[ i + 4 * j + k ] ;
        }

        i = pVideoMemory[ 0x1C2 ] | ( pVideoMemory[ 0x1C3 ] << 8 ) ;

        if (ChipType == XG45)
        {
        for( j = 0 ; j < 8 ; j++ )
        {
            pVBInfo->XG45CR6E[ j ] = pVideoMemory[i] ;
        }
        }
        else
        {
        for( j = 0 ; j < 8 ; j++ )
        {
            for( k = 0 ; k < 4 ; k++ )
                pVBInfo->CR6E[ j ][ k ] = pVideoMemory[ i + 4 * j + k ] ;
        }
	}

        i = pVideoMemory[ 0x1C4 ] | ( pVideoMemory[ 0x1C5 ] << 8 ) ;
        if (ChipType == XG45)
        {
        for( j = 0 ; j < 8 ; j++ )
        {
            pVBInfo->XG45CR6F[ j ] = pVideoMemory[i] ;
        }
	}
	else
	{
        for( j = 0 ; j < 8 ; j++ )
        {
            for( k = 0 ; k < 32 ; k++ )
                pVBInfo->CR6F[ j ][ k ] = pVideoMemory[ i + 32 * j + k ] ;
        }
        }

        i = pVideoMemory[ 0x1C6 ] | ( pVideoMemory[ 0x1C7 ] << 8 ) ;

        for( j = 0 ; j < 8 ; j++ )
        {
            for( k = 0 ; k < 2 ; k++ )
                pVBInfo->CR89[ j ][ k ] = pVideoMemory[ i + 2 * j + k ] ;
        }

        i = pVideoMemory[ 0x1C8 ] | ( pVideoMemory[ 0x1C9 ] << 8 ) ;
        for( j = 0 ; j < 12 ; j++ )
            pVBInfo->AGPReg[ j ] = pVideoMemory[ i + j ] ;

        i = pVideoMemory[ 0x1CF ] | ( pVideoMemory[ 0x1D0 ] << 8 ) ;
        for( j = 0 ; j < 4 ; j++ )
            pVBInfo->SR16[ j ] = pVideoMemory[ i + j ] ;

        *pVBInfo->pCRCF = pVideoMemory[ 0x1CA ] ;
        *pVBInfo->pXGINew_DRAMTypeDefinition = pVideoMemory[ 0x1CB ] ;
        *pVBInfo->pXGINew_I2CDefinition = pVideoMemory[ 0x1D1 ] ;
        if ( ChipType == XG20 )
           *pVBInfo->pXGINew_CR97 = pVideoMemory[ 0x1D2 ] ;
    }
    /* Volari customize data area end */
}

/* --------------------------------------------------------------------- */
/* Function : XGINew_DDR1x_MRS_XG20 */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void XGINew_DDR1x_MRS_XG20( USHORT P3c4 , PVB_DEVICE_INFO pVBInfo)
{

    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x18 , 0x01 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x19 , 0x40 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x16 , 0x00 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x16 , 0x80 ) ;
    DelayUS( 60 ) ;

    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x18 , 0x00 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x19 , 0x40 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x16 , 0x00 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x16 , 0x80 ) ;
    DelayUS( 60 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x18 , pVBInfo->SR15[ 2 ][ XGINew_RAMType ] ) ;	/* SR18 */
    /* XGI_SetReg((XGIIOADDRESS) P3c4 , 0x18 , 0x31 ) ; */
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x19 , 0x01 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x16 , 0x03 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x16 , 0x83 ) ;
    DelayUS( 1000 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x1B , 0x03 ) ;
    DelayUS( 500 ) ;
    /* XGI_SetReg((XGIIOADDRESS) P3c4 , 0x18 , 0x31 ) ; */
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x18 , pVBInfo->SR15[ 2 ][ XGINew_RAMType ] ) ;	/* SR18 */
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x19 , 0x00 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x16 , 0x03 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x16 , 0x83 ) ;
    XGI_SetReg((XGIIOADDRESS) P3c4 , 0x1B , 0x00 ) ;
}

/* --------------------------------------------------------------------- */
/* Function : XGINew_SetDRAMModeRegister_XG20 */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void XGINew_SetDRAMModeRegister_XG20( PXGI_HW_DEVICE_INFO HwDeviceExtension )
{
#ifndef LINUX_XF86
    UCHAR data ;
#endif
    VB_DEVICE_INFO VBINF;
    PVB_DEVICE_INFO pVBInfo = &VBINF;
    pVBInfo->ROMAddr = HwDeviceExtension->pjVirtualRomBase ;
    pVBInfo->FBAddr = HwDeviceExtension->pjVideoMemoryAddress ;
    pVBInfo->BaseAddr = ( USHORT )HwDeviceExtension->pjIOAddress ;
    pVBInfo->ISXPDOS = 0 ;

    pVBInfo->P3c4 = pVBInfo->BaseAddr + 0x14 ;
    pVBInfo->P3d4 = pVBInfo->BaseAddr + 0x24 ;
    pVBInfo->P3c0 = pVBInfo->BaseAddr + 0x10 ;
    pVBInfo->P3ce = pVBInfo->BaseAddr + 0x1e ;
    pVBInfo->P3c2 = pVBInfo->BaseAddr + 0x12 ;
    pVBInfo->P3ca = pVBInfo->BaseAddr + 0x1a ;
    pVBInfo->P3c6 = pVBInfo->BaseAddr + 0x16 ;
    pVBInfo->P3c7 = pVBInfo->BaseAddr + 0x17 ;
    pVBInfo->P3c8 = pVBInfo->BaseAddr + 0x18 ;
    pVBInfo->P3c9 = pVBInfo->BaseAddr + 0x19 ;
    pVBInfo->P3da = pVBInfo->BaseAddr + 0x2A ;
    pVBInfo->Part0Port = pVBInfo->BaseAddr + XGI_CRT2_PORT_00 ;
    pVBInfo->Part1Port = pVBInfo->BaseAddr + XGI_CRT2_PORT_04 ;
    pVBInfo->Part2Port = pVBInfo->BaseAddr + XGI_CRT2_PORT_10 ;
    pVBInfo->Part3Port = pVBInfo->BaseAddr + XGI_CRT2_PORT_12 ;
    pVBInfo->Part4Port = pVBInfo->BaseAddr + XGI_CRT2_PORT_14 ;
    pVBInfo->Part5Port = pVBInfo->BaseAddr + XGI_CRT2_PORT_14 + 2 ;

    InitTo330Pointer(HwDeviceExtension->jChipType,pVBInfo);

    ReadVBIOSTablData( HwDeviceExtension->jChipType , pVBInfo) ;

    if ( XGINew_Get340DRAMType( HwDeviceExtension, pVBInfo) == 0 )
        XGINew_DDR1x_MRS_XG20( pVBInfo->P3c4, pVBInfo ) ;
    else
        XGINew_DDR2x_MRS_340( HwDeviceExtension,  pVBInfo->P3c4, pVBInfo ) ;

    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x1B , 0x03 ) ;
}

/* -------------------------------------------------------- */
/* Function : XGINew_ChkSenseStatus */
/* Input : */
/* Output : */
/* Description : */
/* -------------------------------------------------------- */
void XGINew_ChkSenseStatus ( PXGI_HW_DEVICE_INFO HwDeviceExtension , PVB_DEVICE_INFO pVBInfo)
{
    USHORT tempbx=0 , temp , tempcx , CR3CData;

    temp = XGINew_GetReg1( pVBInfo->P3d4 , 0x32 ) ;

    if ( temp & Monitor1Sense )
    	tempbx |= ActiveCRT1 ;
    if ( temp & LCDSense )
    	tempbx |= ActiveLCD ;
    if ( temp & Monitor2Sense )
    	tempbx |= ActiveCRT2 ;
    if ( temp & TVSense )
    {
    	tempbx |= ActiveTV ;
    	if ( temp & AVIDEOSense )
    	    tempbx |= ( ActiveAVideo << 8 );
    	if ( temp & SVIDEOSense )
    	    tempbx |= ( ActiveSVideo << 8 );
    	if ( temp & SCARTSense )
    	    tempbx |= ( ActiveSCART << 8 );
    	if ( temp & HiTVSense )
    	    tempbx |= ( ActiveHiTV << 8 );
    	if ( temp & YPbPrSense )
    	    tempbx |= ( ActiveYPbPr << 8 );
    }

    tempcx = XGINew_GetReg1( pVBInfo->P3d4 , 0x3d ) ;
    tempcx |= ( XGINew_GetReg1( pVBInfo->P3d4 , 0x3e ) << 8 ) ;

    if ( tempbx & tempcx )
    {
    	CR3CData = XGINew_GetReg1( pVBInfo->P3d4 , 0x3c ) ;
    	if ( !( CR3CData & DisplayDeviceFromCMOS ) )
    	{
    	    tempcx = 0x1FF0 ;
    	    if ( *pVBInfo->pSoftSetting & ModeSoftSetting )
    	    {
    	    	tempbx = 0x1FF0 ;
    	    }
    	}
    }
    else
    {
    	tempcx = 0x1FF0 ;
    	if ( *pVBInfo->pSoftSetting & ModeSoftSetting )
    	{
    	    tempbx = 0x1FF0 ;
    	}
    }

    tempbx &= tempcx ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x3d , ( tempbx & 0x00FF ) ) ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x3e , ( ( tempbx & 0xFF00 ) >> 8 )) ;
}
/* -------------------------------------------------------- */
/* Function : XGINew_SetModeScratch */
/* Input : */
/* Output : */
/* Description : */
/* -------------------------------------------------------- */
void XGINew_SetModeScratch ( PXGI_HW_DEVICE_INFO HwDeviceExtension , PVB_DEVICE_INFO pVBInfo )
{
    USHORT temp , tempcl = 0 , tempch = 0 , CR31Data , CR38Data;

    temp = XGINew_GetReg1( pVBInfo->P3d4 , 0x3d ) ;
    temp |= XGINew_GetReg1( pVBInfo->P3d4 , 0x3e ) << 8 ;
    temp |= ( XGINew_GetReg1( pVBInfo->P3d4 , 0x31 ) & ( DriverMode >> 8) ) << 8 ;

    if ( pVBInfo->IF_DEF_CRT2Monitor == 1)
    {
    	if ( temp & ActiveCRT2 )
    	   tempcl = SetCRT2ToRAMDAC ;
    }

    if ( temp & ActiveLCD )
    {
    	tempcl |= SetCRT2ToLCD ;
    	if  ( temp & DriverMode )
    	{
    	    if ( temp & ActiveTV )
    	    {
    	    	tempch = SetToLCDA | EnableDualEdge ;
    	    	temp ^= SetCRT2ToLCD ;

    	    	if ( ( temp >> 8 ) & ActiveAVideo )
    	    	    tempcl |= SetCRT2ToAVIDEO ;
    	    	if ( ( temp >> 8 ) & ActiveSVideo )
    	    	    tempcl |= SetCRT2ToSVIDEO ;
    	    	if ( ( temp >> 8 ) & ActiveSCART )
    	    	    tempcl |= SetCRT2ToSCART ;

    	    	if ( pVBInfo->IF_DEF_HiVision == 1 )
    	    	{
    	    	    if ( ( temp >> 8 ) & ActiveHiTV )
    	    	    tempcl |= SetCRT2ToHiVisionTV ;
    	    	}

    	    	if ( pVBInfo->IF_DEF_YPbPr == 1 )
    	    	{
    	    	    if ( ( temp >> 8 ) & ActiveYPbPr )
    	    	    tempch |= SetYPbPr ;
    	    	}
    	    }
    	}
    }
    else
    {
    	if ( ( temp >> 8 ) & ActiveAVideo )
    	   tempcl |= SetCRT2ToAVIDEO ;
    	if ( ( temp >> 8 ) & ActiveSVideo )
  	   tempcl |= SetCRT2ToSVIDEO ;
    	if ( ( temp >> 8 ) & ActiveSCART )
   	   tempcl |= SetCRT2ToSCART ;

   	if ( pVBInfo->IF_DEF_HiVision == 1 )
    	{
    	   if ( ( temp >> 8 ) & ActiveHiTV )
    	   tempcl |= SetCRT2ToHiVisionTV ;
    	}

    	if ( pVBInfo->IF_DEF_YPbPr == 1 )
    	{
    	   if ( ( temp >> 8 ) & ActiveYPbPr )
    	   tempch |= SetYPbPr ;
    	}
    }


    tempcl |= SetSimuScanMode ;
    if ( (!( temp & ActiveCRT1 )) && ( ( temp & ActiveLCD ) || ( temp & ActiveTV ) || ( temp & ActiveCRT2 ) ) )
       tempcl ^= ( SetSimuScanMode | SwitchToCRT2 ) ;
    if ( ( temp & ActiveLCD ) && ( temp & ActiveTV ) )
       tempcl ^= ( SetSimuScanMode | SwitchToCRT2 ) ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x30 , tempcl ) ;

    CR31Data = XGINew_GetReg1( pVBInfo->P3d4 , 0x31 ) ;
    CR31Data &= ~( SetNotSimuMode >> 8 ) ;
    if ( !( temp & ActiveCRT1 ) )
        CR31Data |= ( SetNotSimuMode >> 8 ) ;
    CR31Data &= ~( DisableCRT2Display >> 8 ) ;
    if  (!( ( temp & ActiveLCD ) || ( temp & ActiveTV ) || ( temp & ActiveCRT2 ) ) )
        CR31Data |= ( DisableCRT2Display >> 8 ) ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x31 , CR31Data ) ;

    CR38Data = XGINew_GetReg1( pVBInfo->P3d4 , 0x38 ) ;
    CR38Data &= ~SetYPbPr ;
    CR38Data |= tempch ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x38 , CR38Data ) ;

}
