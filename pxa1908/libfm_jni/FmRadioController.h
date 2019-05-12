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

#ifndef __FM_RADIO_CTRL_H__
#define __FM_RADIO_CTRL_H__

#include <pthread.h>
#include <ctime>
#include <string>

class FmRadioController
{
    private:
        char af_enabled;
        int cur_fm_state;
        long int prev_freq;
        bool seek_scan_canceled;
        bool is_seeking;
        bool is_rds_support;
        bool is_ps_event_received = false;
        bool is_rt_event_received = false;
        bool is_af_jump_received = false;
        bool event_listener_canceled;
        pthread_mutex_t mutex_fm_state;
        char rds_enabled;
        int fd_driver;
        pthread_t event_listener_thread;
        bool processing_rds;

        std::string radio_name;
        std::string radio_text;

        bool GetSoftMute(void);
        void ClearRDSData();
        void set_fm_state(int state);
        int get_fm_state(void);
        int SetRdsGrpMask(int mask);
        int SetRdsGrpProcessing(int grps);
        struct timespec set_time_out(int secs);
        int GetStationList(uint16_t *scan_tbl, int *max_cnt);

        int EnableRDS(void);
        int DisableRDS(void);

        int EnableAF(void);
        int DisableAF(void);

        int SetStereo(void);
        int SetMono(void);

        int MuteOn(void);
        int MuteOff(void);

        long GetCurrentRSSI(void);
        void LoadSoftMuteControl();

    public:
        FmRadioController();
        ~FmRadioController();
        int Pwr_Up(int freq);
        int Pwr_Down(void);
        long GetChannel(void);
        int TuneChannel(long);
        int SetSoftMute(bool mode);
        int Seek(int dir);
        int open_dev(void);
        int close_dev();
        bool IsRds_support();
        int ScanList(uint16_t *scan_tbl, int *max_cnt);
        int ReadRDS(void);
        int Get_ps(char *ps, int *ps_len);
        int Get_rt(char *rt, int *rt_len);
        int Get_AF_freq(uint16_t *ret_freq);
        int SetDeConstant(long );
        int Set_mute(bool mute);
        int SetBand(long);
        int SetChannelSpacing(long);
        int Stop_Scan_Seek(void);
        int Turn_On_Off_Rds(bool onoff);
        int Antenna_Switch(int antenna);
};

#endif
