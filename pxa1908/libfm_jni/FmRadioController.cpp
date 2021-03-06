/*
Copyright (c) 2015, The Linux Foundation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#define LOG_TAG "android_hardware_fm"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <utils/Log.h>
#include <cutils/properties.h>
#include <sys/ioctl.h>
#include <math.h>

#include "FM_Const.h"
#include "FmRadioController.h"

#include <errno.h>
#include <mrvl_fmhal.h>

static int  attenuation = 2;
static int  mute_thresh = -75; //db
static bool is_softmuted = false;
static bool debug_fm = false;

static int state_band = DEFAULT_BAND;

#define LOW_BAND  frequencies_band[state_band][BAND_LOW]
#define HIGH_BAND frequencies_band[state_band][BAND_HIGH]

#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif // MIN

#ifndef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif // MAX

enum RDS_TYPE
{
    PSDATA = 0x01, // the radio name
    RTDATA = 0x02, // the radio text
    AFDATA = 0x04, // AF Jump event
};

static struct
{
    int8_t type;
    char ps[MAX_PS_LEN+1];
    char rt[MAX_RT_LEN+1];
    int32_t channel;
} rdsData;

int RemoveBlank(char *str, int len)
{
    len = strnlen(str, len); // Get the string length, but the max is len
    while( len )
    {
        // Strip string
        if( str[len-1] == ' ' || str[len-1] == '\0' )
            str[--len] = '\0';
        else
            break;
    }
    return len;
}

pthread_mutex_t using_rdsData = PTHREAD_MUTEX_INITIALIZER;

void FMRadio_SetRDSData(fmRDSEvent_t* data)
{
    int name_len;

    if( data )
    {
        pthread_mutex_lock(&using_rdsData);

        if( data->psData && data->serviceName[0] )
        {
            name_len = RemoveBlank(data->serviceName, MAX_PS_LEN);
            if( name_len )
            {
                memset(rdsData.ps, 0, sizeof(rdsData.ps));
                rdsData.type |= PSDATA;
                strncpy(rdsData.ps, data->serviceName, name_len);
            }
        }
        if( data->rtData && data->radioText[0] )
        {
            name_len = RemoveBlank(data->radioText, MAX_RT_LEN);
            if( name_len )
            {
                memset(rdsData.rt, 0, sizeof(rdsData.rt));
                rdsData.type |= RTDATA;
                strncpy(rdsData.rt, data->radioText, name_len);
            }
        }

        pthread_mutex_unlock(&using_rdsData);
    }
}

int rdslistener_callback(fmEvent_t const *event, void *_param)
{
    int result;

    if ( event )
    {
        if ( event->rdsEvent.fmEvent == RDS_EVENT )
        {
            FMRadio_SetRDSData(const_cast<fmRDSEvent_t*>(&event->rdsEvent));
        }
        else
        {
            ALOGI("rdslistener_callback() : received an unknown event (%d)!\n", event->rdsEvent.fmEvent);
        }
        result = FM_SUCCESS;
    }
    else
    {
        ALOGI("rdslistener_callback() : RDS data is NULL\n");
        result = FM_FAILURE;
    }
    return result;
}

void aflistener_callback(const fmAfNotification_t * notif)
{
    ALOGD("AF notification : state = %d, channel = %d, success = %d", notif->state, notif->channel, (int)notif->success);
    if( notif->state == 1 && notif->success )
    {
        pthread_mutex_lock(&using_rdsData);

        rdsData.channel = notif->channel;
        rdsData.type |= AFDATA;

        pthread_mutex_unlock(&using_rdsData);
    }
}

int exceptionlistener_callback(void* _data)
{
    ALOGD("FM %s in", __func__);
    return FM_SUCCESS;
}

//Reset all variables to default value
//static FmIoctlsInterface * FmIoct;

FmRadioController::FmRadioController():
    radio_name(MAX_PS_LEN+1, 0),
    radio_text(MAX_RT_LEN+1, 0)
{
    cur_fm_state = FM_OFF;
    prev_freq = -1;
    seek_scan_canceled = false;
    af_enabled = 0;
    rds_enabled = 0;
    is_rds_support = true;
    is_af_jump_received = false;
    mutex_fm_state = PTHREAD_MUTEX_INITIALIZER;
    mutex_ps_rt = PTHREAD_MUTEX_INITIALIZER;
    processing_rds = false;

    LoadSoftMuteControl();
}

/* Turn off FM */
FmRadioController::~FmRadioController()
{
    Pwr_Down();
}

