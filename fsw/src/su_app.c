/*******************************************************************************
** File: su_app.c
**
** Purpose:
**   This file contains the source code for the Sample App.
**
*******************************************************************************/

/*
**   Include Files:
*/

#include "su_app.h"
#include "su_app_perfids.h"
#include "su_app_msgids.h"
#include "su_app_msg.h"
#include "su_app_events.h"
#include "su_app_version.h"

/*
** global data
*/

su_hk_tlm_t    SU_HkTelemetryPkt;
CFE_SB_PipeId_t    SU_CommandPipe;
CFE_SB_MsgPtr_t    SUMsgPtr;

static CFE_EVS_BinFilter_t  SU_EventFilters[] =
       {  /* Event ID    mask */
          {SU_STARTUP_INF_EID,       0x0000},
          {SU_COMMAND_ERR_EID,       0x0000},
          {SU_COMMANDNOP_INF_EID,    0x0000},
          {SU_COMMANDRST_INF_EID,    0x0000},
       };

/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* SU_AppMain() -- Application entry point and main process loop          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void SU_AppMain( void )
{
    int32  status;
    uint32 RunStatus = CFE_ES_APP_RUN;

    CFE_ES_PerfLogEntry(SU_APP_PERF_ID);

    status = SU_AppInit();

    if(status != CFE_SUCCESS)
    {
         /* Set request to terminate main loop */
        RunStatus = CFE_ES_APP_ERROR;
    }     

    /*
    ** SU Runloop
    */
    while (CFE_ES_RunLoop(&RunStatus) == TRUE)
    {
        CFE_ES_PerfLogExit(SU_APP_PERF_ID);

        /* Pend on receipt of command packet -- timeout set to 500 millisecs */
        status = CFE_SB_RcvMsg(&SUMsgPtr, SU_CommandPipe, 500);
        
        CFE_ES_PerfLogEntry(SU_APP_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            SU_ProcessCommandPacket();
        }

    }

    CFE_ES_ExitApp(RunStatus);

} /* End of SU_AppMain() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* SU_AppInit() --  initialization                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void SU_AppInit(void)
{  
    int32 result;
    /*
    ** Register the app with Executive services
    */
    result = CFE_ES_RegisterApp();

    /*
    ** Register the events
    */ 
    
    CFE_EVS_Register(SU_EventFilters,
                     sizeof(SU_EventFilters)/sizeof(CFE_EVS_BinFilter_t),
                     CFE_EVS_BINARY_FILTER);

    /*
    ** Create the Software Bus command pipe and subscribe to housekeeping
    **  messages
    */
    CFE_SB_CreatePipe(&SU_CommandPipe, SU_PIPE_DEPTH,"SU_CMD_PIPE");
    CFE_SB_Subscribe(SU_APP_CMD_MID, SU_CommandPipe);
    CFE_SB_Subscribe(SU_APP_SEND_HK_MID, SU_CommandPipe);

    SU_ResetCounters();

    CFE_SB_InitMsg(&SU_HkTelemetryPkt,
                   SU_APP_HK_TLM_MID,
                   SU_APP_HK_TLM_LNGTH, TRUE);

    CFE_EVS_SendEvent (SU_STARTUP_INF_EID, CFE_EVS_INFORMATION,
               "SU App Initialized. Version %d.%d.%d.%d",
                SU_APP_MAJOR_VERSION,
                SU_APP_MINOR_VERSION, 
                SU_APP_REVISION, 
                SU_APP_MISSION_REV);
				
} /* End of SU_AppInit() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  SU_ProcessCommandPacket                                        */
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the SU    */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void SU_ProcessCommandPacket(void)
{
    CFE_SB_MsgId_t  MsgId;

    MsgId = CFE_SB_GetMsgId(SUMsgPtr);

    switch (MsgId)
    {
        case SU_APP_CMD_MID:
            SU_ProcessGroundCommand();
            break;

        case SU_APP_SEND_HK_MID:
            SU_ReportHousekeeping();
            break;

        default:
            SU_HkTelemetryPkt.su_command_error_count++;
            CFE_EVS_SendEvent(SU_COMMAND_ERR_EID,CFE_EVS_ERROR,
			"SU: invalid command packet,MID = 0x%x", MsgId);
            break;
    }

    return;

} /* End SU_ProcessCommandPacket */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* SU_ProcessGroundCommand() -- SU ground commands                    */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

void SU_ProcessGroundCommand(void)
{
    uint16 CommandCode;

    CommandCode = CFE_SB_GetCmdCode(SUMsgPtr);

    /* Process "known" SU app ground commands */
    switch (CommandCode)
    {
        case SU_APP_NOOP_CC:
            SU_HkTelemetryPkt.su_command_count++;
            CFE_EVS_SendEvent(SU_COMMANDNOP_INF_EID,CFE_EVS_INFORMATION,
			"SU: NOOP command");
            break;

        case SU_APP_RESET_COUNTERS_CC:
            SU_ResetCounters();
            break;

        /* default case already found during FC vs length test */
        default:
            break;
    }
    return;

} /* End of SU_ProcessGroundCommand() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  SU_ReportHousekeeping                                              */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void SU_ReportHousekeeping(void)
{
    CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &SU_HkTelemetryPkt);
    CFE_SB_SendMsg((CFE_SB_Msg_t *) &SU_HkTelemetryPkt);
    return;

} /* End of SU_ReportHousekeeping() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  SU_ResetCounters                                               */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void SU_ResetCounters(void)
{
    /* Status of commands processed by the SU App */
    SU_HkTelemetryPkt.su_command_count       = 0;
    SU_HkTelemetryPkt.su_command_error_count = 0;

    CFE_EVS_SendEvent(SU_COMMANDRST_INF_EID, CFE_EVS_INFORMATION,
		"SU: RESET command");
    return;

} /* End of SU_ResetCounters() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* SU_VerifyCmdLength() -- Verify command packet length                   */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
boolean SU_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength)
{     
    boolean result = TRUE;

    uint16 ActualLength = CFE_SB_GetTotalMsgLength(msg);

    /*
    ** Verify the command packet length.
    */
    if (ExpectedLength != ActualLength)
    {
        CFE_SB_MsgId_t MessageID   = CFE_SB_GetMsgId(msg);
        uint16         CommandCode = CFE_SB_GetCmdCode(msg);

        CFE_EVS_SendEvent(SU_LEN_ERR_EID, CFE_EVS_ERROR,
           "Invalid msg length: ID = 0x%X,  CC = %d, Len = %d, Expected = %d",
              MessageID, CommandCode, ActualLength, ExpectedLength);
        result = FALSE;
        SU_HkTelemetryPkt.su_command_error_count++;
    }

    return(result);

} /* End of SU_VerifyCmdLength() */

