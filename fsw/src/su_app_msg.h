/*******************************************************************************
** File:
**   su_app_msg.h 
**
** Purpose: 
**  Define SU App  Messages and info
**
** Notes:
**
**
*******************************************************************************/
#ifndef _su_app_msg_h_
#define _su_app_msg_h_

/*
** SU App command codes
*/
#define SU_APP_NOOP_CC                 0
#define SU_APP_RESET_COUNTERS_CC       1

/*************************************************************************/
/*
** Type definition (generic "no arguments" command)
*/
typedef struct
{
   uint8    CmdHeader[CFE_SB_CMD_HDR_SIZE];

} SU_NoArgsCmd_t;

/*************************************************************************/
/*
** Type definition (SU App housekeeping)
*/
typedef struct 
{
    uint8              TlmHeader[CFE_SB_TLM_HDR_SIZE];
    uint8              su_command_error_count;
    uint8              su_command_count;
    uint8              spare[2];

}   OS_PACK su_hk_tlm_t  ;

#define SU_APP_HK_TLM_LNGTH   sizeof ( su_hk_tlm_t )

#endif /* _su_app_msg_h_ */

/************************/
/*  End of File Comment */
/************************/