void FmRadioController::LoadSoftMuteControl()
{
    char buff[PROPERTY_VALUE_MAX];
    char default_val[PROPERTY_VALUE_MAX];

    sprintf(default_val, "%d", -mute_thresh);
    property_get("service.mrvl.fm.mute_thresh", buff, default_val);
    mute_thresh = -atoi(buff);
    sprintf(default_val, "%d", attenuation);
    property_get("service.mrvl.fm.attenuation", buff, default_val);
    attenuation = atoi(buff);
    ALOGI("%s mute=%d, attenuation=%d.", "LoadSoftMuteControl",
           mute_thresh, attenuation);

}

int FmRadioController::Pwr_Up(int freq)
{
    if( cur_fm_state == FM_ON || cur_fm_state == FM_TUNE_IN_PROGRESS || cur_fm_state == FM_SCAN_IN_PROGRESS )
        return FM_SUCCESS;

    if( cur_fm_state == FM_OFF_IN_PROGRESS || cur_fm_state == FM_ON_IN_PROGRESS )
        return FM_FAILURE;

    set_fm_state(FM_ON_IN_PROGRESS);
    int res = FMSequence_Enable(freq, rdslistener_callback, aflistener_callback, exceptionlistener_callback);
    if( res )
    {
        set_fm_state(FM_OFF);
        ALOGE("%s: FM Enable Failed (error code = %d)", __func__, res);
        return FM_FAILURE;
    }
    set_fm_state(FM_ON);
    prev_freq = freq;

    ClearRDSData();

    SetBand(DEFAULT_BAND);
    SetChannelSpacing(SS_CHAN_SPACE_100);

    SetDeConstant(1);

    SetStereo();
    FMSequence_SetVolume(2);

    char buff[PROPERTY_VALUE_MAX];
    property_get("ro.product.model", buff, nullptr);
    if( !strncmp(buff, "SM-G361F", 8) )
    {
        SetSoftMute(true);
    }

    return FM_SUCCESS;
}

int FmRadioController::Pwr_Down()
{
    if( cur_fm_state == FM_OFF || cur_fm_state == FM_OFF_IN_PROGRESS )
        return FM_SUCCESS;

    set_fm_state(FM_OFF_IN_PROGRESS);
    int res = FMSequence_Disable();
    if( res )
    {
        ALOGE("%s: FM Disable Failed (error code = %d)", __func__, res);
        set_fm_state(FM_ON);
        return FM_FAILURE;
    }

    set_fm_state(FM_OFF);
    return FM_SUCCESS;
}

void FmRadioController::ClearRDSData()
{
    /*
    radio_name.clear();
    radio_text.clear();
    is_rt_event_received = false;
    is_ps_event_received = false;
    */
}

int FmRadioController::SetSoftMute(bool mode)
{
    ALOGI("%s: softmute = %d, attenuation=%d, mute_thresh=%d", __func__, mode, attenuation, mute_thresh);
    is_softmuted = mode;

    property_set("service.mrvl.fm.enable_swmute", mode ? "1" : "0");

    return FMSequence_SetSoftMute(attenuation, mute_thresh, mode);
}

bool FmRadioController :: GetSoftMute()
{
    return is_softmuted;
}

//Get current tuned frequency
//Return -1 if failed to get freq
long FmRadioController :: GetChannel(void)
{
    uint32_t freq = FM_FAILURE;

    if( cur_fm_state != FM_ON )
        return FM_FAILURE;

    if( FMSequence_GetChannel(&freq) )
    {
        ALOGI("current channel read failed");
    }
    if( debug_fm )
        ALOGI("Current Channel is %d", freq);
    else
        ALOGI("GetChannel: end");

    return freq;
}
//Tune to a Freq
//Return FM_SUCCESS on success FM_FAILURE
//on failure
int FmRadioController::TuneChannel(long freq)
{
    if( cur_fm_state != FM_ON )
        return FM_FAILURE;

    uint32_t freq_set;
    set_fm_state(FM_TUNE_IN_PROGRESS);
    int res = FMSequence_SetChannel(freq, &freq_set);
    set_fm_state(FM_ON);

    if( res )
    {

        ALOGE("%s: FM TuneChannel failed", __func__);
        return FM_FAILURE;
    }


    prev_freq = freq_set;
    ClearRDSData();
    seek_scan_canceled = false;

    return FM_SUCCESS;
}

int FmRadioController :: get_fm_state()
{
    return cur_fm_state;
}

void FmRadioController :: set_fm_state(int state)
{
    pthread_mutex_lock(&mutex_fm_state);
    cur_fm_state = state;
    pthread_mutex_unlock(&mutex_fm_state);
}

