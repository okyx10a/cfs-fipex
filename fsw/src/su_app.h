/*******************************************************************************
** File: su_app.h
**
** Purpose:
**   This file is main hdr file for the SU application.
**
**
*******************************************************************************/

#ifndef _su_app_h_
#define _su_app_h_

/*
** Required header files.
*/
#include "cfe.h"
#include "cfe_error.h"
#include "cfe_evs.h"
#include "cfe_sb.h"
#include "cfe_es.h"

#include <string.h>
#include <errno.h>
#include <unistd.h>

/***********************************************************************/

#define SU_PIPE_DEPTH                     32

/************************************************************************
** Type Definitions
*************************************************************************/

/****************************************************************************/
/*
** Local function prototypes.
**
** Note: Except for the entry point (SU_AppMain), these
**       functions are not called from any other source module.
*/
void SU_AppMain(void);
void SU_AppInit(void);
void SU_ProcessCommandPacket(void);
void SU_ProcessGroundCommand(void);
void SU_ReportHousekeeping(void);
void SU_ResetCounters(void);

boolean SU_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength);

#endif /* _su_app_h_ */
