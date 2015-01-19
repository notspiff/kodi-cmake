/*
 *      Copyright (C) 2010-2014 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "Application.h"
#include "settings/MediaSettings.h"

#include "cores/AudioEngine/Engines/ActiveAE/ActiveAEBuffer.h"
#include "cores/AudioEngine/AEResampleFactory.h"
#include "cores/AudioEngine/Utils/AEUtil.h"
#include "cores/IPlayer.h"
#include "utils/TimeUtils.h"

#include "ActiveAEDSPProcess.h"
#include "ActiveAEDSPMode.h"

extern "C" {
#include "libavutil/channel_layout.h"
#include "libavutil/opt.h"
}

using namespace std;
using namespace ADDON;
using namespace ActiveAE;

#define MIN_DSP_ARRAY_SIZE 4096

CActiveAEDSPProcess::CActiveAEDSPProcess(AE_DSP_STREAM_ID streamId)
 : m_StreamId(streamId)
{
  m_ChannelLayoutIn         = 0;      /* Undefined input channel layout */
  m_ChannelLayoutOut        = 0;      /* Undefined output channel layout */
  m_StreamTypeUsed          = AE_DSP_ASTREAM_INVALID;
  m_NewStreamType           = AE_DSP_ASTREAM_INVALID;
  m_NewMasterMode           = AE_DSP_MASTER_MODE_ID_INVALID;
  m_ForceInit               = false;
  m_ResamplerDSPProcessor   = NULL;
  m_ConvertInput            = NULL;
  m_ConvertOutput           = NULL;
  m_iLastProcessTime        = 0;

  /*!
   * Create predefined process arrays on every supported channel for audio dsp's.
   * All are set if used or not for safety reason and unsued ones can be used from
   * dsp addons as buffer arrays.
   * If a bigger size is neeeded it becomes reallocated during DSP processing.
   */
  m_ProcessArraySize = MIN_DSP_ARRAY_SIZE;
  for (int i = 0; i < AE_DSP_CH_MAX; i++)
  {
    m_ProcessArray[0][i] = (float*)calloc(m_ProcessArraySize, sizeof(float));
    m_ProcessArray[1][i] = (float*)calloc(m_ProcessArraySize, sizeof(float));
  }
}

CActiveAEDSPProcess::~CActiveAEDSPProcess()
{
  ResetStreamFunctionsSelection();

  if (m_ResamplerDSPProcessor)
  {
    delete m_ResamplerDSPProcessor;
    m_ResamplerDSPProcessor = NULL;
  }

  /* Clear the buffer arrays */
  for (int i = 0; i < AE_DSP_CH_MAX; i++)
  {
    if(m_ProcessArray[0][i])
      free(m_ProcessArray[0][i]);
    if(m_ProcessArray[1][i])
      free(m_ProcessArray[1][i]);
  }

  if (m_ConvertInput)
    swr_free(&m_ConvertInput);
  if (m_ConvertOutput)
    swr_free(&m_ConvertOutput);
}

void CActiveAEDSPProcess::ResetStreamFunctionsSelection()
{
  m_NewMasterMode = AE_DSP_MASTER_MODE_ID_INVALID;
  m_NewStreamType = AE_DSP_ASTREAM_INVALID;
  m_Addon_InputResample.Clear();
  m_Addon_OutputResample.Clear();

  m_Addons_InputProc.clear();
  m_Addons_PreProc.clear();
  m_Addons_MasterProc.clear();
  m_Addons_PostProc.clear();
  m_usedMap.clear();
}

