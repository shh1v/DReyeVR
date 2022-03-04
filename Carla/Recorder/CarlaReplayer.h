// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>

#include <functional>
#include "CarlaRecorderInfo.h"
#include "CarlaRecorderFrames.h"
#include "CarlaRecorderEventAdd.h"
#include "CarlaRecorderEventDel.h"
#include "CarlaRecorderEventParent.h"
#include "CarlaRecorderCollision.h"
#include "CarlaRecorderPosition.h"
#include "CarlaRecorderState.h"
#include "CarlaRecorderHelpers.h"
#include "CarlaReplayerHelper.h"

class UCarlaEpisode;

class CarlaReplayer
{
  #pragma pack(push, 1)
  struct Header
  {
    char Id;
    uint32_t Size;
  };
  #pragma pack(pop)

public:
  struct PlayAfterLoadMap
  {
    bool Enabled;
    std::string Filename;
    FString Mapfile;
    double TimeStart;
    double Duration;
    uint32_t FollowId;
    double TimeFactor;
    bool ReplaySensors;
  };

  static PlayAfterLoadMap Autoplay;

  CarlaReplayer() {};
  ~CarlaReplayer() { Stop(); };

  std::string ReplayFile(std::string Filename, double TimeStart = 0.0f, double Duration = 0.0f,
      uint32_t FollowId = 0, bool ReplaySensors = false);

  // void Start(void);
  void Stop(bool KeepActors = false);

  void Enable(void);

  void Disable(void);

  bool IsEnabled(void)
  {
    return Enabled;
  }

  // set episode
  void SetEpisode(UCarlaEpisode *ThisEpisode)
  {
    Episode = ThisEpisode;
    Helper.SetEpisode(ThisEpisode);
  }

  // playback speed (time factor)
  void SetTimeFactor(double NewTimeFactor)
  {
    TimeFactor = NewTimeFactor;
  }

  // set ignore hero
  void SetIgnoreHero(bool InIgnoreHero)
  {
    IgnoreHero = InIgnoreHero;
  }

  // check if after a map is loaded, we need to replay
  void CheckPlayAfterMapLoaded(void);

  // tick for the replayer
  void Tick(float Time);

  // DReyeVR replayer functions
  void PlayPause();
  void Restart();
  void Advance(const float Amnt);
  void IncrTimeFactor(const float Amnt_s);

  void SetSyncMode(bool bSyncModeIn)
  {
    bReplaySync = bSyncModeIn;
    if (bReplaySync)
      UE_LOG(LogTemp, Warning, TEXT("Replay operating in frame-wise sync mode"));
  }
  
private:

  bool Enabled;
  bool bReplaySensors = false;
  bool Paused = false;
  UCarlaEpisode *Episode = nullptr;
  // binary file reader
  std::ifstream File;
  Header Header;
  CarlaRecorderInfo RecInfo;
  CarlaRecorderFrame Frame;
  // positions (to be able to interpolate)
  std::vector<CarlaRecorderPosition> CurrPos;
  std::vector<CarlaRecorderPosition> PrevPos;
  // mapping id
  std::unordered_map<uint32_t, uint32_t> MappedId;
  // times
  double CurrentTime;
  double TimeToStop;
  double TotalTime;
  // helper
  CarlaReplayerHelper Helper;
  // follow camera
  uint32_t FollowId;
  // speed (time factor)
  double TimeFactor { 1.0 };
  // ignore hero vehicles
  bool IgnoreHero { false };
  std::unordered_map<uint32_t, bool> IsHeroMap;

  // utils
  bool ReadHeader();

  void SkipPacket();

  double GetTotalTime(void);

  void Rewind(void);

  // processing packets
  void ProcessToTime(double Time, bool IsFirstTime = false);

  void ProcessEventsAdd(void);
  void ProcessEventsDel(void);
  void ProcessEventsParent(void);

  void ProcessPositions(bool IsFirstTime = false);

  void ProcessStates(void);

  void ProcessAnimVehicle(void);
  void ProcessAnimWalker(void);

  void ProcessLightVehicle(void);
  void ProcessLightScene(void);

  // DReyeVR recordings
  void ProcessDReyeVRData(void);

  // For restarting the recording with the same params
  struct LastReplayStruct
  {
    std::string Filename = "None";
    double TimeStart = 0;
    double Duration = 0;
    uint32_t ThisFollowId = 0;
  };
  LastReplayStruct LastReplay;

  // DReyeVR sensor data
  DReyeVRDataRecorder DReyeVRDataInstance;
  void UpdateDReyeVRSensor(double Per, double DeltaTime);

  bool bReplaySync = false;
  std::vector<double> FrameStartTimes;
  size_t SyncCurrentFrameId = 0;
  void GetFrameStartTimes();
  void ProcessFrameByFrame();

  // positions
  void UpdatePositions(double Per, double DeltaTime);

  void InterpolatePosition(const CarlaRecorderPosition &Start, const CarlaRecorderPosition &End, double Per, double DeltaTime);
};