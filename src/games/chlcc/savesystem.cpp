#include "savesystem.h"

#include <time.h>
#include "../../io/physicalfilestream.h"
#include "../../mem.h"
#include "../../vm/vm.h"
#include "../../profile/savesystem.h"
#include "../../profile/scriptvars.h"

namespace Impacto {
namespace CHLCC {

using namespace Impacto::Vm;
using namespace Impacto::Profile::SaveSystem;
using namespace Impacto::Profile::ScriptVars;

SaveError SaveSystem::MountSaveFile() {
  Io::InputStream* stream;
  IoError err = Io::PhysicalFileStream::Create(SaveFilePath, &stream);
  switch (err) {
    case IoError_NotFound:
      return SaveNotFound;
    case IoError_Fail:
      return SaveCorrupted;
  };

  stream->Seek(0x3b06, SEEK_SET);  // TODO: Actually load system data

  for (int i = 0; i < MaxSaveEntries; i++) {
    QuickSaveEntries[i] = new SaveFileEntry();

    QuickSaveEntries[i]->Status = Io::ReadLE<uint8_t>(stream);
    QuickSaveEntries[i]->Checksum = Io::ReadLE<uint16_t>(stream);
    Io::ReadLE<uint8_t>(stream);
    uint8_t saveMonth = Io::ReadLE<uint8_t>(stream);
    uint8_t saveDay = Io::ReadLE<uint8_t>(stream);
    uint8_t saveHour = Io::ReadLE<uint8_t>(stream);
    uint8_t saveMinute = Io::ReadLE<uint8_t>(stream);
    uint8_t saveYear = Io::ReadLE<uint8_t>(stream);
    uint8_t saveSecond = Io::ReadLE<uint8_t>(stream);
    QuickSaveEntries[i]->SaveDate =
        std::tm{saveSecond, saveMinute,    saveHour,
                saveDay,    saveMonth - 1, saveYear + 100};
    Io::ReadLE<uint16_t>(stream);
    QuickSaveEntries[i]->PlayTime = Io::ReadLE<uint32_t>(stream);
    QuickSaveEntries[i]->SwTitle = Io::ReadLE<uint16_t>(stream);
    Io::ReadLE<uint8_t>(stream);
    stream->Seek(31, SEEK_CUR);
    Io::ReadArrayLE<uint8_t>(
        ((SaveFileEntry*)QuickSaveEntries[i])->FlagWorkScript1,
                             stream, 50);
    Io::ReadArrayLE<uint8_t>(
        ((SaveFileEntry*)QuickSaveEntries[i])->FlagWorkScript2,
                             stream, 100);
    Io::ReadArrayBE<int>(((SaveFileEntry*)QuickSaveEntries[i])->ScrWorkScript1,
                         stream, 300);
    Io::ReadArrayBE<int>(((SaveFileEntry*)QuickSaveEntries[i])->ScrWorkScript2,
                         stream, 1300);
    QuickSaveEntries[i]->MainThreadExecPriority = Io::ReadBE<uint32_t>(stream);
    QuickSaveEntries[i]->MainThreadGroupId = Io::ReadBE<uint32_t>(stream);
    QuickSaveEntries[i]->MainThreadWaitCounter = Io::ReadBE<uint32_t>(stream);
    QuickSaveEntries[i]->MainThreadScriptParam = Io::ReadBE<uint32_t>(stream);
    QuickSaveEntries[i]->MainThreadIp = Io::ReadBE<uint32_t>(stream);
    QuickSaveEntries[i]->MainThreadLoopCounter = Io::ReadBE<uint32_t>(stream);
    QuickSaveEntries[i]->MainThreadLoopAdr = Io::ReadBE<uint32_t>(stream);
    QuickSaveEntries[i]->MainThreadCallStackDepth =
        Io::ReadBE<uint32_t>(stream);
    for (int j = 0; j < 8; j++) {
      QuickSaveEntries[i]->MainThreadReturnAddresses[j] =
          Io::ReadBE<uint32_t>(stream);
      QuickSaveEntries[i]->MainThreadReturnBufIds[j] =
          Io::ReadBE<uint32_t>(stream);
    }
    Io::ReadArrayBE<int>(QuickSaveEntries[i]->MainThreadVariables, stream, 16);
    QuickSaveEntries[i]->MainThreadDialoguePageId =
        Io::ReadBE<uint32_t>(stream);
    stream->Seek(1428, SEEK_CUR);
  }

  for (int i = 0; i < MaxSaveEntries; i++) {
    FullSaveEntries[i] = new SaveFileEntry();

    FullSaveEntries[i]->Status = Io::ReadLE<uint8_t>(stream);
    FullSaveEntries[i]->Checksum = Io::ReadLE<uint16_t>(stream);
    Io::ReadLE<uint8_t>(stream);
    uint8_t saveMonth = Io::ReadLE<uint8_t>(stream);
    uint8_t saveDay = Io::ReadLE<uint8_t>(stream);
    uint8_t saveHour = Io::ReadLE<uint8_t>(stream);
    uint8_t saveMinute = Io::ReadLE<uint8_t>(stream);
    uint8_t saveYear = Io::ReadLE<uint8_t>(stream);
    uint8_t saveSecond = Io::ReadLE<uint8_t>(stream);
    FullSaveEntries[i]->SaveDate =
        std::tm{saveSecond, saveMinute,    saveHour,
                saveDay,    saveMonth - 1, saveYear + 100};
    Io::ReadLE<uint16_t>(stream);
    FullSaveEntries[i]->PlayTime = Io::ReadLE<uint32_t>(stream);
    FullSaveEntries[i]->SwTitle = Io::ReadLE<uint16_t>(stream);
    Io::ReadLE<uint8_t>(stream);
    stream->Seek(31, SEEK_CUR);
    Io::ReadArrayLE<uint8_t>(
        ((SaveFileEntry*)FullSaveEntries[i])->FlagWorkScript1,
                             stream, 50);
    Io::ReadArrayLE<uint8_t>(
        ((SaveFileEntry*)FullSaveEntries[i])->FlagWorkScript2,
                             stream, 100);
    Io::ReadArrayBE<int>(((SaveFileEntry*)FullSaveEntries[i])->ScrWorkScript1,
                         stream, 300);
    Io::ReadArrayBE<int>(((SaveFileEntry*)FullSaveEntries[i])->ScrWorkScript2,
                         stream, 1300);
    FullSaveEntries[i]->MainThreadExecPriority = Io::ReadBE<uint32_t>(stream);
    FullSaveEntries[i]->MainThreadGroupId = Io::ReadBE<uint32_t>(stream);
    FullSaveEntries[i]->MainThreadWaitCounter = Io::ReadBE<uint32_t>(stream);
    FullSaveEntries[i]->MainThreadScriptParam = Io::ReadBE<uint32_t>(stream);
    FullSaveEntries[i]->MainThreadIp = Io::ReadBE<uint32_t>(stream);
    FullSaveEntries[i]->MainThreadLoopCounter = Io::ReadBE<uint32_t>(stream);
    FullSaveEntries[i]->MainThreadLoopAdr = Io::ReadBE<uint32_t>(stream);
    FullSaveEntries[i]->MainThreadCallStackDepth = Io::ReadBE<uint32_t>(stream);
    for (int j = 0; j < 8; j++) {
      FullSaveEntries[i]->MainThreadReturnAddresses[j] =
          Io::ReadBE<uint32_t>(stream);
      FullSaveEntries[i]->MainThreadReturnBufIds[j] =
          Io::ReadBE<uint32_t>(stream);
    }
    Io::ReadArrayBE<int>(FullSaveEntries[i]->MainThreadVariables, stream, 16);
    FullSaveEntries[i]->MainThreadDialoguePageId = Io::ReadBE<uint32_t>(stream);
    stream->Seek(1428, SEEK_CUR);
  }

  return SaveOK;
}

void SaveSystem::SaveMemory(SaveType type, int id) {}

void SaveSystem::LoadMemory(SaveType type, int id) {
  SaveFileEntry* entry = 0;
  switch (type) {
    case SaveQuick:
      entry = (SaveFileEntry*)QuickSaveEntries[id];
      break;
    case SaveFull:
      entry = (SaveFileEntry*)FullSaveEntries[id];
      break;
  }

  if (entry != 0)
    if (entry->Status) {
      memcpy(&FlagWork[50], entry->FlagWorkScript1, 50);
      memcpy(&FlagWork[300], entry->FlagWorkScript2, 100);
      memcpy(&ScrWork[300], entry->ScrWorkScript1, 1200);
      memcpy(&ScrWork[2300], entry->ScrWorkScript2, 5200);

      // TODO: What to do about this mess I wonder...
      ScrWork[2001] = ScrWork[2311];
      ScrWork[2002] = ScrWork[2312];
      ScrWork[2003] = ScrWork[2313];
      ScrWork[2005] = ScrWork[2310];
      ScrWork[2006] = ScrWork[2322];
      ScrWork[2007] = ScrWork[2323];
      ScrWork[2008] = ScrWork[2324];
      ScrWork[2009] = ScrWork[2325];
      ScrWork[2010] = ScrWork[2407];
      ScrWork[2011] = ScrWork[2427];
      ScrWork[2012] = ScrWork[2447];
      ScrWork[2013] = ScrWork[2467];
      ScrWork[2014] = ScrWork[2487];
      ScrWork[2015] = ScrWork[2507];
      ScrWork[2016] = ScrWork[2527];
      ScrWork[2017] = ScrWork[2547];
      ScrWork[2018] = ScrWork[2609];
      ScrWork[2019] = ScrWork[2629];
      ScrWork[2020] = ScrWork[2649];
      ScrWork[2021] = ScrWork[2669];
      ScrWork[2022] = ScrWork[2689];
      ScrWork[2023] = ScrWork[2709];
      ScrWork[2024] = ScrWork[2729];
      ScrWork[2025] = ScrWork[2749];
      ScrWork[2026] = ScrWork[2769];
      ScrWork[2027] = ScrWork[2789];
      ScrWork[2028] = ScrWork[2809];
      ScrWork[2029] = ScrWork[2829];
      ScrWork[2030] = ScrWork[2849];
      ScrWork[2031] = ScrWork[2869];
      ScrWork[2032] = ScrWork[2889];
      ScrWork[2033] = ScrWork[2909];
      ScrWork[2034] = ScrWork[3200];
      ScrWork[2035] = ScrWork[3201];
      ScrWork[2036] = ScrWork[3202];
      ScrWork[2037] = ScrWork[3203];
      ScrWork[2038] = ScrWork[3204];
      ScrWork[2039] = ScrWork[3205];
      ScrWork[2040] = ScrWork[3206];
      ScrWork[2041] = ScrWork[3207];
      ScrWork[2042] = ScrWork[3208];
      ScrWork[2043] = ScrWork[3209];
      ScrWork[2044] = ScrWork[3210];
      ScrWork[2045] = ScrWork[3211];
      ScrWork[2046] = ScrWork[3212];
      ScrWork[2047] = ScrWork[3213];
      ScrWork[2048] = ScrWork[3214];
      ScrWork[2049] = ScrWork[3215];
      ScrWork[2050] = ScrWork[3216];
      ScrWork[2051] = ScrWork[3220];
      ScrWork[2052] = ScrWork[3221];
      ScrWork[2053] = ScrWork[3222];
      ScrWork[2054] = ScrWork[3223];
      ScrWork[2055] = ScrWork[3224];
      ScrWork[2056] = ScrWork[3225];
      ScrWork[2057] = ScrWork[3226];
      ScrWork[2058] = ScrWork[3227];
      ScrWork[2059] = ScrWork[3228];
      ScrWork[2060] = ScrWork[3229];
      ScrWork[2061] = ScrWork[3230];
      ScrWork[2062] = ScrWork[3231];
      ScrWork[2063] = ScrWork[3232];
      ScrWork[2064] = ScrWork[3233];
      ScrWork[2065] = ScrWork[3234];
      ScrWork[2066] = ScrWork[3235];
      ScrWork[2067] = ScrWork[3236];

      int threadId = ScrWork[SW_MAINTHDP];
      Sc3VmThread* thd = &ThreadPool[threadId & 0x7FFFFFFF];
      if (thd != 0 &&
          (thd->GroupId == 4 || thd->GroupId == 5 || thd->GroupId == 6)) {
        thd->ExecPriority = entry->MainThreadExecPriority;
        thd->WaitCounter = entry->MainThreadWaitCounter;
        thd->ScriptParam = entry->MainThreadScriptParam;
        thd->GroupId = entry->MainThreadGroupId >> 16;
        thd->ScriptBufferId = entry->MainThreadGroupId & 0xFFFF;
        if (ScriptBuffers[thd->ScriptBufferId] == 0) {
          LoadScript(thd->ScriptBufferId, ScrWork[2004 + thd->ScriptBufferId]);
        }
        thd->Ip = ScriptBuffers[thd->ScriptBufferId] + entry->MainThreadIp;

        if (ScriptBuffers[entry->MainThreadReturnBufIds[0]] == 0) {
          LoadScript(entry->MainThreadReturnBufIds[0],
                     ScrWork[2004 + entry->MainThreadReturnBufIds[0]]);
        }
        thd->CallStackDepth++;
        thd->ReturnScriptBufferIds[0] = entry->MainThreadReturnBufIds[0];
        thd->ReturnAdresses[0] =
            ScriptBuffers[entry->MainThreadReturnBufIds[0]] +
            entry->MainThreadReturnAddresses[0];

        memcpy(thd->Variables, entry->MainThreadVariables, 64);
        thd->DialoguePageId = entry->MainThreadDialoguePageId;
      }
    }
}

}  // namespace CHLCC
}  // namespace Impacto