bool CActiveAEDSPProcess::Create(AEAudioFormat inputFormat, AEAudioFormat outputFormat, bool upmix, AEQuality quality, AE_DSP_STREAMTYPE iStreamType)
{
  m_InputFormat       = inputFormat;                        /*!< Input format of processed stream */
  m_OutputFormat      = outputFormat;                       /*!< Output format of required stream (set from ADSP system on startup, to have ffmpeg compatible format */
  m_OutputSamplerate  = m_InputFormat.m_sampleRate;         /*!< If no resampler addon is present output samplerate is the same as input */
  m_OutputFrames      = m_InputFormat.m_frames;
  m_StreamQuality     = quality;                            /*!< from XBMC on settings selected resample quality, also passed to addons to support different quality */
  m_dataFormat        = AE_FMT_FLOAT;                       /*!< the base stream format, hard set to float */
  m_ActiveMode        = AE_DSP_MASTER_MODE_ID_PASSOVER;     /*!< Reset the pointer for m_MasterModes about active master process, set here during mode selection */

  CSingleLock lock(m_restartSection);

  CLog::Log(LOGDEBUG, "ActiveAE DSP - %s - Audio DSP processing id %d created:", __FUNCTION__, m_StreamId);

  m_ConvertInput = swr_alloc_set_opts(m_ConvertInput,
                                      CAEUtil::GetAVChannelLayout(m_InputFormat.m_channelLayout),
                                      AV_SAMPLE_FMT_FLTP,
                                      m_InputFormat.m_sampleRate,
                                      CAEUtil::GetAVChannelLayout(m_InputFormat.m_channelLayout),
                                      CAEUtil::GetAVSampleFormat(m_InputFormat.m_dataFormat),
                                      m_InputFormat.m_sampleRate,
                                      0, NULL);
  if (m_ConvertInput == NULL)
  {
    CLog::Log(LOGERROR, "ActiveAE DSP - %s - DSP input convert with data format '%s' not supported!", __FUNCTION__, CAEUtil::DataFormatToStr(inputFormat.m_dataFormat));
    return false;
  }

  if (swr_init(m_ConvertInput) < 0)
  {
    CLog::Log(LOGERROR, "ActiveAE DSP - %s - DSP input convert failed", __FUNCTION__);
    return false;
  }

  m_ConvertOutput = swr_alloc_set_opts(m_ConvertOutput,
                                       CAEUtil::GetAVChannelLayout(m_OutputFormat.m_channelLayout),
                                       CAEUtil::GetAVSampleFormat(m_OutputFormat.m_dataFormat),
                                       m_OutputFormat.m_sampleRate,
                                       CAEUtil::GetAVChannelLayout(m_OutputFormat.m_channelLayout),
                                       AV_SAMPLE_FMT_FLTP,
                                       m_OutputFormat.m_sampleRate,
                                       0, NULL);
  if (m_ConvertOutput == NULL)
  {
    CLog::Log(LOGERROR, "ActiveAE DSP - %s - DSP output convert with data format '%s' not supported!", __FUNCTION__, CAEUtil::DataFormatToStr(outputFormat.m_dataFormat));
    return false;
  }

  if (swr_init(m_ConvertOutput) < 0)
  {
    CLog::Log(LOGERROR, "ActiveAE DSP - %s - DSP output convert failed", __FUNCTION__);
    return false;
  }

  ResetStreamFunctionsSelection();

  CFileItem currentFile(g_application.CurrentFileItem());

  m_StreamTypeDetected = DetectStreamType(&currentFile);

  if (iStreamType == AE_DSP_ASTREAM_AUTO)
    m_StreamTypeUsed = m_StreamTypeDetected;
  else if (iStreamType >= AE_DSP_ASTREAM_BASIC || iStreamType < AE_DSP_ASTREAM_AUTO)
    m_StreamTypeUsed = iStreamType;
  else
  {
    CLog::Log(LOGERROR, "ActiveAE DSP - %s - Unknown audio stream type, falling back to basic", __FUNCTION__);
    m_StreamTypeUsed = AE_DSP_ASTREAM_BASIC;
  }

  /*!
   * Set general stream information about the processed stream
   */

  if (g_application.m_pPlayer->GetAudioStreamCount() > 0)
  {
    int identifier = CMediaSettings::Get().GetCurrentVideoSettings().m_AudioStream;
    if(identifier < 0)
      identifier = g_application.m_pPlayer->GetAudioStream();
    if (identifier < 0)
      identifier = 0;

    SPlayerAudioStreamInfo info;
    g_application.m_pPlayer->GetAudioStreamInfo(identifier, info);

    m_AddonStreamProperties.strName       = info.name.c_str();
    m_AddonStreamProperties.strLanguage   = info.language.c_str();
    m_AddonStreamProperties.strCodecId    = info.audioCodecName.c_str();
    m_AddonStreamProperties.iIdentifier   = identifier;
    m_AddonStreamProperties.iSampleRate   = info.samplerate;
    m_AddonStreamProperties.iChannels     = info.channels;
  }
  else
  {
    m_AddonStreamProperties.strName       = "Unknown";
    m_AddonStreamProperties.strLanguage   = "";
    m_AddonStreamProperties.strCodecId    = "";
    m_AddonStreamProperties.iIdentifier   = m_StreamId;
    m_AddonStreamProperties.iSampleRate   = m_InputFormat.m_sampleRate;
    m_AddonStreamProperties.iChannels     = m_InputFormat.m_channelLayout.Count();
  }

  m_AddonStreamProperties.iStreamID       = m_StreamId;
  m_AddonStreamProperties.iStreamType     = m_StreamTypeUsed;
  m_AddonStreamProperties.iBaseType       = GetBaseType(&m_AddonStreamProperties);

  /*!
   * Create the profile about additional stream related data, e.g. the different Dolby Digital stream flags
   */
  CreateStreamProfile();

  /*!
   * Set exact input and output format settings
   */
  m_AddonSettings.iStreamID               = m_StreamId;
  m_AddonSettings.iStreamType             = m_StreamTypeUsed;
  m_AddonSettings.lInChannelPresentFlags  = 0;                                    /*!< Reset input channel present flags, becomes set on next steps */
  m_AddonSettings.iInChannels             = m_InputFormat.m_channelLayout.Count();/*!< The from stream given channel amount */
  m_AddonSettings.iInFrames               = m_InputFormat.m_frames;               /*!< Input frames given */
  m_AddonSettings.iInSamplerate           = m_InputFormat.m_sampleRate;           /*!< The basic input samplerate from stream source */
  m_AddonSettings.iProcessFrames          = m_InputFormat.m_frames;               /*!< Default the same as input frames, if input resampler is present it becomes corrected */
  m_AddonSettings.iProcessSamplerate      = m_InputFormat.m_sampleRate;           /*!< Default the same as input samplerate, if input resampler is present it becomes corrected */
  m_AddonSettings.lOutChannelPresentFlags = 0;                                    /*!< Reset output channel present flags, becomes set on next steps */
  m_AddonSettings.iOutChannels            = m_OutputFormat.m_channelLayout.Count(); /*!< The for output required amount of channels */
  m_AddonSettings.iOutFrames              = m_OutputFormat.m_frames;              /*! Output frames requested */
  m_AddonSettings.iOutSamplerate          = m_OutputFormat.m_sampleRate;          /*!< The required sample rate for pass over resampling on ActiveAEResample */
  m_AddonSettings.bStereoUpmix            = upmix;                                /*! Stereo upmix value given from XBMC settings */
  m_AddonSettings.bInputResamplingActive  = false;                                /*! Becomes true if input resampling is in use */
  m_AddonSettings.iQualityLevel           = m_StreamQuality;                      /*! Requested stream processing quality, is optional and can be from addon ignored */

  if (m_InputFormat.m_channelLayout.HasChannel(AE_CH_FL))   m_AddonSettings.lInChannelPresentFlags |= AE_DSP_PRSNT_CH_FL;
  if (m_InputFormat.m_channelLayout.HasChannel(AE_CH_FR))   m_AddonSettings.lInChannelPresentFlags |= AE_DSP_PRSNT_CH_FR;
  if (m_InputFormat.m_channelLayout.HasChannel(AE_CH_FC))   m_AddonSettings.lInChannelPresentFlags |= AE_DSP_PRSNT_CH_FC;
  if (m_InputFormat.m_channelLayout.HasChannel(AE_CH_LFE))  m_AddonSettings.lInChannelPresentFlags |= AE_DSP_PRSNT_CH_LFE;
  if (m_InputFormat.m_channelLayout.HasChannel(AE_CH_BL))   m_AddonSettings.lInChannelPresentFlags |= AE_DSP_PRSNT_CH_BL;
  if (m_InputFormat.m_channelLayout.HasChannel(AE_CH_BR))   m_AddonSettings.lInChannelPresentFlags |= AE_DSP_PRSNT_CH_BR;
  if (m_InputFormat.m_channelLayout.HasChannel(AE_CH_FLOC)) m_AddonSettings.lInChannelPresentFlags |= AE_DSP_PRSNT_CH_FLOC;
  if (m_InputFormat.m_channelLayout.HasChannel(AE_CH_FROC)) m_AddonSettings.lInChannelPresentFlags |= AE_DSP_PRSNT_CH_FROC;
  if (m_InputFormat.m_channelLayout.HasChannel(AE_CH_BC))   m_AddonSettings.lInChannelPresentFlags |= AE_DSP_PRSNT_CH_BC;
  if (m_InputFormat.m_channelLayout.HasChannel(AE_CH_SL))   m_AddonSettings.lInChannelPresentFlags |= AE_DSP_PRSNT_CH_SL;
  if (m_InputFormat.m_channelLayout.HasChannel(AE_CH_SR))   m_AddonSettings.lInChannelPresentFlags |= AE_DSP_PRSNT_CH_SR;
  if (m_InputFormat.m_channelLayout.HasChannel(AE_CH_TFL))  m_AddonSettings.lInChannelPresentFlags |= AE_DSP_PRSNT_CH_TFL;
  if (m_InputFormat.m_channelLayout.HasChannel(AE_CH_TFR))  m_AddonSettings.lInChannelPresentFlags |= AE_DSP_PRSNT_CH_TFR;
  if (m_InputFormat.m_channelLayout.HasChannel(AE_CH_TFC))  m_AddonSettings.lInChannelPresentFlags |= AE_DSP_PRSNT_CH_TFC;
  if (m_InputFormat.m_channelLayout.HasChannel(AE_CH_TC))   m_AddonSettings.lInChannelPresentFlags |= AE_DSP_PRSNT_CH_TC;
  if (m_InputFormat.m_channelLayout.HasChannel(AE_CH_TBL))  m_AddonSettings.lInChannelPresentFlags |= AE_DSP_PRSNT_CH_TBL;
  if (m_InputFormat.m_channelLayout.HasChannel(AE_CH_TBR))  m_AddonSettings.lInChannelPresentFlags |= AE_DSP_PRSNT_CH_TBR;
  if (m_InputFormat.m_channelLayout.HasChannel(AE_CH_TBC))  m_AddonSettings.lInChannelPresentFlags |= AE_DSP_PRSNT_CH_TBC;

  if (m_OutputFormat.m_channelLayout.HasChannel(AE_CH_FL))   m_AddonSettings.lOutChannelPresentFlags |= AE_DSP_PRSNT_CH_FL;
  if (m_OutputFormat.m_channelLayout.HasChannel(AE_CH_FR))   m_AddonSettings.lOutChannelPresentFlags |= AE_DSP_PRSNT_CH_FR;
  if (m_OutputFormat.m_channelLayout.HasChannel(AE_CH_FC))   m_AddonSettings.lOutChannelPresentFlags |= AE_DSP_PRSNT_CH_FC;
  if (m_OutputFormat.m_channelLayout.HasChannel(AE_CH_LFE))  m_AddonSettings.lOutChannelPresentFlags |= AE_DSP_PRSNT_CH_LFE;
  if (m_OutputFormat.m_channelLayout.HasChannel(AE_CH_BL))   m_AddonSettings.lOutChannelPresentFlags |= AE_DSP_PRSNT_CH_BL;
  if (m_OutputFormat.m_channelLayout.HasChannel(AE_CH_BR))   m_AddonSettings.lOutChannelPresentFlags |= AE_DSP_PRSNT_CH_BR;
  if (m_OutputFormat.m_channelLayout.HasChannel(AE_CH_FLOC)) m_AddonSettings.lOutChannelPresentFlags |= AE_DSP_PRSNT_CH_FLOC;
  if (m_OutputFormat.m_channelLayout.HasChannel(AE_CH_FROC)) m_AddonSettings.lOutChannelPresentFlags |= AE_DSP_PRSNT_CH_FROC;
  if (m_OutputFormat.m_channelLayout.HasChannel(AE_CH_BC))   m_AddonSettings.lOutChannelPresentFlags |= AE_DSP_PRSNT_CH_BC;
  if (m_OutputFormat.m_channelLayout.HasChannel(AE_CH_SL))   m_AddonSettings.lOutChannelPresentFlags |= AE_DSP_PRSNT_CH_SL;
  if (m_OutputFormat.m_channelLayout.HasChannel(AE_CH_SR))   m_AddonSettings.lOutChannelPresentFlags |= AE_DSP_PRSNT_CH_SR;
  if (m_OutputFormat.m_channelLayout.HasChannel(AE_CH_TFL))  m_AddonSettings.lOutChannelPresentFlags |= AE_DSP_PRSNT_CH_TFL;
  if (m_OutputFormat.m_channelLayout.HasChannel(AE_CH_TFR))  m_AddonSettings.lOutChannelPresentFlags |= AE_DSP_PRSNT_CH_TFR;
  if (m_OutputFormat.m_channelLayout.HasChannel(AE_CH_TFC))  m_AddonSettings.lOutChannelPresentFlags |= AE_DSP_PRSNT_CH_TFC;
  if (m_OutputFormat.m_channelLayout.HasChannel(AE_CH_TC))   m_AddonSettings.lOutChannelPresentFlags |= AE_DSP_PRSNT_CH_TC;
  if (m_OutputFormat.m_channelLayout.HasChannel(AE_CH_TBL))  m_AddonSettings.lOutChannelPresentFlags |= AE_DSP_PRSNT_CH_TBL;
  if (m_OutputFormat.m_channelLayout.HasChannel(AE_CH_TBR))  m_AddonSettings.lOutChannelPresentFlags |= AE_DSP_PRSNT_CH_TBR;
  if (m_OutputFormat.m_channelLayout.HasChannel(AE_CH_TBC))  m_AddonSettings.lOutChannelPresentFlags |= AE_DSP_PRSNT_CH_TBC;

  /*!
   * Setup off mode, used if dsp master processing is set off, required to have data
   * for stream information functions.
   */
  sDSPProcessHandle internalMode;
  internalMode.Clear();
  internalMode.iAddonModeNumber = AE_DSP_MASTER_MODE_ID_PASSOVER;
  internalMode.pMode            = CActiveAEDSPModePtr(new CActiveAEDSPMode(internalMode.iAddonModeNumber, (AE_DSP_BASETYPE)m_AddonStreamProperties.iBaseType));
  internalMode.iLastTime        = 0;
  m_Addons_MasterProc.push_back(internalMode);
  m_ActiveMode = AE_DSP_MASTER_MODE_ID_PASSOVER;

  if (upmix && m_AddonSettings.iInChannels <= 2)
  {
    internalMode.Clear();
    internalMode.iAddonModeNumber = AE_DSP_MASTER_MODE_ID_INTERNAL_STEREO_UPMIX;
    internalMode.pMode            = CActiveAEDSPModePtr(new CActiveAEDSPMode(internalMode.iAddonModeNumber, (AE_DSP_BASETYPE)m_AddonStreamProperties.iBaseType));
    internalMode.iLastTime        = 0;
    m_Addons_MasterProc.push_back(internalMode);
  }

  /*!
   * Load all selected processing types, stored in a database and available from addons
   */
  AE_DSP_ADDONMAP addonMap;
  if (CActiveAEDSP::Get().GetEnabledAudioDSPAddons(addonMap) > 0)
  {
    int foundInputResamplerId = -1; /*!< Used to prevent double call of StreamCreate if input stream resampling is together with outer processing types */

    /*!
     * First find input resample addon to become information about processing sample rate and
     * load one allowed before master processing & final resample addon
     */
    CLog::Log(LOGDEBUG, "  ---- DSP input resample addon ---");
    const AE_DSP_MODELIST listInputResample = CActiveAEDSP::Get().GetAvailableModes(AE_DSP_MODE_TYPE_INPUT_RESAMPLE);
    if (listInputResample.size() == 0)
      CLog::Log(LOGDEBUG, "  | - no input resample addon present or enabled");
    for (unsigned int i = 0; i < listInputResample.size(); i++)
    {
      /// For resample only one call is allowed. Use first one and ignore everything else.
      CActiveAEDSPModePtr pMode = listInputResample[i].first;
      AE_DSP_ADDON        addon = listInputResample[i].second;
      if (addon->Enabled() && addon->SupportsInputResample() && pMode->IsEnabled())
      {
        AE_DSP_ERROR err = addon->StreamCreate(&m_AddonSettings, &m_AddonStreamProperties);
        if (err == AE_DSP_ERROR_NO_ERROR)
        {
          if (addon->StreamIsModeSupported(m_StreamId, pMode->ModeType(), pMode->AddonModeNumber(), pMode->ModeID()))
          {
            int processSamplerate = addon->InputResampleSampleRate(m_StreamId);
            if (processSamplerate == (int)m_InputFormat.m_sampleRate)
            {
              CLog::Log(LOGDEBUG, "  | - input resample addon %s ignored, input sample rate %i the same as process rate", addon->GetFriendlyName().c_str(), m_InputFormat.m_sampleRate);
            }
            else if (processSamplerate > 0)
            {
              CLog::Log(LOGDEBUG, "  | - %s with resampling from %i to %i", addon->GetAudioDSPName().c_str(), m_InputFormat.m_sampleRate, processSamplerate);

              m_OutputSamplerate                      = processSamplerate;                  /*!< overwrite output sample rate with the new rate */
              m_OutputFrames                          = (int) ceil((1.0 * processSamplerate) / m_AddonSettings.iInSamplerate * m_AddonSettings.iInFrames);
              m_AddonSettings.iProcessSamplerate      = m_OutputSamplerate;                 /*!< the processing sample rate required for all behind called processes */
              m_AddonSettings.iProcessFrames          = m_OutputFrames;
              m_AddonSettings.bInputResamplingActive  = true;

              m_Addon_InputResample.iAddonModeNumber  = pMode->AddonModeNumber();
              m_Addon_InputResample.pMode             = pMode;
              m_Addon_InputResample.pAddon            = addon;
              m_Addon_InputResample.iLastTime         = 0;
            }
            else
            {
              CLog::Log(LOGERROR, "ActiveAE DSP - %s - input resample addon %s return invalid samplerate and becomes disabled", __FUNCTION__, addon->GetFriendlyName().c_str());
            }

            foundInputResamplerId = addon->GetID();
            m_usedMap.insert(std::make_pair(addon->GetID(), addon));
          }
        }
        else if (err != AE_DSP_ERROR_IGNORE_ME)
          CLog::Log(LOGERROR, "ActiveAE DSP - %s - input resample addon creation failed on %s with %s", __FUNCTION__, addon->GetAudioDSPName().c_str(), CActiveAEDSPAddon::ToString(err));
        break;
      }
    }

    /*!
     * Now init all other dsp relavant addons
     */
    for (AE_DSP_ADDONMAP_ITR itr = addonMap.begin(); itr != addonMap.end(); itr++)
    {
      AE_DSP_ADDON addon = itr->second;
      if (addon->Enabled() && addon->GetID() != foundInputResamplerId)
      {
        AE_DSP_ERROR err = addon->StreamCreate(&m_AddonSettings, &m_AddonStreamProperties);
        if (err == AE_DSP_ERROR_NO_ERROR)
        {
          m_usedMap.insert(std::make_pair(addon->GetID(), addon));
        }
        else if (err == AE_DSP_ERROR_IGNORE_ME)
          continue;
        else
          CLog::Log(LOGERROR, "ActiveAE DSP - %s - addon creation failed on %s with %s", __FUNCTION__, addon->GetAudioDSPName().c_str(), CActiveAEDSPAddon::ToString(err));
      }
    }

    for (AE_DSP_ADDONMAP_ITR itr = m_usedMap.begin(); itr != m_usedMap.end(); itr++)
    {
      AE_DSP_ADDON addon = itr->second;
      if (addon->SupportsInputInfoProcess())
        m_Addons_InputProc.push_back(addon);
    }

    /*!
     * Load all required pre process dsp addon functions
     */
    CLog::Log(LOGDEBUG, "  ---- DSP active pre process modes ---");
    const AE_DSP_MODELIST listPreProcess = CActiveAEDSP::Get().GetAvailableModes(AE_DSP_MODE_TYPE_PRE_PROCESS);
    for (unsigned int i = 0; i < listPreProcess.size(); i++)
    {
      CActiveAEDSPModePtr pMode = listPreProcess[i].first;
      AE_DSP_ADDON        addon = listPreProcess[i].second;

      if (m_usedMap.find(addon->GetID()) == m_usedMap.end())
        continue;
      if (addon->Enabled() && addon->SupportsPreProcess() && pMode->IsEnabled() &&
          addon->StreamIsModeSupported(m_StreamId, pMode->ModeType(), pMode->AddonModeNumber(), pMode->ModeID()))
      {
        CLog::Log(LOGDEBUG, "  | - %i - %s (%s)", i, pMode->AddonModeName().c_str(), addon->GetAudioDSPName().c_str());

        sDSPProcessHandle modeHandle;
        modeHandle.iAddonModeNumber = pMode->AddonModeNumber();
        modeHandle.pMode            = pMode;
        modeHandle.pAddon           = addon;
        modeHandle.iLastTime        = 0;
        m_Addons_PreProc.push_back(modeHandle);
      }
    }
    if (m_Addons_PreProc.empty())
      CLog::Log(LOGDEBUG, "  | - no pre processing addon's present or enabled");

    /*!
     * Load all available master modes from addons and put together with database
     */
    CLog::Log(LOGDEBUG, "  ---- DSP active master process modes ---");
    const AE_DSP_MODELIST listMasterProcess = CActiveAEDSP::Get().GetAvailableModes(AE_DSP_MODE_TYPE_MASTER_PROCESS);
    for (unsigned int i = 0; i < listMasterProcess.size(); i++)
    {
      CActiveAEDSPModePtr pMode = listMasterProcess[i].first;
      AE_DSP_ADDON        addon = listMasterProcess[i].second;

      if (m_usedMap.find(addon->GetID()) == m_usedMap.end())
        continue;
      if (addon->Enabled() && addon->SupportsMasterProcess() && pMode->IsEnabled() &&
          addon->StreamIsModeSupported(m_StreamId, pMode->ModeType(), pMode->AddonModeNumber(), pMode->ModeID()))
      {
        CLog::Log(LOGDEBUG, "  | - %i - %s (%s)", i, pMode->AddonModeName().c_str(), addon->GetAudioDSPName().c_str());

        sDSPProcessHandle modeHandle;
        modeHandle.iAddonModeNumber = pMode->AddonModeNumber();
        modeHandle.pMode            = pMode;
        modeHandle.pAddon           = addon;
        modeHandle.iLastTime        = 0;
        modeHandle.pMode->SetBaseType((AE_DSP_BASETYPE)m_AddonStreamProperties.iBaseType);
        m_Addons_MasterProc.push_back(modeHandle);
      }
    }
    if (m_Addons_MasterProc.empty())
      CLog::Log(LOGDEBUG, "  | - no master processing addon's present or enabled");

    /*!
     * Get selected source for current input
     */
    int ModeID = CMediaSettings::Get().GetCurrentAudioSettings().m_MasterModes[m_AddonStreamProperties.iStreamType][m_AddonStreamProperties.iBaseType];
    if (ModeID == AE_DSP_MASTER_MODE_ID_INVALID)
      ModeID = AE_DSP_MASTER_MODE_ID_PASSOVER;

    for (unsigned int ptr = 0; ptr < m_Addons_MasterProc.size(); ptr++)
    {
      CActiveAEDSPModePtr mode = m_Addons_MasterProc.at(ptr).pMode;
      if (mode->ModeID() == ModeID)
      {
        m_ActiveMode = (int)ptr;
        CLog::Log(LOGDEBUG, "  | -- %s (selected)", mode->AddonModeName().c_str());
        break;
      }
    }

    /*!
     * Setup the one allowed master processing addon and inform about selected mode
     */
    m_ActiveModeOutChannels = -1;
    if (m_Addons_MasterProc[m_ActiveMode].pAddon)
    {
      AE_DSP_ERROR err = m_Addons_MasterProc[m_ActiveMode].pAddon->MasterProcessSetMode(m_StreamId, m_AddonStreamProperties.iStreamType, m_Addons_MasterProc[m_ActiveMode].pMode->AddonModeNumber(), m_Addons_MasterProc[m_ActiveMode].pMode->ModeID());
      if (err != AE_DSP_ERROR_NO_ERROR)
      {
        CLog::Log(LOGERROR, "ActiveAE DSP - %s - addon master mode selection failed on %s with Mode '%s' with %s",
                                __FUNCTION__,
                                m_Addons_MasterProc[m_ActiveMode].pAddon->GetAudioDSPName().c_str(),
                                m_Addons_MasterProc[m_ActiveMode].pMode->AddonModeName().c_str(),
                                CActiveAEDSPAddon::ToString(err));
        m_Addons_MasterProc.erase(m_Addons_MasterProc.begin()+m_ActiveMode);
        m_ActiveMode = AE_DSP_MASTER_MODE_ID_PASSOVER;
      }
      else
        m_ActiveModeOutChannels = m_Addons_MasterProc[m_ActiveMode].pAddon->MasterProcessGetOutChannels(m_StreamId, m_ActiveModeOutChannelsPresent);
    }
    else
    {
      CLog::Log(LOGDEBUG, "  | -- No master process selected!");
    }

    /*!
     * Load all required post process dsp addon functions
     */
    CLog::Log(LOGDEBUG, "  ---- DSP active post process modes ---");
    const AE_DSP_MODELIST listPostProcess = CActiveAEDSP::Get().GetAvailableModes(AE_DSP_MODE_TYPE_POST_PROCESS);
    for (unsigned int i = 0; i < listPostProcess.size(); i++)
    {
      CActiveAEDSPModePtr pMode = listPostProcess[i].first;
      AE_DSP_ADDON        addon = listPostProcess[i].second;

      if (m_usedMap.find(addon->GetID()) == m_usedMap.end())
        continue;

      if (addon->Enabled() && addon->SupportsPostProcess() && pMode->IsEnabled() &&
          addon->StreamIsModeSupported(m_StreamId, pMode->ModeType(), pMode->AddonModeNumber(), pMode->ModeID()))
      {
        CLog::Log(LOGDEBUG, "  | - %i - %s (%s)", i, pMode->AddonModeName().c_str(), addon->GetAudioDSPName().c_str());

        sDSPProcessHandle modeHandle;
        modeHandle.iAddonModeNumber = pMode->AddonModeNumber();
        modeHandle.pMode            = pMode;
        modeHandle.pAddon           = addon;
        modeHandle.iLastTime        = 0;
        m_Addons_PostProc.push_back(modeHandle);
      }
    }
    if (m_Addons_PostProc.empty())
      CLog::Log(LOGDEBUG, "  | - no post processing addon's present or enabled");

    /*!
     * Load one allowed addon for resampling after final post processing
     */
    CLog::Log(LOGDEBUG, "  ---- DSP post resample addon ---");
    if (m_AddonSettings.iProcessSamplerate != m_OutputFormat.m_sampleRate)
    {
      const AE_DSP_MODELIST listOutputResample = CActiveAEDSP::Get().GetAvailableModes(AE_DSP_MODE_TYPE_OUTPUT_RESAMPLE);
      if (listOutputResample.size() == 0)
        CLog::Log(LOGDEBUG, "  | - no final post resample addon present or enabled, becomes performed by XBMC");
      for (unsigned int i = 0; i < listOutputResample.size(); i++)
      {
        /// For resample only one call is allowed. Use first one and ignore everything else.
        CActiveAEDSPModePtr pMode = listOutputResample[i].first;
        AE_DSP_ADDON        addon = listOutputResample[i].second;
        if (m_usedMap.find(addon->GetID()) != m_usedMap.end() &&
            addon->Enabled() && addon->SupportsOutputResample() && pMode->IsEnabled() &&
            addon->StreamIsModeSupported(m_StreamId, pMode->ModeType(), pMode->AddonModeNumber(), pMode->ModeID()))
        {
          int outSamplerate = addon->OutputResampleSampleRate(m_StreamId);
          if (outSamplerate > 0)
          {
            CLog::Log(LOGDEBUG, "  | - %s with resampling to %i", addon->GetAudioDSPName().c_str(), outSamplerate);

            m_OutputSamplerate                      = outSamplerate;
            m_OutputFrames                          = (int) ceil((1.0 * outSamplerate) / m_AddonSettings.iProcessSamplerate * m_AddonSettings.iProcessFrames);

            m_Addon_OutputResample.iAddonModeNumber = pMode->AddonModeNumber();
            m_Addon_OutputResample.pMode            = pMode;
            m_Addon_OutputResample.pAddon           = addon;
            m_Addon_OutputResample.iLastTime        = 0;
          }
          else
          {
            CLog::Log(LOGERROR, "ActiveAE DSP - %s - post resample addon %s return invalid samplerate and becomes disabled", __FUNCTION__, addon->GetFriendlyName().c_str());
          }
          break;
        }
      }
    }
    else
    {
      CLog::Log(LOGDEBUG, "  | - no final resampling needed, process and final samplerate the same");
    }
  }

  /*!
   * Initialize fallback matrix mixer
   */
  InitFFMpegDSPProcessor();

  if (CLog::GetLogLevel() == LOGDEBUG) // Speed improve
  {
    CLog::Log(LOGDEBUG, "  ----  Input stream  ----");
    CLog::Log(LOGDEBUG, "  | Identifier           : %d", m_AddonStreamProperties.iIdentifier);
    CLog::Log(LOGDEBUG, "  | Stream Type          : %s", m_AddonStreamProperties.iStreamType == AE_DSP_ASTREAM_BASIC   ? "Basic"   :
                                                         m_AddonStreamProperties.iStreamType == AE_DSP_ASTREAM_MUSIC   ? "Music"   :
                                                         m_AddonStreamProperties.iStreamType == AE_DSP_ASTREAM_MOVIE   ? "Movie"   :
                                                         m_AddonStreamProperties.iStreamType == AE_DSP_ASTREAM_GAME    ? "Game"    :
                                                         m_AddonStreamProperties.iStreamType == AE_DSP_ASTREAM_APP     ? "App"     :
                                                         m_AddonStreamProperties.iStreamType == AE_DSP_ASTREAM_PHONE   ? "Phone"   :
                                                         m_AddonStreamProperties.iStreamType == AE_DSP_ASTREAM_MESSAGE ? "Message" :
                                                         "Unknown");
    CLog::Log(LOGDEBUG, "  | Name                 : %s", m_AddonStreamProperties.strName);
    CLog::Log(LOGDEBUG, "  | Language             : %s", m_AddonStreamProperties.strLanguage);
    CLog::Log(LOGDEBUG, "  | Codec                : %s", m_AddonStreamProperties.strCodecId);
    CLog::Log(LOGDEBUG, "  | Sample Rate          : %d", m_AddonStreamProperties.iSampleRate);
    CLog::Log(LOGDEBUG, "  | Channels             : %d", m_AddonStreamProperties.iChannels);
    CLog::Log(LOGDEBUG, "  ----  Input format  ----");
    CLog::Log(LOGDEBUG, "  | Sample Rate          : %d", m_AddonSettings.iInSamplerate);
    CLog::Log(LOGDEBUG, "  | Sample Format        : %s", CAEUtil::DataFormatToStr(m_InputFormat.m_dataFormat));
    CLog::Log(LOGDEBUG, "  | Channel Count        : %d", m_InputFormat.m_channelLayout.Count());
    CLog::Log(LOGDEBUG, "  | Channel Layout       : %s", ((string)m_InputFormat.m_channelLayout).c_str());
    CLog::Log(LOGDEBUG, "  | Frames               : %d", m_AddonSettings.iInFrames);
    CLog::Log(LOGDEBUG, "  ----  Process format ----");
    CLog::Log(LOGDEBUG, "  | Sample Rate          : %d", m_AddonSettings.iProcessSamplerate);
    CLog::Log(LOGDEBUG, "  | Sample Format        : %s", "AE_FMT_FLOATP");
    CLog::Log(LOGDEBUG, "  | Frames               : %d", m_AddonSettings.iProcessFrames);
    CLog::Log(LOGDEBUG, "  | Internal processing  : %s", m_ResamplerDSPProcessor ? "yes" : "no");
    CLog::Log(LOGDEBUG, "  ----  Output format ----");
    CLog::Log(LOGDEBUG, "  | Sample Rate          : %d", m_OutputSamplerate);
    CLog::Log(LOGDEBUG, "  | Sample Format        : %s", CAEUtil::DataFormatToStr(m_OutputFormat.m_dataFormat));
    CLog::Log(LOGDEBUG, "  | Channel Count        : %d", m_OutputFormat.m_channelLayout.Count());
    CLog::Log(LOGDEBUG, "  | Channel Layout       : %s", ((string)m_OutputFormat.m_channelLayout).c_str());
    CLog::Log(LOGDEBUG, "  | Frames               : %d", m_OutputFrames);
  }

  m_ForceInit = true;
  return true;
}

