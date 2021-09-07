#include <lq-types.h>
#include <lq-diagnostics.h>
#include <stdio.h>

diagnosticControl_t g_diagControl  __attribute__ ((section (".noinit")));


uint8_t S_calcNotifyCbChk(uint32_t notifCbAddr)
{
    uint8_t result =  notifCbAddr & 0xFF;
    result = result ^ (notifCbAddr >> 4) & 0xFF;
    result = result ^ (notifCbAddr >> 12) & 0xFF;
    result = result ^ (notifCbAddr >> 20) & 0xFF;
    return result;
}


/**
 *  \brief Get diagnostic information from the device.
 * 
 *  \param notifyCallback Function pointer to the application's event notification callback function, which may or may not return.
 */
void lqDiag_registerNotifCallback(appNotifyFunc_t notifyCallback)
{
    //g_diagControl.assertMagic = assert__assertControlMagic;
    g_diagControl.notifyCB = notifyCallback;
    g_diagControl.notifyCbChk = S_calcNotifyCbChk((int32_t)notifyCallback);
}


void lqDiag_setResetCause(uint8_t resetcause)
{
    if (resetcause != diagRcause_watchdog && resetcause != diagRcause_system)           // if non-error reset, clear diagnostics
    {
        uint8_t bfSave = g_diagControl.diagnosticInfo.bootFlag;
        memset(&g_diagControl.diagnosticInfo, 0, sizeof(diagnosticInfo_t));
        g_diagControl.diagnosticInfo.bootFlag = bfSave;
    }
    else
    {
        g_diagControl.diagnosticInfo.bootFlag += g_diagControl.diagnosticInfo.bootFlag == 0xFF ? 0 : 1;
    }
    g_diagControl.diagnosticInfo.rcause = resetcause;
}


void inline lqDiag_setBootSafe()
{
    g_diagControl.diagnosticInfo.bootFlag = 0xFF;
    memset(&g_diagControl.diagnosticInfo, 0, sizeof(diagnosticInfo_t));
}


/**
 *  \brief Get diagnostic information from the device.
 * 
 *  \returns Pointer to the diagnostics information block, or NULL if no valid diagnostics available.
 */
diagnosticInfo_t *lqDiag_getDiagnosticsInfo()
{
    return &g_diagControl.diagnosticInfo;
}


/**
 *  \brief Set application level diagnostic information to report to host.
 */
void lqDiag_setApplicationDiagnosticsInfo(int16_t commState, int16_t ntwkState, int16_t signalState)
{
    g_diagControl.diagnosticInfo.commState = commState;         // indications: TCP/UDP/SSL connected, MQTT state, etc.
    g_diagControl.diagnosticInfo.ntwkState = ntwkState;         // indications: LTE PDP, etc.
    g_diagControl.diagnosticInfo.signalState = signalState;     // indications: rssi, etc.
}


/**
 *  \brief Set application notification information to report to host.
 */
void lqDiag_setApplicationMessage(uint8_t notifCode, const char *notifMsg)
{
    g_diagControl.diagnosticInfo.diagMagic = assert__diagnosticsMagic;
    g_diagControl.diagnosticInfo.notifCode = notifCode;
    memcpy(g_diagControl.diagnosticInfo.notifMsg, notifMsg, 20);
}


/**
 *  \brief Clear diagnostics information block.
 * 
 *  LQCloud uses this after checking diagnostics at system start. LQCloud will use reset cause and diagnostics magic value
 *  to determine if the diagnostics are relevant and sent.
 */
void lqDiag_clearDiagnosticInfo()
{
    memset(&(g_diagControl.diagnosticInfo), 0, sizeof(diagnosticInfo_t));       // clear diagnostics, magic == 0 is no diags available
}

#pragma region ASSERT Functionality
/* --------------------------------------------------------------------------------------------- */

/**
 *  \brief Function to service an application failed ASSERT().
 * 
 *  \param pc
 *  \param lr
 *  \param fileIn [in] The numeric ID of the file containing the triggered ASSERT_W()
 *  \param line [in] The line number of the ASSERT_W() triggering the invoke.
 */
void assert_invoke(void *pc, const void *lr, uint16_t fileId, uint16_t line)
{
    g_diagControl.diagnosticInfo.diagMagic = assert__diagnosticsMagic;
    g_diagControl.diagnosticInfo.pc = pc;
    g_diagControl.diagnosticInfo.lr = lr;
    g_diagControl.diagnosticInfo.line = line;
    g_diagControl.diagnosticInfo.fileId = fileId;

    if (g_diagControl.notifyCB != NULL && g_diagControl.notifyCbChk == S_calcNotifyCbChk(g_diagControl.notifyCB))
    {
        char notifyMsg[40];
        snprintf(notifyMsg, sizeof(notifyMsg), "ASSERT f:%d,l:%d-pc=%d,lr=%d", fileId, line, pc, lr);
        g_diagControl.notifyCB(lqNotifType_assertFailed, notifyMsg);
    }
    assert_brk();                                                               // stop here if notify callback returned
}


/**
 *  \brief Function to service an application failed ASSERT_W().
 * 
 *  Assert warnings are intended to be used during active development to signal back non-optimal
 *  code patterns.
 * 
 *  \param fileIn [in] The numeric ID of the file containing the triggered ASSERT_W()
 *  \param line [in] The line number of the ASSERT_W() triggering the invoke.
 *  \param faultTxt [in] Error message
 */
void assert_warning(uint16_t fileId, uint16_t line, const char *faultTxt)
{
    if (g_diagControl.notifyCB != NULL && g_diagControl.notifyCbChk == S_calcNotifyCbChk(g_diagControl.notifyCB))
    {
        char notifyMsg[80];
        snprintf(notifyMsg, sizeof(notifyMsg), "WARN f:%X,l:%d-%s\r", fileId, line, faultTxt);
        g_diagControl.notifyCB(lqNotifType_assertWarning, notifyMsg);
    }
}


inline void assert_brk()
{
    __asm__("BKPT 9");
}
 #pragma endregion


void lqDiag_setProtoState(int16_t pstate)
{
    g_diagControl.diagnosticInfo.diagMagic = assert__diagnosticsMagic;
    g_diagControl.diagnosticInfo.commState = pstate;
}


void lqDiag_setNtwkState(int16_t pstate)
{
    g_diagControl.diagnosticInfo.diagMagic = assert__diagnosticsMagic;
    g_diagControl.diagnosticInfo.ntwkState = pstate;
}


void lqDiag_setSignalState(int16_t pstate)
{
    g_diagControl.diagnosticInfo.diagMagic = assert__diagnosticsMagic;
    g_diagControl.diagnosticInfo.signalState = pstate;
}