int FmRadioController::Seek(int dir)
{
    uint32_t new_freq;
    int res;

    if( cur_fm_state != FM_ON )
        return FM_FAILURE;

    set_fm_state(FM_SEEK_IN_PROGRESS);
    res = (dir ? FMSequence_GetNext(&new_freq) : FMSequence_GetPrev(&new_freq) );
    set_fm_state(FM_ON);

    ClearRDSData();
    if( seek_scan_canceled )
    {
        if( debug_fm )
            ALOGI("Seek is canceled. %d", new_freq);
        else
            ALOGI("Seek is canceled.");
        seek_scan_canceled = false;
    }
    else if( res || new_freq == 0 )
    {
        ALOGI("Seek failed (errorCode = %d)", res);
        FMSequence_SetChannel(dir ? HIGH_BAND : LOW_BAND, &new_freq);
        if( seek_scan_canceled )
        {
            ALOGI("SetChannel process cancel.");
            seek_scan_canceled = false;
        }
    }

    if(new_freq >= LOW_BAND && new_freq <= HIGH_BAND)
        prev_freq = new_freq;
    else
    {
        ALOGI("Seek channel: out of band (%d: %d - %d).", new_freq, LOW_BAND, HIGH_BAND);
        new_freq = prev_freq;
    }

    if( debug_fm )
        ALOGI("Seek channel is %d", new_freq);
    else
        ALOGI("Seek channel: end");

    return new_freq;
}

int FmRadioController :: Stop_Scan_Seek()
{
    seek_scan_canceled = true;
    if( cur_fm_state == FM_SEEK_IN_PROGRESS ||
        cur_fm_state == FM_SCAN_IN_PROGRESS )
    {
        int seeking;
        FMSequence_StopScan(1, &seeking);
        for( int i = 0; i < 100; ++i )
        {
            usleep(10000);
            if( !seek_scan_canceled )
                break;
        }
    }

    return FM_SUCCESS;
}

//HardMute both audio channels
int FmRadioController::MuteOn()
{
    return FM_SUCCESS;
}

//Unmute both audio channel
int FmRadioController ::MuteOff()
{
    return FM_SUCCESS;
}

int FmRadioController :: Set_mute(bool mute)
{
    return (mute ? MuteOn() : MuteOff());
}

//Set regional band
int FmRadioController :: SetBand(long band)
{
    if( cur_fm_state != FM_ON )
        return FM_FAILURE;
    if( band > MAX_STATE_BAND || band < 0 ) band = DEFAULT_BAND;
    return FMSequence_SetBand(frequencies_band[band][BAND_LOW], frequencies_band[band][BAND_HIGH]);
}

//set spacing for successive channels
int FmRadioController :: SetChannelSpacing(long spacing)
{
    if( cur_fm_state != FM_ON )
        return FM_FAILURE;
    return FMSequence_SetChannelStepSize(spacing);
}

//Enable RDS data receiving
//Enable RT, PS, AF Jump, RTPLUS, ERT etc
int FmRadioController :: EnableRDS(void)
{
    if( cur_fm_state != FM_ON )
        return FM_FAILURE;

    int res = FMSequence_EnableRds(1);
    if(res) return res;
    rds_enabled = true;
    return EnableAF();
}

//Disable all RDS data processing
//RT, ERT, RT PLUS, PS
int FmRadioController :: DisableRDS(void)
{
    if( cur_fm_state != FM_ON )
        return FM_FAILURE;

    int res = FMSequence_EnableRds(0);
    if(res) return res;
    rds_enabled = false;
    return DisableAF();
}

int FmRadioController :: Turn_On_Off_Rds(bool onoff)
{
    return (onoff ? EnableRDS() : DisableRDS());
}

//Enables Alternate Frequency switching
int FmRadioController :: EnableAF(void)
{
    int res = FMSequence_EnableAf(1);
    if( !res ) af_enabled = true;
    return res;
}

//Disables Alternate Frequency switching
int FmRadioController :: DisableAF(void)
{
    int res = FMSequence_EnableAf(0);
    if( !res ) af_enabled = false;
    return res;
}

bool FmRadioController::IsRds_support(void)
{
    return true;
}

int FmRadioController::open_dev()
{
    return FM_SUCCESS;
}

int FmRadioController::close_dev()
{
    return FM_SUCCESS;
}

int FmRadioController :: Get_AF_freq(uint16_t *ret_freq)
{
    int channel = GetChannel();
    if( channel >= frequencies_band[DEFAULT_BAND][BAND_LOW]
    && channel <= frequencies_band[DEFAULT_BAND][BAND_HIGH])
    {
        *ret_freq = channel;
        return FM_SUCCESS;
    }

    return FM_FAILURE;
}