void CActiveAEDSPProcess::InitFFMpegDSPProcessor()
{
  /*!
   * If ffmpeg resampler is already present delete it first to create it from new
   */
  if (m_ResamplerDSPProcessor)
  {
    delete m_ResamplerDSPProcessor;
    m_ResamplerDSPProcessor = NULL;
  }

  /*!
   * if the amount of input channels is higher as output and the active master mode gives more channels out or if it is not set of it
   * a forced channel downmix becomes enabled.
   */
  bool upmix = m_AddonSettings.bStereoUpmix && m_Addons_MasterProc[m_ActiveMode].pMode->ModeID() == AE_DSP_MASTER_MODE_ID_INTERNAL_STEREO_UPMIX ? true : false;
  if (upmix || (m_AddonSettings.iInChannels > m_AddonSettings.iOutChannels && (m_ActiveModeOutChannels <= 0 || m_ActiveModeOutChannels > m_AddonSettings.iOutChannels)))
  {
    m_ResamplerDSPProcessor = CAEResampleFactory::Create();
    if (!m_ResamplerDSPProcessor->Init(CAEUtil::GetAVChannelLayout(m_OutputFormat.m_channelLayout),
                                m_OutputFormat.m_channelLayout.Count(),
                                m_AddonSettings.iProcessSamplerate,
                                AV_SAMPLE_FMT_FLTP, sizeof(float) << 3, 0,
                                CAEUtil::GetAVChannelLayout(m_InputFormat.m_channelLayout),
                                m_InputFormat.m_channelLayout.Count(),
                                m_AddonSettings.iProcessSamplerate,
                                AV_SAMPLE_FMT_FLTP, sizeof(float) << 3, 0,
                                upmix,
                                true,
                                NULL,
                                m_StreamQuality))
    {
      delete m_ResamplerDSPProcessor;
      m_ResamplerDSPProcessor = NULL;

      CLog::Log(LOGERROR, "ActiveAE DSP - %s - Initialize of channel mixer failed", __FUNCTION__);
    }
  }
}

