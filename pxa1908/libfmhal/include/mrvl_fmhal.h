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

int FMSequence_ScanSearch(int *x);
int FMSequence_ScanSearchSynchronous(int *x, uint16_t *y);
int FMSequence_StopScan(int stop, int *seeking);

int FMSequence_GetNext(uint32_t *freq);
int FMSequence_GetPrev(uint32_t *freq);

int FMSequence_SetBand(uint32_t low_freq, uint32_t high_freq);
int FMSequence_SetChannelStepSize(int spacing);
int FMSequence_SetDemphasis(uint32_t demphasis);

int FMSequence_SetMonoAudioMode(int is_mono);

int FMSequence_GetVolume(uint16_t *vol);
int FMSequence_SetVolume(int vol);

/*
FMSequence_AfJump	0000E0AC	
FMSequence_DisableforRecovery	0000D504	
FMSequence_ExceptionListener	0000DBA8	
FMSequence_GetBand	0000CDB8	
FMSequence_GetDigitalAudioCorrection	0000D288	
FMSequence_GetRSSIPeakThreshold	0000D0FC	
FMSequence_GetRSSIThreshold	0000D0B0	
FMSequence_GetSignalQuality	0000CFDC	
FMSequence_GetTuningMetrics	0000D484	
FMSequence_Mute	0000CCF8	
FMSequence_Recovery	0000DB24	
FMSequence_SetAFThreshold	0000D3FC	
FMSequence_SetAFValidThreshold	0000D440	
FMSequence_SetNotificationLevel	0000D2B8	
FMSequence_SetRSSIPeakThreshold	0000D0CC	
FMSequence_SetRSSIThreshold	0000D064	
FMSequence_change_MIPI_Clock	0000D584	
*/

#ifdef __cplusplus
}
#endif

#endif // __INCLUDED_FMHAL_H__
