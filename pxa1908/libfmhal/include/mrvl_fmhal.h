#ifndef __INCLUDED_FMHAL_H__
#define __INCLUDED_FMHAL_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum FM_EVENTS
{
    RDS_EVENT = 0x04,
};

typedef int(*event_callback)(void*, void*);

int FMSequence_Enable(int freq, event_callback rdslistener, event_callback aflistener, event_callback exeptionlistener);
int FMSequence_Disable();

int FMSequence_SetSoftMute(uint8_t attenuation, uint8_t threshold, int mute);

int FMSequence_SetChannel(uint32_t freq, uint32_t *freq_set);
int FMSequence_GetChannel(uint32_t *freq);

int FMSequence_EnableAf(int enable);
int FMSequence_EnableRds(int enable);

int FMSequence_StopScan(int stop, int *seeking);
int FMSequence_GetNext(uint32_t *freq);
int FMSequence_GetPrev(uint32_t *freq);

int FMSequence_SetBand(uint32_t low_freq, uint32_t high_freq);
int FMSequence_SetChannelStepSize(int spacing);
int FMSequence_SetDemphasis(uint32_t demphasis);

int FMSequence_SetMonoAudioMode(int is_mono);

int FMSequence_SetVolume(int vol);

/*
Name	Address	Ordinal
FMCmdDisableI2SClocks	0000C6A0	
FMCmdGetAudioPath	0000C414	
FMCmdGetAudioVolume	0000BF70	
FMCmdGetChannel	0000C2D0	
FMCmdGetCurrentFlags	0000BCEC	
FMCmdGetCurrentRssi	0000BE8C	
FMCmdGetEventMask	0000BD7C	
FMCmdGetForceMono	0000BBBC	
FMCmdGetMuteMode	0000C378	
FMCmdGetRSSIPeakThreshold	0000BC50	
FMCmdGetRdsData	0000C448	
FMCmdGetRdsSyncStatus	0000BCB8	
FMCmdGetSearchDirection	0000BB5C	
FMCmdGetStereoBlendingOffset	0000C008	
FMCmdGetStereoStatus	0000C03C	
FMCmdInitialize	0000BA10	
FMCmdPowerDown	0000C7DC	
FMCmdRdsDataFlush	0000C470	
FMCmdRdsGetBlockB	0000BDF8	
FMCmdRdsGetBlockBMask	0000BE58	
FMCmdRdsGetDropEBlocks	0000C604	
FMCmdRdsGetMemDepth	0000C4C4	
FMCmdRdsGetPi	0000C52C	
FMCmdRdsGetPiMask	0000C59C	
FMCmdRdsGetStats	0000C770	
FMCmdRdsResetStats	0000C178	
FMCmdRdsSetBlockB	0000BDCC	
FMCmdRdsSetBlockBMask	0000BE2C	
FMCmdRdsSetDropEBlocks	0000C5D8	
FMCmdRdsSetMemDepth	0000C498	
FMCmdRdsSetPi	0000C4F8	
FMCmdRdsSetPiMask	0000C568	
FMCmdRegRdWr	0000C1B8	
FMCmdReset	0000B9E8	
FMCmdSetAFC	0000C0F8	
FMCmdSetAfChannel	0000BAC0	
FMCmdSetAudioPath	0000C3E0	
FMCmdSetAudioSamplingRate	0000C640	
FMCmdSetAudioSamplingRateShift	0000C670	
FMCmdSetAudioVolume	0000BF3C	
FMCmdSetAutoSearchMode	0000BB90	
FMCmdSetBand	0000BC8C	
FMCmdSetChannel	0000BA50	
FMCmdSetChannelStepSize	0000BEC8	
FMCmdSetDemphasis	0000C320	
FMCmdSetEventMask	0000BD3C	
FMCmdSetForceMono	0000BBF0	
FMCmdSetLnaConfig	0000BFAC	
FMCmdSetLoCalTime	0000C6CC	
FMCmdSetMPX	0000C13C	
FMCmdSetMode	0000C2A4	
FMCmdSetMuteMode	0000C34C	
FMCmdSetRSSIPeakThreshold	0000BC1C	
FMCmdSetRSSIThreshold	0000BF08	
FMCmdSetRefClkErr	0000C738	
FMCmdSetSearchDirection	0000BB30	
FMCmdSetSkippedTones	0000C0A8	
FMCmdSetSoftMute	0000C3AC	
FMCmdSetStereoBlendingOffset	0000BFDC	
FMCmdSetVolumeRamps	0000C070	
FMCmdSocSetPinMux	0000C260	
FMCmdStopScanSearch	0000C704	
FMException_Notify	00009308	
FMException_RegisterListener	000092E4	
FMException_startRecoveryThread	0000935C	
FMException_stopRecoeryThread	000093F4	
FMExtCmd_setRegion	0000C224	
FMExtCmd_setScanSearchParams	0000C23C	
FMHALDefineConfiguration	0000ADB8	
FMHALInitialize	00004ADC	
FMHALInitializeExtended	00004A14	
FMHALLoadConfiguration	0000B0FC	
FMHALStop	00004AEC	
FMIODefaultRxCallback	0000967C	
FMIOGetRdThreadStatus	00009A38	
FMIOSendCommand	00009C30	
FMIOStartRdThread	00009A44	
FMIOStopRdThread	00009B84	
FMIOWriteBytes	00009BEC	
FMInternal_AfChannelBurst	0000A3A8	
FMInternal_AfEnable	0000A310	
FMInternal_AfJump	0000CAE8	
FMInternal_AfMain	0000A4EC	
FMInternal_AfRegisterListener	0000A2A8	
FMInternal_AfSendCommand	0000A344	
FMInternal_AudioMute	0000C8B8	
FMInternal_DisableEvents	0000A93C	
FMInternal_EnableEvents	0000A8FC	
FMInternal_GetBClkFrequency	0000A138	
FMInternal_GetDigitalBusCorrection	0000A1D8	
FMInternal_InitFMSequence	0000A780	
FMInternal_InitMuteState	0000A814	
FMInternal_Mute	0000A8F4	
FMInternal_MuteForceCmd	0000A82C	
FMInternal_NotifyAfNotificationListener	0000A2BC	
FMInternal_RdsEnable	00009E28	
FMInternal_RdsMain	00009EB0	
FMInternal_RdsSendCommmand	00009E5C	
FMInternal_ResetSynchronousScan	0000A9C0	
FMInternal_ScanSearch	0000C93C	
FMInternal_ScanSearchSynchronous	0000AA48	
FMInternal_SetEventConfiguration	0000AB68	
FMInternal_SetNotificationLevel	0000AC30	
FMInternal_StartAfRdsThreads	0000AC74	
FMInternal_StopAfRdsThreads	0000AD20	
FMInternal_SynchronousScanMain	0000A73C	
FMInternal_convertLinearTodB	0000C860	
FMInternal_convertdBToLinear	0000C808	
FMInternal_forceStopSynchronousScan	0000A97C	
FMRdsDcoder_AddChannelInAf	00008BB4	
FMRdsDcoder_CleanAfList	00008B30	
FMRdsDecoder_AfDecoding	00008CD0	
FMRdsDecoder_ConvertAFCodeToChannel	00008B14	
FMRdsDecoder_Decoder	00009038	
FMRdsDecoder_DecodingGroup0	00008E90	
FMRdsDecoder_DecodingGroup2	00008EC4	
FMRdsDecoder_Initialize	00008A60	
FMRdsDecoder_NotifyNewChannel	00008B20	
FMRdsDecoder_PsDecoding	00008DE8	
FMRdsDecoder_SetPiCode	00008D58	
FMSendCommandAndReturnResult	00004D6C	
FMSendCommandWithOGFOCF	00004CBC	
FMSendLockedSingleCommand	00004B50	
FMSequence_AfJump	0000E0AC	
FMSequence_Disable	0000DC78	
FMSequence_DisableforRecovery	0000D504	
FMSequence_Enable	0000D648	
FMSequence_EnableAf	0000D39C	
FMSequence_EnableRds	0000D354	
FMSequence_ExceptionListener	0000DBA8	
FMSequence_GetBand	0000CDB8	
FMSequence_GetChannel	0000CD48	
FMSequence_GetDigitalAudioCorrection	0000D288	
FMSequence_GetNext	0000DF8C	
FMSequence_GetPrev	0000E01C	
FMSequence_GetRSSIPeakThreshold	0000D0FC	
FMSequence_GetRSSIThreshold	0000D0B0	
FMSequence_GetSignalQuality	0000CFDC	
FMSequence_GetTuningMetrics	0000D484	
FMSequence_GetVolume	0000D1CC	
FMSequence_Mute	0000CCF8	
FMSequence_Recovery	0000DB24	
FMSequence_ScanSearch	0000DF30	
FMSequence_ScanSearchSynchronous	0000DED4	
FMSequence_SetAFThreshold	0000D3FC	
FMSequence_SetAFValidThreshold	0000D440	
FMSequence_SetBand	0000CE10	
FMSequence_SetChannel	0000DD28	
FMSequence_SetChannelStepSize	0000CF1C	
FMSequence_SetDemphasis	0000CF7C	
FMSequence_SetMonoAudioMode	0000CEB8	
FMSequence_SetNotificationLevel	0000D2B8	
FMSequence_SetRSSIPeakThreshold	0000D0CC	
FMSequence_SetRSSIThreshold	0000D064	
FMSequence_SetSoftMute	0000D24C	
FMSequence_SetVolume	0000D150	
FMSequence_StopScan	0000D304	
FMSequence_change_MIPI_Clock	0000D584	
_mssCfgGetSetEntryNextValue	000066B8	
afNotificationListener	00016A60	
check_driver_loaded	00009454	
fmAfStatus	00016A48	
fmCondVarRunningHCICmd	00017C0C	
fmErrStr	00009C94	
fmGet_Command_and_Event_name	00004F34	
fmHALConfiguration	000170F8	
fmHciCmdCallback	000160C4	
fmHciCmdResLength	00017C1C	
fmHciCmdResult	00017B0C	
fmLog	00004F08	
fmLogCfg	00004F22	
fmLogHci	00005438	
fmLogHciOn	00016008	
fmLogHciStream	000160C8	
fmLogLevel	00016004	
fmLogLevelConfig	00004D98	
fmLogShutdown	00004DB8	
fmLogStream	000160CC	
fmLogStreamConfig	00004E08	
fmLogStreamConfigWithFileNo	00004E34	
fmLogSystemLogEnable	00016009	
fmMutexAttr	00017C10	
fmMutexRunningHCICmd	00017C14	
fmPredicateRunningHCICmd	00017C1D	
fmRdsAfData	00017A04	
fmRdsStatus	00016A34	
fmSearchStatus	00016A64	
fmSequencesState	00016080	
fmSetTxt_Hci_Log	000052F4	
fmSignalQualityCfg	00016A2C	
fmSingleCmdMutex	00017C18	
fmUtilsGetBarNr	00009D64	
fmUtilsGetDbmfromReal	00009DB0	
fmUtilsGetRealfromDbm	00009DE8	
fmVLog	00004E7C	
fm_bandLimits	00014424	
fm_band_regions	000160B0	
fm_event_decoder	0000B7E4	
fm_register_event_listener	0000B7D8	
mssCfgAddEntry	00006308	
mssCfgAddSection	000061FC	
mssCfgDump	00006754	
mssCfgEntryTypeStr	00006D14	
mssCfgFinalize	00006C80	
mssCfgGetBool	000076D8	
mssCfgGetEntry	00006538	
mssCfgGetEntryNValues	000065F8	
mssCfgGetEntryNextValue	00006748	
mssCfgGetInt16	00007130	
mssCfgGetInt32	00007308	
mssCfgGetInt64	000074E8	
mssCfgGetInt8	00006F58	
mssCfgGetSection	000064EC	
mssCfgGetSectionNValues	000065B0	
mssCfgGetSectionNextValue	00006628	
mssCfgGetStr	000078AC	
mssCfgGetUInt16	00007040	
mssCfgGetUInt32	00007218	
mssCfgGetUInt64	000073F4	
mssCfgGetUInt8	00006E6C	
mssCfgInitialize	00006C4C	
mssCfgParse	00007BAC	
mssCfgSetBool	00008560	
mssCfgSetEntryNextValue	0000674E	
mssCfgSetInt16	00008198	
mssCfgSetInt32	0000831C	
mssCfgSetInt64	00008498	
mssCfgSetInt8	00008014	
mssCfgSetLogSink	000061F0	
mssCfgSetLogStream	000061D4	
mssCfgSetSectionNextValue	00006670	
mssCfgSetStr	00008628	
mssCfgSetUInt16	000080D0	
mssCfgSetUInt32	00008254	
mssCfgSetUInt64	000083D8	
mssCfgSetUInt8	00007F48	
*/

#ifdef __cplusplus
}
#endif

#endif // __INCLUDED_FMHAL_H__