bool CActiveAEDSPProcess::CreateStreamProfile()
{
  switch (m_AddonStreamProperties.iBaseType)
  {
    case AE_DSP_ABASE_AC3:
    case AE_DSP_ABASE_EAC3:
      /// TODO: Add handling for ffmpeg related stream information
      break;
    case AE_DSP_ABASE_DTS:
    case AE_DSP_ABASE_DTSHD_HRA:
    case AE_DSP_ABASE_DTSHD_MA:
      /// TODO: Add handling for ffmpeg related stream information
      break;
    case AE_DSP_ABASE_TRUEHD:
    case AE_DSP_ABASE_MLP:
      /// TODO: Add handling for ffmpeg related stream information
      break;
    case AE_DSP_ABASE_FLAC:
    default:
      break;
  };
  return false;
}

void CActiveAEDSPProcess::Destroy()
{
  CSingleLock lock(m_restartSection);

  if (!CActiveAEDSP::Get().IsActivated())
    return;

  for (AE_DSP_ADDONMAP_ITR itr = m_usedMap.begin(); itr != m_usedMap.end(); itr++)
  {
    itr->second->StreamDestroy(m_StreamId);
  }

  ResetStreamFunctionsSelection();
}

void CActiveAEDSPProcess::ForceReinit()
{
  CSingleLock lock(m_restartSection);
  m_ForceInit = true;
}

AE_DSP_STREAMTYPE CActiveAEDSPProcess::DetectStreamType(const CFileItem *item)
{
  AE_DSP_STREAMTYPE detected = AE_DSP_ASTREAM_BASIC;
  if (item->HasMusicInfoTag())
    detected = AE_DSP_ASTREAM_MUSIC;
  else if (item->HasVideoInfoTag() || g_application.m_pPlayer->HasVideo())
    detected = AE_DSP_ASTREAM_MOVIE;
//    else if (item->HasVideoInfoTag())
//      detected = AE_DSP_ASTREAM_GAME;
//    else if (item->HasVideoInfoTag())
//      detected = AE_DSP_ASTREAM_APP;
//    else if (item->HasVideoInfoTag())
//      detected = AE_DSP_ASTREAM_MESSAGE;
//    else if (item->HasVideoInfoTag())
//      detected = AE_DSP_ASTREAM_PHONE;
  else
    detected = AE_DSP_ASTREAM_BASIC;

  return detected;
}

AE_DSP_STREAM_ID CActiveAEDSPProcess::GetStreamId() const
{
  return m_StreamId;
}

unsigned int CActiveAEDSPProcess::GetInputChannels()
{
  return m_InputFormat.m_channelLayout.Count();
}

string CActiveAEDSPProcess::GetInputChannelNames()
{
  return m_InputFormat.m_channelLayout;
}

unsigned int CActiveAEDSPProcess::GetInputSamplerate()
{
  return m_InputFormat.m_sampleRate;
}

unsigned int CActiveAEDSPProcess::GetProcessSamplerate()
{
  return m_AddonSettings.iProcessSamplerate;
}

unsigned int CActiveAEDSPProcess::GetOutputChannels()
{
  return m_OutputFormat.m_channelLayout.Count();
}

string CActiveAEDSPProcess::GetOutputChannelNames()
{
  return m_OutputFormat.m_channelLayout;
}

unsigned int CActiveAEDSPProcess::GetOutputSamplerate()
{
  return m_OutputSamplerate;
}

unsigned int CActiveAEDSPProcess::GetOutputFrames()
{
  return m_OutputFrames;
}

float CActiveAEDSPProcess::GetCPUUsage(void) const
{
  return m_fLastProcessUsage;
}

CAEChannelInfo CActiveAEDSPProcess::GetChannelLayout()
{
  return m_OutputFormat.m_channelLayout;
}

AEDataFormat CActiveAEDSPProcess::GetDataFormat()
{
  return m_dataFormat;
}

AEAudioFormat CActiveAEDSPProcess::GetInputFormat()
{
  return m_InputFormat;
}

AE_DSP_STREAMTYPE CActiveAEDSPProcess::GetDetectedStreamType()
{
  return m_StreamTypeDetected;
}

AE_DSP_STREAMTYPE CActiveAEDSPProcess::GetUsedStreamType()
{
  return m_StreamTypeUsed;
}

AE_DSP_BASETYPE CActiveAEDSPProcess::GetBaseType(AE_DSP_STREAM_PROPERTIES *props)
{
  if (!strcmp(props->strCodecId, "ac3"))
    return AE_DSP_ABASE_AC3;
  else if (!strcmp(props->strCodecId, "eac3"))
    return AE_DSP_ABASE_EAC3;
  else if (!strcmp(props->strCodecId, "dca") || !strcmp(props->strCodecId, "dts"))
    return AE_DSP_ABASE_DTS;
  else if (!strcmp(props->strCodecId, "dtshd_hra"))
    return AE_DSP_ABASE_DTSHD_HRA;
  else if (!strcmp(props->strCodecId, "dtshd_ma"))
    return AE_DSP_ABASE_DTSHD_MA;
  else if (!strcmp(props->strCodecId, "truehd"))
    return AE_DSP_ABASE_TRUEHD;
  else if (!strcmp(props->strCodecId, "mlp"))
    return AE_DSP_ABASE_MLP;
  else if (!strcmp(props->strCodecId, "flac"))
    return AE_DSP_ABASE_FLAC;
  else if (props->iChannels > 2)
    return AE_DSP_ABASE_MULTICHANNEL;
  else if (props->iChannels == 2)
    return AE_DSP_ABASE_STEREO;
  else
    return AE_DSP_ABASE_MONO;
}

AE_DSP_BASETYPE CActiveAEDSPProcess::GetUsedBaseType()
{
  return GetBaseType(&m_AddonStreamProperties);
}

bool CActiveAEDSPProcess::GetMasterModeStreamInfoString(string &strInfo)
{
  if (m_ActiveMode <= AE_DSP_MASTER_MODE_ID_PASSOVER)
  {
    strInfo = "";
    return true;
  }

  if (m_ActiveMode < 0 || !m_Addons_MasterProc[m_ActiveMode].pAddon)
    return false;

  strInfo = m_Addons_MasterProc[m_ActiveMode].pAddon->MasterProcessGetStreamInfoString(m_StreamId);

  return true;
}

bool CActiveAEDSPProcess::GetMasterModeTypeInformation(AE_DSP_STREAMTYPE &streamTypeUsed, AE_DSP_BASETYPE &baseType, int &iModeID)
{
  streamTypeUsed  = (AE_DSP_STREAMTYPE)m_AddonStreamProperties.iStreamType;

  if (m_ActiveMode < 0)
    return false;

  baseType = m_Addons_MasterProc[m_ActiveMode].pMode->BaseType();
  iModeID  = m_Addons_MasterProc[m_ActiveMode].pMode->ModeID();
  return true;
}

const char *CActiveAEDSPProcess::GetStreamTypeName(AE_DSP_STREAMTYPE iStreamType)
{
  return iStreamType == AE_DSP_ASTREAM_BASIC   ? "Basic"     :
         iStreamType == AE_DSP_ASTREAM_MUSIC   ? "Music"     :
         iStreamType == AE_DSP_ASTREAM_MOVIE   ? "Movie"     :
         iStreamType == AE_DSP_ASTREAM_GAME    ? "Game"      :
         iStreamType == AE_DSP_ASTREAM_APP     ? "App"       :
         iStreamType == AE_DSP_ASTREAM_PHONE   ? "Phone"     :
         iStreamType == AE_DSP_ASTREAM_MESSAGE ? "Message"   :
         iStreamType == AE_DSP_ASTREAM_AUTO    ? "Automatic" :
         "Unknown";
}