int FmRadioController::ReadRDS()
{
    int ret = 0;
    if( cur_fm_state != FM_OFF && cur_fm_state != FM_OFF_IN_PROGRESS )
    {
        // If we are not processing rds
        if( !processing_rds )
        {
            processing_rds = true;
            pthread_mutex_lock(&using_rdsData);

            if( rdsData.type & PSDATA )
            {
                if( radio_name != rdsData.ps )
                {
                    pthread_mutex_lock(&mutex_ps_rt);
                    radio_name = rdsData.ps;
                    pthread_mutex_unlock(&mutex_ps_rt);
                    ret |= RDS_EVT_PS_UPDATE;
                    is_ps_event_received = true;
                }
            }
            if( rdsData.type & RTDATA )
            {
                if( radio_text != rdsData.rt )
                {
                    pthread_mutex_lock(&mutex_ps_rt);
                    radio_text = rdsData.rt;
                    pthread_mutex_unlock(&mutex_ps_rt);
                    ret |= RDS_EVT_RT_UPDATE;
                    is_rt_event_received = true;
                }
            }
            if( rdsData.type & AFDATA )
            {
                if( prev_freq != rdsData.channel )
                {
	            prev_freq = rdsData.channel;
                    ret |= RDS_EVT_AF_JUMP;
		    is_af_jump_received = true;
                }
            }

            pthread_mutex_unlock(&using_rdsData);
            processing_rds = false;
        }
    }

    return ret;
}

int FmRadioController :: Get_ps(char *ps, int *ps_len)
{
    if( !is_ps_event_received ) return FM_FAILURE;

    pthread_mutex_lock(&mutex_ps_rt);
    strncpy(ps, radio_name.c_str(), MAX_PS_LEN);
    *ps_len = MIN(MAX_PS_LEN, radio_name.length());
    is_ps_event_received = false;
    pthread_mutex_unlock(&mutex_ps_rt);

    ps[*ps_len] = '\0';

    return FM_SUCCESS;
}

int FmRadioController :: Get_rt(char *rt, int *rt_len)
{
    if( !is_rt_event_received ) return FM_FAILURE;

    pthread_mutex_lock(&mutex_ps_rt);
    strncpy(rt, radio_text.c_str(), MAX_RT_LEN);
    *rt_len = MIN(MAX_RT_LEN, radio_text.length());
    is_rt_event_received = false;
    pthread_mutex_unlock(&mutex_ps_rt);

    rt[*rt_len] = 0;

    return FM_SUCCESS;
}

int FmRadioController :: SetStereo()
{
    if( cur_fm_state == FM_ON )
        return FMSequence_SetMonoAudioMode(0);
    else
        return FM_FAILURE;
}

int FmRadioController :: SetMono()
{
    if( cur_fm_state == FM_ON )
        return FMSequence_SetMonoAudioMode(1);
    else
        return FM_FAILURE;
}

//Emphasis:
//75microsec: 0, 50 microsec: 1
//return FM_SUCCESS on success, FM_FAILURE
//on failure
int FmRadioController::SetDeConstant(long emphasis)
{
    if( cur_fm_state == FM_ON )
        return FMSequence_SetDemphasis(emphasis);
    else
        return FM_FAILURE;
}

int FmRadioController::ScanList(uint16_t *scan_tbl, int *max_cnt)
{
    int res = FM_FAILURE;

    if( cur_fm_state == FM_ON )
    {
        int freq;
        uint16_t strength;
        int cnt = 0;
        set_fm_state(FM_SCAN_IN_PROGRESS);

        for( int i = 0; i < *max_cnt; ++i )
        {
            if( seek_scan_canceled )
            {
                seek_scan_canceled = false;
                break;
            }

            res = FMSequence_ScanSearchSynchronous(&freq, &strength);

            if( res != FM_SUCCESS )
                break;

            // No more station
            if( !freq )
            {
                res = FM_SUCCESS;
                break;
            }

            if( freq >= LOW_BAND && freq <= HIGH_BAND )
            {
                ALOGI("FM Found freq %d", freq);
                scan_tbl[cnt++] = freq/SRCH_DIV;
            }
        }
        *max_cnt = cnt;
        set_fm_state(FM_ON);
    }
    else
    {
        *max_cnt = 0;
        *scan_tbl = 0;
    }

    return res;
}

int FmRadioController::Antenna_Switch(int antenna)
{
    return FM_SUCCESS;
}