bool CActiveAEDSPProcess::MasterModeChange(int iModeID, AE_DSP_STREAMTYPE iStreamType)
{
  bool bReturn           = false;
  bool bSwitchStreamType = iStreamType != AE_DSP_ASTREAM_INVALID;

  /* The Mode is already used and need not to set up again */
  if (m_Addons_MasterProc[m_ActiveMode].pMode->ModeID() == iModeID && !bSwitchStreamType)
    return true;

  CSingleLock lock(m_restartSection);

  CLog::Log(LOGDEBUG, "ActiveAE DSP - %s - Audio DSP processing id %d mode change:", __FUNCTION__, m_StreamId);
  if (bSwitchStreamType && m_StreamTypeUsed != iStreamType)
  {
    AE_DSP_STREAMTYPE old = m_StreamTypeUsed;
    CLog::Log(LOGDEBUG, "  ----  Input stream  ----");
    if (iStreamType == AE_DSP_ASTREAM_AUTO)
      m_StreamTypeUsed = m_StreamTypeDetected;
    else if (iStreamType >= AE_DSP_ASTREAM_BASIC || iStreamType < AE_DSP_ASTREAM_AUTO)
      m_StreamTypeUsed = iStreamType;
    else
    {
      CLog::Log(LOGWARNING, "ActiveAE DSP - %s - Unknown audio stream type, falling back to basic", __FUNCTION__);
      m_StreamTypeUsed = AE_DSP_ASTREAM_BASIC;
    }

    CLog::Log(LOGDEBUG, "  | Stream Type change   : From '%s' to '%s'", GetStreamTypeName(old), GetStreamTypeName(m_StreamTypeUsed));
  }

  /*!
   * Set the new stream type to the addon settings and properties structures.
   * If the addon want to use another stream type, it can be becomes written inside
   * the m_AddonStreamProperties.iStreamType.
   */
  m_AddonStreamProperties.iStreamType = m_StreamTypeUsed;
  m_AddonSettings.iStreamType         = m_StreamTypeUsed;
  m_ActiveModeOutChannels             = -1;

  if (iModeID <= AE_DSP_MASTER_MODE_ID_PASSOVER)
  {
    CLog::Log(LOGINFO, "ActiveAE DSP - Switching master mode off");
    m_ActiveMode = 0;
    bReturn      = true;
  }
  else
  {
    CActiveAEDSPModePtr mode;
    for (unsigned int ptr = 0; ptr < m_Addons_MasterProc.size(); ptr++)
    {
      mode = m_Addons_MasterProc.at(ptr).pMode;
      if (mode->ModeID() == iModeID && mode->IsEnabled())
      {
        if (m_Addons_MasterProc[ptr].pAddon)
        {
          AE_DSP_ERROR err = m_Addons_MasterProc[ptr].pAddon->MasterProcessSetMode(m_StreamId, m_AddonStreamProperties.iStreamType, mode->AddonModeNumber(), mode->ModeID());
          if (err != AE_DSP_ERROR_NO_ERROR)
          {
            CLog::Log(LOGERROR, "ActiveAE DSP - %s - addon master mode selection failed on %s with Mode '%s' with %s",
                                    __FUNCTION__,
                                    m_Addons_MasterProc[ptr].pAddon->GetAudioDSPName().c_str(),
                                    mode->AddonModeName().c_str(),
                                    CActiveAEDSPAddon::ToString(err));
          }
          else
          {
            CLog::Log(LOGINFO, "ActiveAE DSP - Switching master mode to '%s' as '%s' on '%s'",
                                    mode->AddonModeName().c_str(),
                                    GetStreamTypeName((AE_DSP_STREAMTYPE)m_AddonStreamProperties.iStreamType),
                                    m_Addons_MasterProc[ptr].pAddon->GetAudioDSPName().c_str());

            m_ActiveMode            = (int)ptr;
            m_ActiveModeOutChannels = m_Addons_MasterProc[m_ActiveMode].pAddon->MasterProcessGetOutChannels(m_StreamId, m_ActiveModeOutChannelsPresent);
            bReturn                 = true;
          }
        }
        else if (mode->ModeID() >= AE_DSP_MASTER_MODE_ID_INTERNAL_TYPES)
        {
          CLog::Log(LOGINFO, "ActiveAE DSP - Switching master mode to internal '%s' as '%s'",
                                  mode->AddonModeName().c_str(),
                                  GetStreamTypeName((AE_DSP_STREAMTYPE)m_AddonStreamProperties.iStreamType));

          m_ActiveMode            = (int)ptr;
          m_ActiveModeOutChannels = -1;
          bReturn                 = true;
        }
        break;
      }
    }
  }

  /*!
   * Initialize fallback matrix mixer
   */
  InitFFMpegDSPProcessor();

  return bReturn;
}

void CActiveAEDSPProcess::ClearArray(float **array, unsigned int samples)
{
  unsigned int presentFlag = 1;
  for (int i = 0; i < AE_DSP_CH_MAX; i++)
  {
    if (m_AddonSettings.lOutChannelPresentFlags & presentFlag)
      memset(array[i], 0, samples*sizeof(float));
    presentFlag <<= 1;
  }
}

bool CActiveAEDSPProcess::Process(CSampleBuffer *in, CSampleBuffer *out)
{
  CSingleLock lock(m_restartSection);

  bool needDSPAddonsReinit  = m_ForceInit;
  unsigned int iTime        = XbmcThreads::SystemClockMillis() * 10000;
  int64_t hostFrequency     = CurrentHostFrequency();
  unsigned int frames       = in->pkt->nb_samples;

  /* Detect interleaved input stream channel positions if unknown or changed */
  if (m_ChannelLayoutIn != in->pkt->config.channel_layout)
  {
    m_ChannelLayoutIn = in->pkt->config.channel_layout;

    m_idx_in[AE_CH_FL]    = av_get_channel_layout_channel_index(m_ChannelLayoutIn, AV_CH_FRONT_LEFT);
    m_idx_in[AE_CH_FR]    = av_get_channel_layout_channel_index(m_ChannelLayoutIn, AV_CH_FRONT_RIGHT);
    m_idx_in[AE_CH_FC]    = av_get_channel_layout_channel_index(m_ChannelLayoutIn, AV_CH_FRONT_CENTER);
    m_idx_in[AE_CH_LFE]   = av_get_channel_layout_channel_index(m_ChannelLayoutIn, AV_CH_LOW_FREQUENCY);
    m_idx_in[AE_CH_BL]    = av_get_channel_layout_channel_index(m_ChannelLayoutIn, AV_CH_BACK_LEFT);
    m_idx_in[AE_CH_BR]    = av_get_channel_layout_channel_index(m_ChannelLayoutIn, AV_CH_BACK_RIGHT);
    m_idx_in[AE_CH_FLOC]  = av_get_channel_layout_channel_index(m_ChannelLayoutIn, AV_CH_FRONT_LEFT_OF_CENTER);
    m_idx_in[AE_CH_FROC]  = av_get_channel_layout_channel_index(m_ChannelLayoutIn, AV_CH_FRONT_RIGHT_OF_CENTER);
    m_idx_in[AE_CH_BC]    = av_get_channel_layout_channel_index(m_ChannelLayoutIn, AV_CH_BACK_CENTER);
    m_idx_in[AE_CH_SL]    = av_get_channel_layout_channel_index(m_ChannelLayoutIn, AV_CH_SIDE_LEFT);
    m_idx_in[AE_CH_SR]    = av_get_channel_layout_channel_index(m_ChannelLayoutIn, AV_CH_SIDE_RIGHT);
    m_idx_in[AE_CH_TC]    = av_get_channel_layout_channel_index(m_ChannelLayoutIn, AV_CH_TOP_CENTER);
    m_idx_in[AE_CH_TFL]   = av_get_channel_layout_channel_index(m_ChannelLayoutIn, AV_CH_TOP_FRONT_LEFT);
    m_idx_in[AE_CH_TFC]   = av_get_channel_layout_channel_index(m_ChannelLayoutIn, AV_CH_TOP_FRONT_CENTER);
    m_idx_in[AE_CH_TFR]   = av_get_channel_layout_channel_index(m_ChannelLayoutIn, AV_CH_TOP_FRONT_RIGHT);
    m_idx_in[AE_CH_TBL]   = av_get_channel_layout_channel_index(m_ChannelLayoutIn, AV_CH_TOP_BACK_LEFT);
    m_idx_in[AE_CH_TBC]   = av_get_channel_layout_channel_index(m_ChannelLayoutIn, AV_CH_TOP_BACK_CENTER);
    m_idx_in[AE_CH_TBR]   = av_get_channel_layout_channel_index(m_ChannelLayoutIn, AV_CH_TOP_BACK_RIGHT);

    needDSPAddonsReinit = true;
  }

  /* Detect also interleaved output stream channel positions if unknown or changed */
  if (m_ChannelLayoutOut != out->pkt->config.channel_layout)
  {
    m_ChannelLayoutOut = out->pkt->config.channel_layout;

    m_idx_out[AE_CH_FL]   = av_get_channel_layout_channel_index(m_ChannelLayoutOut, AV_CH_FRONT_LEFT);
    m_idx_out[AE_CH_FR]   = av_get_channel_layout_channel_index(m_ChannelLayoutOut, AV_CH_FRONT_RIGHT);
    m_idx_out[AE_CH_FC]   = av_get_channel_layout_channel_index(m_ChannelLayoutOut, AV_CH_FRONT_CENTER);
    m_idx_out[AE_CH_LFE]  = av_get_channel_layout_channel_index(m_ChannelLayoutOut, AV_CH_LOW_FREQUENCY);
    m_idx_out[AE_CH_BL]   = av_get_channel_layout_channel_index(m_ChannelLayoutOut, AV_CH_BACK_LEFT);
    m_idx_out[AE_CH_BR]   = av_get_channel_layout_channel_index(m_ChannelLayoutOut, AV_CH_BACK_RIGHT);
    m_idx_out[AE_CH_FLOC] = av_get_channel_layout_channel_index(m_ChannelLayoutOut, AV_CH_FRONT_LEFT_OF_CENTER);
    m_idx_out[AE_CH_FROC] = av_get_channel_layout_channel_index(m_ChannelLayoutOut, AV_CH_FRONT_RIGHT_OF_CENTER);
    m_idx_out[AE_CH_BC]   = av_get_channel_layout_channel_index(m_ChannelLayoutOut, AV_CH_BACK_CENTER);
    m_idx_out[AE_CH_SL]   = av_get_channel_layout_channel_index(m_ChannelLayoutOut, AV_CH_SIDE_LEFT);
    m_idx_out[AE_CH_SR]   = av_get_channel_layout_channel_index(m_ChannelLayoutOut, AV_CH_SIDE_RIGHT);
    m_idx_out[AE_CH_TC]   = av_get_channel_layout_channel_index(m_ChannelLayoutOut, AV_CH_TOP_CENTER);
    m_idx_out[AE_CH_TFL]  = av_get_channel_layout_channel_index(m_ChannelLayoutOut, AV_CH_TOP_FRONT_LEFT);
    m_idx_out[AE_CH_TFC]  = av_get_channel_layout_channel_index(m_ChannelLayoutOut, AV_CH_TOP_FRONT_CENTER);
    m_idx_out[AE_CH_TFR]  = av_get_channel_layout_channel_index(m_ChannelLayoutOut, AV_CH_TOP_FRONT_RIGHT);
    m_idx_out[AE_CH_TBL]  = av_get_channel_layout_channel_index(m_ChannelLayoutOut, AV_CH_TOP_BACK_LEFT);
    m_idx_out[AE_CH_TBC]  = av_get_channel_layout_channel_index(m_ChannelLayoutOut, AV_CH_TOP_BACK_CENTER);
    m_idx_out[AE_CH_TBR]  = av_get_channel_layout_channel_index(m_ChannelLayoutOut, AV_CH_TOP_BACK_RIGHT);

    needDSPAddonsReinit = true;
  }

  if (needDSPAddonsReinit)
  {
    m_AddonSettings.lInChannelPresentFlags = 0;
    if (m_idx_in[AE_CH_FL] >= 0)    m_AddonSettings.lInChannelPresentFlags |= AE_DSP_PRSNT_CH_FL;
    if (m_idx_in[AE_CH_FR] >= 0)    m_AddonSettings.lInChannelPresentFlags |= AE_DSP_PRSNT_CH_FR;
    if (m_idx_in[AE_CH_FC] >= 0)    m_AddonSettings.lInChannelPresentFlags |= AE_DSP_PRSNT_CH_FC;
    if (m_idx_in[AE_CH_LFE] >= 0)   m_AddonSettings.lInChannelPresentFlags |= AE_DSP_PRSNT_CH_LFE;
    if (m_idx_in[AE_CH_BL] >= 0)    m_AddonSettings.lInChannelPresentFlags |= AE_DSP_PRSNT_CH_BL;
    if (m_idx_in[AE_CH_BR] >= 0)    m_AddonSettings.lInChannelPresentFlags |= AE_DSP_PRSNT_CH_BR;
    if (m_idx_in[AE_CH_FLOC] >= 0)  m_AddonSettings.lInChannelPresentFlags |= AE_DSP_PRSNT_CH_FLOC;
    if (m_idx_in[AE_CH_FROC] >= 0)  m_AddonSettings.lInChannelPresentFlags |= AE_DSP_PRSNT_CH_FROC;
    if (m_idx_in[AE_CH_BC] >= 0)    m_AddonSettings.lInChannelPresentFlags |= AE_DSP_PRSNT_CH_BC;
    if (m_idx_in[AE_CH_SL] >= 0)    m_AddonSettings.lInChannelPresentFlags |= AE_DSP_PRSNT_CH_SL;
    if (m_idx_in[AE_CH_SR] >= 0)    m_AddonSettings.lInChannelPresentFlags |= AE_DSP_PRSNT_CH_SR;
    if (m_idx_in[AE_CH_TFL] >= 0)   m_AddonSettings.lInChannelPresentFlags |= AE_DSP_PRSNT_CH_TFL;
    if (m_idx_in[AE_CH_TFR] >= 0)   m_AddonSettings.lInChannelPresentFlags |= AE_DSP_PRSNT_CH_TFR;
    if (m_idx_in[AE_CH_TFC] >= 0)   m_AddonSettings.lInChannelPresentFlags |= AE_DSP_PRSNT_CH_TFC;
    if (m_idx_in[AE_CH_TC] >= 0)    m_AddonSettings.lInChannelPresentFlags |= AE_DSP_PRSNT_CH_TC;
    if (m_idx_in[AE_CH_TBL] >= 0)   m_AddonSettings.lInChannelPresentFlags |= AE_DSP_PRSNT_CH_TBL;
    if (m_idx_in[AE_CH_TBR] >= 0)   m_AddonSettings.lInChannelPresentFlags |= AE_DSP_PRSNT_CH_TBR;
    if (m_idx_in[AE_CH_TBC] >= 0)   m_AddonSettings.lInChannelPresentFlags |= AE_DSP_PRSNT_CH_TBC;

    m_AddonSettings.lOutChannelPresentFlags = 0;
    if (m_idx_out[AE_CH_FL] >= 0)   m_AddonSettings.lOutChannelPresentFlags |= AE_DSP_PRSNT_CH_FL;
    if (m_idx_out[AE_CH_FR] >= 0)   m_AddonSettings.lOutChannelPresentFlags |= AE_DSP_PRSNT_CH_FR;
    if (m_idx_out[AE_CH_FC] >= 0)   m_AddonSettings.lOutChannelPresentFlags |= AE_DSP_PRSNT_CH_FC;
    if (m_idx_out[AE_CH_LFE] >= 0)  m_AddonSettings.lOutChannelPresentFlags |= AE_DSP_PRSNT_CH_LFE;
    if (m_idx_out[AE_CH_BL] >= 0)   m_AddonSettings.lOutChannelPresentFlags |= AE_DSP_PRSNT_CH_BL;
    if (m_idx_out[AE_CH_BR] >= 0)   m_AddonSettings.lOutChannelPresentFlags |= AE_DSP_PRSNT_CH_BR;
    if (m_idx_out[AE_CH_FLOC] >= 0) m_AddonSettings.lOutChannelPresentFlags |= AE_DSP_PRSNT_CH_FLOC;
    if (m_idx_out[AE_CH_FROC] >= 0) m_AddonSettings.lOutChannelPresentFlags |= AE_DSP_PRSNT_CH_FROC;
    if (m_idx_out[AE_CH_BC] >= 0)   m_AddonSettings.lOutChannelPresentFlags |= AE_DSP_PRSNT_CH_BC;
    if (m_idx_out[AE_CH_SL] >= 0)   m_AddonSettings.lOutChannelPresentFlags |= AE_DSP_PRSNT_CH_SL;
    if (m_idx_out[AE_CH_SR] >= 0)   m_AddonSettings.lOutChannelPresentFlags |= AE_DSP_PRSNT_CH_SR;
    if (m_idx_out[AE_CH_TFL] >= 0)  m_AddonSettings.lOutChannelPresentFlags |= AE_DSP_PRSNT_CH_TFL;
    if (m_idx_out[AE_CH_TFR] >= 0)  m_AddonSettings.lOutChannelPresentFlags |= AE_DSP_PRSNT_CH_TFR;
    if (m_idx_out[AE_CH_TFC] >= 0)  m_AddonSettings.lOutChannelPresentFlags |= AE_DSP_PRSNT_CH_TFC;
    if (m_idx_out[AE_CH_TC] >= 0)   m_AddonSettings.lOutChannelPresentFlags |= AE_DSP_PRSNT_CH_TC;
    if (m_idx_out[AE_CH_TBL] >= 0)  m_AddonSettings.lOutChannelPresentFlags |= AE_DSP_PRSNT_CH_TBL;
    if (m_idx_out[AE_CH_TBR] >= 0)  m_AddonSettings.lOutChannelPresentFlags |= AE_DSP_PRSNT_CH_TBR;
    if (m_idx_out[AE_CH_TBC] >= 0)  m_AddonSettings.lOutChannelPresentFlags |= AE_DSP_PRSNT_CH_TBC;

    m_AddonSettings.iStreamID           = m_StreamId;
    m_AddonSettings.iInChannels         = in->pkt->config.channels;
    m_AddonSettings.iOutChannels        = out->pkt->config.channels;
    m_AddonSettings.iInSamplerate       = in->pkt->config.sample_rate;
    m_AddonSettings.iProcessSamplerate  = m_Addon_InputResample.pAddon  ? m_Addon_InputResample.pAddon->InputResampleSampleRate(m_StreamId)   : m_AddonSettings.iInSamplerate;
    m_AddonSettings.iOutSamplerate      = m_Addon_OutputResample.pAddon ? m_Addon_OutputResample.pAddon->OutputResampleSampleRate(m_StreamId) : m_AddonSettings.iProcessSamplerate;

    if (m_NewMasterMode >= 0)
    {
      MasterModeChange(m_NewMasterMode, m_NewStreamType);
      m_NewMasterMode = AE_DSP_MASTER_MODE_ID_INVALID;
      m_NewStreamType = AE_DSP_ASTREAM_INVALID;
    }

    for (AE_DSP_ADDONMAP_ITR itr = m_usedMap.begin(); itr != m_usedMap.end(); itr++)
    {
      AE_DSP_ERROR err = itr->second->StreamInitialize(&m_AddonSettings);
      if (err != AE_DSP_ERROR_NO_ERROR)
      {
        CLog::Log(LOGERROR, "ActiveAE DSP - %s - addon initialize failed on %s with %s", __FUNCTION__, itr->second->GetAudioDSPName().c_str(), CActiveAEDSPAddon::ToString(err));
      }
    }

    RecheckProcessArray(frames);
    ClearArray(m_ProcessArray[0], m_ProcessArraySize);
    ClearArray(m_ProcessArray[1], m_ProcessArraySize);

    m_ForceInit         = false;
    m_iLastProcessTime  = XbmcThreads::SystemClockMillis() * 10000;
    m_iLastProcessUsage = 0;
    m_fLastProcessUsage = 0.0f;

    /**
     * Setup ffmpeg convert array for input stream
     */
    SetFFMpegDSPProcessorArray(m_FFMpegConvertArray, m_ProcessArray[0], NULL);
  }

  int64_t startTime;
  float **lastOutArray    = m_ProcessArray[0];
  unsigned int togglePtr  = 1;

  /**
   * Convert to required planar float format inside dsp system
   */
  if (swr_convert(m_ConvertInput, (uint8_t **)m_FFMpegConvertArray[0], m_ProcessArraySize, (const uint8_t **)in->pkt->data , frames) < 0)
  {
    CLog::Log(LOGERROR, "ActiveAE DSP - %s - input audio convert failed", __FUNCTION__);
    return false;
  }

    /**********************************************/
   /** DSP Processing Algorithms following here **/
  /**********************************************/

  /**
   * DSP input processing
   * Can be used to have unchanged input stream..
   * All DSP addons allowed todo this.
   */
  for (unsigned int i = 0; i < m_Addons_InputProc.size(); i++)
  {
    if (!m_Addons_InputProc[i]->InputProcess(m_StreamId, (const float **)lastOutArray, frames))
    {
      CLog::Log(LOGERROR, "ActiveAE DSP - %s - input process failed on addon No. %i", __FUNCTION__, i);
      return false;
    }
  }

  /**
   * DSP resample processing before master
   * Here a high quality resample can be performed.
   * Only one DSP addon is allowed todo this!
   */
  if (m_Addon_InputResample.pAddon)
  {
    startTime = CurrentHostCounter();

    frames = m_Addon_InputResample.pAddon->InputResampleProcess(m_StreamId, lastOutArray, m_ProcessArray[togglePtr], frames);
    if (frames == 0)
      return false;

    m_Addon_InputResample.iLastTime += 1000 * 10000 * (CurrentHostCounter() - startTime) / hostFrequency;

    lastOutArray = m_ProcessArray[togglePtr];
    togglePtr ^= 1;
  }

  /**
   * DSP pre processing
   * All DSP addons allowed todo this and order of it set on settings.
   */
  for (unsigned int i = 0; i < m_Addons_PreProc.size(); i++)
  {
    startTime = CurrentHostCounter();

    frames = m_Addons_PreProc[i].pAddon->PreProcess(m_StreamId, m_Addons_PreProc[i].iAddonModeNumber, lastOutArray, m_ProcessArray[togglePtr], frames);
    if (frames == 0)
      return false;

    m_Addons_PreProc[i].iLastTime += 1000 * 10000 * (CurrentHostCounter() - startTime) / hostFrequency;

    lastOutArray = m_ProcessArray[togglePtr];
    togglePtr ^= 1;
  }

  /**
   * DSP master processing
   * Here a channel upmix/downmix for stereo surround sound can be performed
   * Only one DSP addon is allowed todo this!
   */
  if (m_Addons_MasterProc[m_ActiveMode].pAddon)
  {
    startTime = CurrentHostCounter();

    frames = m_Addons_MasterProc[m_ActiveMode].pAddon->MasterProcess(m_StreamId, lastOutArray, m_ProcessArray[togglePtr], frames);
    if (frames == 0)
      return false;

    m_Addons_MasterProc[m_ActiveMode].iLastTime += 1000 * 10000 * (CurrentHostCounter() - startTime) / hostFrequency;

    lastOutArray = m_ProcessArray[togglePtr];
    togglePtr ^= 1;
  }

  /**
   * Perform fallback channel mixing if input channel alignment is different
   * from output and not becomes processed by active master processing mode or
   * perform ffmpeg related internal master processes.
   */
  if (m_ResamplerDSPProcessor)
  {
    startTime = CurrentHostCounter();

    if (needDSPAddonsReinit)
      SetFFMpegDSPProcessorArray(m_FFMpegProcessArray, lastOutArray, m_ProcessArray[togglePtr]);

    frames = m_ResamplerDSPProcessor->Resample((uint8_t**)m_FFMpegProcessArray[FFMPEG_PROC_ARRAY_OUT], frames, (uint8_t**)m_FFMpegProcessArray[FFMPEG_PROC_ARRAY_IN], frames, 1.0);
    if (frames <= 0)
    {
      CLog::Log(LOGERROR, "CActiveAEResample::Resample - resample failed");
      return false;
    }

    m_Addons_MasterProc[m_ActiveMode].iLastTime += 1000 * 10000 * (CurrentHostCounter() - startTime) / hostFrequency;

    lastOutArray = m_ProcessArray[togglePtr];
    togglePtr ^= 1;
  }

  /**
   * DSP post processing
   * On the post processing can be things performed with additional channel upmix like 6.1 to 7.1
   * or frequency/volume corrections, speaker distance handling, equalizer... .
   * All DSP addons allowed todo this and order of it set on settings.
   */
  for (unsigned int i = 0; i < m_Addons_PostProc.size(); i++)
  {
    startTime = CurrentHostCounter();

    frames = m_Addons_PostProc[i].pAddon->PostProcess(m_StreamId, m_Addons_PostProc[i].iAddonModeNumber, lastOutArray, m_ProcessArray[togglePtr], frames);
    if (frames == 0)
      return false;

    m_Addons_PostProc[i].iLastTime += 1000 * 10000 * (CurrentHostCounter() - startTime) / hostFrequency;

    lastOutArray = m_ProcessArray[togglePtr];
    togglePtr ^= 1;
  }

  /**
   * DSP resample processing behind master
   * Here a high quality resample can be performed.
   * Only one DSP addon is allowed todo this!
   */
  if (m_Addon_OutputResample.pAddon)
  {
    startTime = CurrentHostCounter();

    frames = m_Addon_OutputResample.pAddon->OutputResampleProcess(m_StreamId, lastOutArray, m_ProcessArray[togglePtr], frames);
    if (frames == 0)
      return false;

    m_Addon_OutputResample.iLastTime += 1000 * 10000 * (CurrentHostCounter() - startTime) / hostFrequency;

    lastOutArray = m_ProcessArray[togglePtr];
    togglePtr ^= 1;
  }

  /**
   * Setup ffmpeg convert array for output stream, performed here to now last array
   */
  if (needDSPAddonsReinit)
    SetFFMpegDSPProcessorArray(m_FFMpegConvertArray, NULL, lastOutArray);

  /**
   * Convert back to required output format
   */
  if (swr_convert(m_ConvertOutput, (uint8_t **)out->pkt->data, m_ProcessArraySize, (const uint8_t **)m_FFMpegConvertArray[1], frames) < 0)
  {
    CLog::Log(LOGERROR, "ActiveAE DSP - %s - output audio convert failed", __FUNCTION__);
    return false;
  }
  out->pkt->nb_samples = frames;

  /**
   * Update cpu process percent usage values for modes and total (every second)
   */
  if (iTime >= m_iLastProcessTime + 1000*10000)
    CalculateCPUUsage(iTime);

  return true;
}

bool CActiveAEDSPProcess::RecheckProcessArray(unsigned int inputFrames)
{
  /* Check for big enough array */
  unsigned int framesNeeded;
  unsigned int framesOut = m_ProcessArraySize;

  if (inputFrames > framesOut)
    framesOut = inputFrames;

  if (m_Addon_InputResample.pAddon)
  {
    framesNeeded = m_Addon_InputResample.pAddon->InputResampleProcessNeededSamplesize(m_StreamId);
    if (framesNeeded > framesOut)
      framesOut = framesNeeded;
  }

  for (unsigned int i = 0; i < m_Addons_PreProc.size(); i++)
  {
    framesNeeded = m_Addons_PreProc[i].pAddon->PreProcessNeededSamplesize(m_StreamId, m_Addons_PreProc[i].iAddonModeNumber);
    if (framesNeeded > framesOut)
      framesOut = framesNeeded;
  }

  if (m_Addons_MasterProc[m_ActiveMode].pAddon)
  {
    framesNeeded = m_Addons_MasterProc[m_ActiveMode].pAddon->MasterProcessNeededSamplesize(m_StreamId);
    if (framesNeeded > framesOut)
      framesOut = framesNeeded;
  }

  for (unsigned int i = 0; i < m_Addons_PostProc.size(); i++)
  {
    framesNeeded = m_Addons_PostProc[i].pAddon->PostProcessNeededSamplesize(m_StreamId, m_Addons_PostProc[i].iAddonModeNumber);
    if (framesNeeded > framesOut)
      framesOut = framesNeeded;
  }

  if (m_Addon_OutputResample.pAddon)
  {
    framesNeeded = m_Addon_OutputResample.pAddon->OutputResampleProcessNeededSamplesize(m_StreamId);
    if (framesNeeded > framesOut)
      framesOut = framesNeeded;
  }

  if (framesOut > m_ProcessArraySize)
  {
    if (!ReallocProcessArray(framesOut))
      return false;

    m_ProcessArraySize = framesOut;
  }
  return true;
}

bool CActiveAEDSPProcess::ReallocProcessArray(unsigned int requestSize)
{
  m_ProcessArraySize = requestSize + MIN_DSP_ARRAY_SIZE / 10;
  for (int i = 0; i < AE_DSP_CH_MAX; i++)
  {
    m_ProcessArray[0][i] = (float*)realloc(m_ProcessArray[0][i], m_ProcessArraySize*sizeof(float));
    m_ProcessArray[1][i] = (float*)realloc(m_ProcessArray[1][i], m_ProcessArraySize*sizeof(float));
    if (m_ProcessArray[0][i] == NULL || m_ProcessArray[1][i] == NULL)
    {
      CLog::Log(LOGERROR, "ActiveAE DSP - %s - realloc of process data array failed", __FUNCTION__);
      return false;
    }
  }
  return true;
}

// in this function the usage for each adsp-mode in percent is calculated
void CActiveAEDSPProcess::CalculateCPUUsage(unsigned int iTime)
{
  int64_t iUsage = CThread::GetCurrentThread()->GetAbsoluteUsage();

  if (iTime != m_iLastProcessTime)
  {
    // calculate usage only if we don't divide by zero
    if (m_iLastProcessUsage > 0 && m_iLastProcessTime > 0)
    {
      m_fLastProcessUsage = (float)(iUsage - m_iLastProcessUsage) / (float)(iTime - m_iLastProcessTime) * 100.0f;
    }

    float dTFactor = 100.0f / (float)(iTime - m_iLastProcessTime);

    if(m_Addon_InputResample.pMode)
    {
      m_Addon_InputResample.pMode->SetCPUUsage((float)(m_Addon_InputResample.iLastTime)*dTFactor);
      m_Addon_InputResample.iLastTime = 0;
    }

    for (unsigned int i = 0; i < m_Addons_PreProc.size(); i++)
    {
      m_Addons_PreProc[i].pMode->SetCPUUsage((float)(m_Addons_PreProc[i].iLastTime)*dTFactor);
      m_Addons_PreProc[i].iLastTime = 0;
    }

    if (m_Addons_MasterProc[m_ActiveMode].pMode)
    {
      m_Addons_MasterProc[m_ActiveMode].pMode->SetCPUUsage((float)(m_Addons_MasterProc[m_ActiveMode].iLastTime)*dTFactor);
      m_Addons_MasterProc[m_ActiveMode].iLastTime = 0;
    }

    for (unsigned int i = 0; i < m_Addons_PostProc.size(); i++)
    {
      m_Addons_PostProc[i].pMode->SetCPUUsage((float)(m_Addons_PostProc[i].iLastTime)*dTFactor);
      m_Addons_PostProc[i].iLastTime = 0;
    }

    if (m_Addon_OutputResample.pMode)
    {
      m_Addon_OutputResample.pMode->SetCPUUsage((float)(m_Addon_OutputResample.iLastTime)*dTFactor);
      m_Addon_OutputResample.iLastTime = 0;
    }
  }

  m_iLastProcessUsage = iUsage;
  m_iLastProcessTime  = iTime;
}

void CActiveAEDSPProcess::SetFFMpegDSPProcessorArray(float *array_ffmpeg[2][AE_DSP_CH_MAX], float **array_in, float **array_out)
{
  /*!
   * Setup ffmpeg resampler channel setup, this way is not my favorite but it works to become
   * the correct channel alignment on the given input and output signal.
   * The problem is, the process array of ffmpeg is not fixed and for every selected channel setup
   * the positions are different. For this case a translation from the fixed dsp stream format to
   * ffmpeg format must be performed. It use a separate process array table which becomes set by
   * already present channel memory storage.
   */

  //! Initialize input channel alignmment for ffmpeg process array
  if (array_in)
  {
    if (m_AddonSettings.lInChannelPresentFlags & AE_DSP_PRSNT_CH_FL)
    {
      array_ffmpeg[FFMPEG_PROC_ARRAY_IN][m_idx_in[AE_CH_FL]] = array_in[AE_DSP_CH_FL];
      array_ffmpeg[FFMPEG_PROC_ARRAY_IN][m_idx_in[AE_CH_FR]] = array_in[AE_DSP_CH_FR];
    }
    if (m_AddonSettings.lInChannelPresentFlags & AE_DSP_PRSNT_CH_FC)
    {
      array_ffmpeg[FFMPEG_PROC_ARRAY_IN][m_idx_in[AE_CH_FC]] = array_in[AE_DSP_CH_FC];
    }
    if (m_AddonSettings.lInChannelPresentFlags & AE_DSP_PRSNT_CH_LFE)
    {
      array_ffmpeg[FFMPEG_PROC_ARRAY_IN][m_idx_in[AE_CH_LFE]] = array_in[AE_DSP_CH_LFE];
    }
    if (m_AddonSettings.lInChannelPresentFlags & AE_DSP_PRSNT_CH_BL)
    {
      array_ffmpeg[FFMPEG_PROC_ARRAY_IN][m_idx_in[AE_CH_BL]] = array_in[AE_DSP_CH_BL];
      array_ffmpeg[FFMPEG_PROC_ARRAY_IN][m_idx_in[AE_CH_BR]] = array_in[AE_DSP_CH_BR];
    }
    if (m_AddonSettings.lInChannelPresentFlags & AE_DSP_PRSNT_CH_FLOC)
    {
      array_ffmpeg[FFMPEG_PROC_ARRAY_IN][m_idx_in[AE_CH_FLOC]] = array_in[AE_DSP_CH_FLOC];
      array_ffmpeg[FFMPEG_PROC_ARRAY_IN][m_idx_in[AE_CH_FROC]] = array_in[AE_DSP_CH_FROC];
    }
    if (m_AddonSettings.lInChannelPresentFlags & AE_DSP_PRSNT_CH_BC)
    {
      array_ffmpeg[FFMPEG_PROC_ARRAY_IN][m_idx_in[AE_CH_BC]] = array_in[AE_DSP_CH_BC];
    }
    if (m_AddonSettings.lInChannelPresentFlags & AE_DSP_PRSNT_CH_SL)
    {
      array_ffmpeg[FFMPEG_PROC_ARRAY_IN][m_idx_in[AE_CH_SL]] = array_in[AE_DSP_CH_SL];
      array_ffmpeg[FFMPEG_PROC_ARRAY_IN][m_idx_in[AE_CH_SR]] = array_in[AE_DSP_CH_SR];
    }
    if (m_AddonSettings.lInChannelPresentFlags & AE_DSP_PRSNT_CH_TFL)
    {
      array_ffmpeg[FFMPEG_PROC_ARRAY_IN][m_idx_in[AE_CH_TFL]] = array_in[AE_DSP_CH_TFL];
      array_ffmpeg[FFMPEG_PROC_ARRAY_IN][m_idx_in[AE_CH_TFR]] = array_in[AE_DSP_CH_TFR];
    }
    if (m_AddonSettings.lInChannelPresentFlags & AE_DSP_PRSNT_CH_TFC)
    {
      array_ffmpeg[FFMPEG_PROC_ARRAY_IN][m_idx_in[AE_CH_TFC]] = array_in[AE_DSP_CH_TFC];
    }
    if (m_AddonSettings.lInChannelPresentFlags & AE_DSP_PRSNT_CH_TC)
    {
      array_ffmpeg[FFMPEG_PROC_ARRAY_IN][m_idx_in[AE_CH_TC]] = array_in[AE_DSP_CH_TC];
    }
    if (m_AddonSettings.lInChannelPresentFlags & AE_DSP_PRSNT_CH_TBL)
    {
      array_ffmpeg[FFMPEG_PROC_ARRAY_IN][m_idx_in[AE_CH_TBL]] = array_in[AE_DSP_CH_TBL];
      array_ffmpeg[FFMPEG_PROC_ARRAY_IN][m_idx_in[AE_CH_TBR]] = array_in[AE_DSP_CH_TBR];
    }
    if (m_AddonSettings.lInChannelPresentFlags & AE_DSP_PRSNT_CH_TBC)
    {
      array_ffmpeg[FFMPEG_PROC_ARRAY_IN][m_idx_in[AE_CH_TBC]] = array_in[AE_DSP_CH_TBC];
    }
  }

  if (array_out)
  {
    //! Initialize output channel alignmment for ffmpeg process array
    if (m_AddonSettings.lOutChannelPresentFlags & AE_DSP_PRSNT_CH_FL)
    {
      array_ffmpeg[FFMPEG_PROC_ARRAY_OUT][m_idx_out[AE_CH_FL]] = array_out[AE_DSP_CH_FL];
      array_ffmpeg[FFMPEG_PROC_ARRAY_OUT][m_idx_out[AE_CH_FR]] = array_out[AE_DSP_CH_FR];
    }
    if (m_AddonSettings.lOutChannelPresentFlags & AE_DSP_PRSNT_CH_FC)
    {
      array_ffmpeg[FFMPEG_PROC_ARRAY_OUT][m_idx_out[AE_CH_FC]] = array_out[AE_DSP_CH_FC];
    }
    if (m_AddonSettings.lOutChannelPresentFlags & AE_DSP_PRSNT_CH_LFE)
    {
      array_ffmpeg[FFMPEG_PROC_ARRAY_OUT][m_idx_out[AE_CH_LFE]] = array_out[AE_DSP_CH_LFE];
    }
    if (m_AddonSettings.lOutChannelPresentFlags & AE_DSP_PRSNT_CH_BL)
    {
      array_ffmpeg[FFMPEG_PROC_ARRAY_OUT][m_idx_out[AE_CH_BL]] = array_out[AE_DSP_CH_BL];
      array_ffmpeg[FFMPEG_PROC_ARRAY_OUT][m_idx_out[AE_CH_BR]] = array_out[AE_DSP_CH_BR];
    }
    if (m_AddonSettings.lOutChannelPresentFlags & AE_DSP_PRSNT_CH_FLOC)
    {
      array_ffmpeg[FFMPEG_PROC_ARRAY_OUT][m_idx_out[AE_CH_FLOC]] = array_out[AE_DSP_CH_FLOC];
      array_ffmpeg[FFMPEG_PROC_ARRAY_OUT][m_idx_out[AE_CH_FROC]] = array_out[AE_DSP_CH_FROC];
    }
    if (m_AddonSettings.lOutChannelPresentFlags & AE_DSP_PRSNT_CH_BC)
    {
      array_ffmpeg[FFMPEG_PROC_ARRAY_OUT][m_idx_out[AE_CH_BC]] = array_out[AE_DSP_CH_BC];
    }
    if (m_AddonSettings.lOutChannelPresentFlags & AE_DSP_PRSNT_CH_SL)
    {
      array_ffmpeg[FFMPEG_PROC_ARRAY_OUT][m_idx_out[AE_CH_SL]] = array_out[AE_DSP_CH_SL];
      array_ffmpeg[FFMPEG_PROC_ARRAY_OUT][m_idx_out[AE_CH_SR]] = array_out[AE_DSP_CH_SR];
    }
    if (m_AddonSettings.lOutChannelPresentFlags & AE_DSP_PRSNT_CH_TFL)
    {
      array_ffmpeg[FFMPEG_PROC_ARRAY_OUT][m_idx_out[AE_CH_TFL]] = array_out[AE_DSP_CH_TFL];
      array_ffmpeg[FFMPEG_PROC_ARRAY_OUT][m_idx_out[AE_CH_TFR]] = array_out[AE_DSP_CH_TFR];
    }
    if (m_AddonSettings.lOutChannelPresentFlags & AE_DSP_PRSNT_CH_TFC)
    {
      array_ffmpeg[FFMPEG_PROC_ARRAY_OUT][m_idx_out[AE_CH_TFC]] = array_out[AE_DSP_CH_TFC];
    }
    if (m_AddonSettings.lOutChannelPresentFlags & AE_DSP_PRSNT_CH_TC)
    {
      array_ffmpeg[FFMPEG_PROC_ARRAY_OUT][m_idx_out[AE_CH_TC]] = array_out[AE_DSP_CH_TC];
    }
    if (m_AddonSettings.lOutChannelPresentFlags & AE_DSP_PRSNT_CH_TBL)
    {
      array_ffmpeg[FFMPEG_PROC_ARRAY_OUT][m_idx_out[AE_CH_TBL]] = array_out[AE_DSP_CH_TBL];
      array_ffmpeg[FFMPEG_PROC_ARRAY_OUT][m_idx_out[AE_CH_TBR]] = array_out[AE_DSP_CH_TBR];
    }
    if (m_AddonSettings.lOutChannelPresentFlags & AE_DSP_PRSNT_CH_TBC)
    {
      array_ffmpeg[FFMPEG_PROC_ARRAY_OUT][m_idx_out[AE_CH_TBC]] = array_out[AE_DSP_CH_TBC];
    }
  }
}

float CActiveAEDSPProcess::GetDelay()
{
  float delay = 0.0f;

  CSingleLock lock(m_critSection);

  if (m_Addon_InputResample.pAddon)
    delay += m_Addon_InputResample.pAddon->InputResampleGetDelay(m_StreamId);

  for (unsigned int i = 0; i < m_Addons_PreProc.size(); i++)
    delay += m_Addons_PreProc[i].pAddon->PreProcessGetDelay(m_StreamId, m_Addons_PreProc[i].iAddonModeNumber);

  if (m_Addons_MasterProc[m_ActiveMode].pAddon)
    delay += m_Addons_MasterProc[m_ActiveMode].pAddon->MasterProcessGetDelay(m_StreamId);

  for (unsigned int i = 0; i < m_Addons_PostProc.size(); i++)
    delay += m_Addons_PostProc[i].pAddon->PostProcessGetDelay(m_StreamId, m_Addons_PostProc[i].iAddonModeNumber);

  if (m_Addon_OutputResample.pAddon)
    delay += m_Addon_OutputResample.pAddon->OutputResampleGetDelay(m_StreamId);

  return delay;
}

bool CActiveAEDSPProcess::HasActiveModes(AE_DSP_MODE_TYPE type)
{
  bool bReturn(false);

  CSingleLock lock(m_critSection);

  switch (type)
  {
  case AE_DSP_MODE_TYPE_INPUT_RESAMPLE:
    if (m_Addon_InputResample.pAddon != NULL)
      bReturn = true;
    break;
  case AE_DSP_MODE_TYPE_PRE_PROCESS:
    if (!m_Addons_PreProc.empty())
      bReturn = true;
    break;
  case AE_DSP_MODE_TYPE_MASTER_PROCESS:
    if (!m_Addons_MasterProc.empty())
      bReturn = true;
    break;
  case AE_DSP_MODE_TYPE_POST_PROCESS:
    if (!m_Addons_PostProc.empty())
      bReturn = true;
    break;
  case AE_DSP_MODE_TYPE_OUTPUT_RESAMPLE:
    if (m_Addon_OutputResample.pAddon != NULL)
      bReturn = true;
    break;
  default:
    break;
  };

  return bReturn;
}

void CActiveAEDSPProcess::GetActiveModes(AE_DSP_MODE_TYPE type, std::vector<CActiveAEDSPModePtr> &modes)
{
  CSingleLock lock(m_critSection);

  if (m_Addon_InputResample.pAddon != NULL && (type == AE_DSP_MODE_TYPE_UNDEFINED || type == AE_DSP_MODE_TYPE_INPUT_RESAMPLE))
    modes.push_back(m_Addon_InputResample.pMode);

  if (type == AE_DSP_MODE_TYPE_UNDEFINED || type == AE_DSP_MODE_TYPE_PRE_PROCESS)
    for (unsigned int i = 0; i < m_Addons_PreProc.size(); i++)
      modes.push_back(m_Addons_PreProc[i].pMode);

  if (m_Addons_MasterProc[m_ActiveMode].pAddon != NULL && (type == AE_DSP_MODE_TYPE_UNDEFINED || type == AE_DSP_MODE_TYPE_MASTER_PROCESS))
    modes.push_back(m_Addons_MasterProc[m_ActiveMode].pMode);

  if (type == AE_DSP_MODE_TYPE_UNDEFINED || type == AE_DSP_MODE_TYPE_POST_PROCESS)
    for (unsigned int i = 0; i < m_Addons_PostProc.size(); i++)
      modes.push_back(m_Addons_PostProc[i].pMode);

  if (m_Addon_OutputResample.pAddon != NULL && (type == AE_DSP_MODE_TYPE_UNDEFINED || type == AE_DSP_MODE_TYPE_OUTPUT_RESAMPLE))
    modes.push_back(m_Addon_OutputResample.pMode);
}

void CActiveAEDSPProcess::GetAvailableMasterModes(AE_DSP_STREAMTYPE streamType, std::vector<CActiveAEDSPModePtr> &modes)
{
  CSingleLock lock(m_critSection);

  for (unsigned int i = 0; i < m_Addons_MasterProc.size(); i++)
  {
    if (m_Addons_MasterProc[i].pMode->SupportStreamType(streamType))
      modes.push_back(m_Addons_MasterProc[i].pMode);
  }
}

int CActiveAEDSPProcess::GetActiveMasterModeID()
{
  CSingleLock lock(m_critSection);

  return m_ActiveMode < 0 ? AE_DSP_MASTER_MODE_ID_INVALID : m_Addons_MasterProc[m_ActiveMode].pMode->ModeID();
}

CActiveAEDSPModePtr CActiveAEDSPProcess::GetActiveMasterMode() const
{
  CSingleLock lock(m_critSection);

  CActiveAEDSPModePtr mode;

  if (m_ActiveMode < 0)
    return mode;

  mode = m_Addons_MasterProc[m_ActiveMode].pMode;
  return mode;
}

bool CActiveAEDSPProcess::SetMasterMode(AE_DSP_STREAMTYPE streamType, int iModeID, bool bSwitchStreamType)
{
  /*!
   * if the unique master mode id is already used a reinit is not needed
   */
  if (m_Addons_MasterProc[m_ActiveMode].pMode->ModeID() == iModeID && !bSwitchStreamType)
    return true;

  CSingleLock lock(m_restartSection);

  m_NewMasterMode = iModeID;
  m_NewStreamType = bSwitchStreamType ? streamType : AE_DSP_ASTREAM_INVALID;
  m_ForceInit     = true;
  return true;
}

bool CActiveAEDSPProcess::IsMenuHookModeActive(AE_DSP_MENUHOOK_CAT category, int iAddonId, unsigned int iModeNumber)
{
  std::vector <sDSPProcessHandle> *addons = NULL;

  switch (category)
  {
    case AE_DSP_MENUHOOK_MASTER_PROCESS:
      addons = &m_Addons_MasterProc;
      break;
    case AE_DSP_MENUHOOK_PRE_PROCESS:
      addons = &m_Addons_PreProc;
      break;
    case AE_DSP_MENUHOOK_POST_PROCESS:
      addons = &m_Addons_PostProc;
      break;
    case AE_DSP_MENUHOOK_RESAMPLE:
      {
        if (m_Addon_InputResample.iAddonModeNumber > 0 &&
            m_Addon_InputResample.pMode &&
            m_Addon_InputResample.pMode->AddonID() == iAddonId &&
            m_Addon_InputResample.pMode->AddonModeNumber() == iModeNumber)
          return true;

        if (m_Addon_OutputResample.iAddonModeNumber > 0 &&
            m_Addon_OutputResample.pMode &&
            m_Addon_OutputResample.pMode->AddonID() == iAddonId &&
            m_Addon_OutputResample.pMode->AddonModeNumber() == iModeNumber)
          return true;
      }
    default:
      break;
  }

  if (addons)
  {
    for (unsigned int i = 0; i < addons->size(); i++)
    {
      if (addons->at(i).iAddonModeNumber > 0 &&
          addons->at(i).pMode->AddonID() == iAddonId &&
          addons->at(i).pMode->AddonModeNumber() == iModeNumber)
        return true;
    }
  }
  return false;
}
