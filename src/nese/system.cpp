#include "system.h"
#include "apu.h"
#include "bus.h"
#include "cartridge.h"
#include "controller.h"
#include "cpu.h"
#include "ppu.h"

#include "common/bitutils.h"
#include "common/error.h"
#include "common/log.h"
#include "common/md5_digest.h"

#include "retro2/retro2_core_exports.h"
#include "retro2/retro2_error.h"
#include "retro2/retro2_filesystem.h"
#include "retro2/retro2_framebuffer.h"
#include "retro2/retro2_session.h"
#include "retro2/retro2_settings.h"

#include <cstring>

IRetro2Error R2Error;
IRetro2Settings R2Settings;
IRetro2FileSystem R2FileSystem;
IRetro2Session R2Session;
IRetro2FramebufferVideo R2Video;

System::System()
  : m_bus(std::make_unique<Bus>()), m_cpu(std::make_unique<CPU>()), m_ppu(std::make_unique<PPU>()),
    m_apu(std::make_unique<APU>())
{
}

System::~System() = default;

void System::SetCartridge(std::unique_ptr<Cartridge> cartridge)
{
  m_cartridge = std::move(cartridge);
  m_bus->SetCartridge(m_cartridge.get());
}

void System::SetController(u32 index, std::unique_ptr<Controller> controller)
{
  m_controllers[index] = std::move(controller);
  m_bus->SetController(index, m_controllers[index].get());
}

void System::Initialize(std::unique_ptr<Cartridge> cartridge)
{
  m_bus->Initialize(m_cpu.get(), m_ppu.get(), m_apu.get());
  m_cpu->Initialize(this, m_bus.get());
  m_ppu->Initialize(this, m_bus.get());
  m_apu->Initialize(m_bus.get());
  SetCartridge(std::move(cartridge));
}

void System::Reset()
{
  m_bus->Reset();
  m_cartridge->Reset();
  m_ppu->Reset();
  m_apu->Reset();
  m_cpu->Reset();
  m_frame_number = 1;
}

void System::SingleStep()
{
  m_cpu->Execute(1);
  m_bus->ExecutePendingCycles();
}

void System::FrameStep()
{
  const u32 prev_frame_number = m_frame_number;
  while (m_frame_number == prev_frame_number)
  {
    const u32 cpu_cycles = std::min(m_ppu->GetMaxExecutionDelay(), m_apu->GetMaxExecutionDelay());
    m_cpu->Execute(cpu_cycles);
    // m_cpu->Execute(1);
    m_bus->ExecutePendingCycles();
  }
}

void System::EndFrame()
{
  m_frame_number++;

#if 0
  static Timer tmr;
  static u32 lf;
  static u32 cc;
  if (tmr.GetTimeSeconds() > 1.0f)
  {
    u32 f = m_frame_number - lf;
    std::fprintf(stderr, "%u frames in %f seconds (%f fps), %u cycles\n", f, tmr.GetTimeSeconds(), f / tmr.GetTimeSeconds(), m_cpu->GetCyclesSinceReset() - cc);
    tmr.Reset();
    lf = m_frame_number;
    cc = m_cpu->GetCyclesSinceReset();
  }
#endif
}

#define SYSTEM static_cast<System*>(userdata)

static void R2CoreSetControllerInputValue(void* userdata, uint32_t controller_index, uint32_t input_index, float value)
{
  StandardController* const controller = (controller_index < System::NUM_CONTROLLERS) ?
                                           static_cast<StandardController*>(SYSTEM->GetController(controller_index)) :
                                           nullptr;
  if (!controller)
    return;

  const bool down = (value > 0.5f);
  switch (input_index)
  {
    case RETRO2_GAMEPAD_INPUT_ID_DPAD_UP:
      controller->SetButtonState(StandardController::Button_Up, down);
      break;

    case RETRO2_GAMEPAD_INPUT_ID_DPAD_DOWN:
      controller->SetButtonState(StandardController::Button_Down, down);
      break;

    case RETRO2_GAMEPAD_INPUT_ID_DPAD_LEFT:
      controller->SetButtonState(StandardController::Button_Left, down);
      break;

    case RETRO2_GAMEPAD_INPUT_ID_DPAD_RIGHT:
      controller->SetButtonState(StandardController::Button_Right, down);
      break;

    case RETRO2_GAMEPAD_INPUT_ID_EAST:
      controller->SetButtonState(StandardController::Button_A, down);
      break;

    case RETRO2_GAMEPAD_INPUT_ID_SOUTH:
      controller->SetButtonState(StandardController::Button_B, down);
      break;

    case RETRO2_GAMEPAD_INPUT_ID_START:
      controller->SetButtonState(StandardController::Button_Start, down);
      break;

    case RETRO2_GAMEPAD_INPUT_ID_BACK:
      controller->SetButtonState(StandardController::Button_Select, down);
      break;

    default:
      break;
  }
}

static void R2CoreReset(void* userdata)
{
  SYSTEM->Reset();
}

static void R2CoreShutdown(void* userdata)
{
  delete SYSTEM;
}

static void R2CoreRunFrame(void* userdata)
{
  SYSTEM->FrameStep();
}

static std::array<u8, RETRO2_ACHIEVEMENTS_HASH_SIZE> R2CoreGetAchievementsHash(const Cartridge* cart)
{
  MD5Digest md5;
  md5.Update(cart->GetPRGROM());
  md5.Update(cart->GetCHRROM());

  std::array<u8, RETRO2_ACHIEVEMENTS_HASH_SIZE> digest;
  md5.Final(digest);
  return digest;
}

static uint32_t R2CoreReadAchievementsMemory(void* userdata, uint32_t address, uint8_t* buffer, uint32_t num_bytes)
{
  // slack, not safe at all
  for (u32 i = 0; i < num_bytes; i++)
    *(buffer++) = SYSTEM->GetBus()->ReadCPUAddress(Truncate16(address++));

  return num_bytes;
}

static uint32_t R2CoreWriteAchievementsMemory(void* userdata, uint32_t address, const uint8_t* buffer,
                                              uint32_t num_bytes)
{
  for (u32 i = 0; i < num_bytes; i++)
    SYSTEM->GetBus()->WriteCPUAddress(Truncate16(address++), *(buffer++));

  return num_bytes;
}

static size_t R2CoreGetMaxStateBufferSize(void*)
{
  return 1 * 1024 * 1024;
}

#if 0
static bool R2CoreLoadStateFromBuffer(void* userdata, const uint8_t* buffer, size_t buffer_size, Retro2Error* error)
{
  Error merror;
  if (SYSTEM->LoadState(std::span<const u8>(buffer, buffer_size), &merror))
    return true;

  R2Error.SetErrorString(error, merror.GetDescription().c_str());
  return false;
}

static bool R2CoreSaveStateToBuffer(void* userdata, uint8_t* buffer, size_t buffer_size, size_t* written_size,
                                    Retro2Error* error)
{
  Error merror;
  if (SYSTEM->SaveState(std::span<u8>(buffer, buffer_size), written_size, &merror))
    return true;

  R2Error.SetErrorString(error, merror.GetDescription().c_str());
  return false;
}
#endif

static bool R2CoreSessionProviderStartSession(const Retro2StartSessionParams* params, IRetro2CoreSession* core_session,
                                              RETRO2_SYSTEM_TYPE* system_type, RETRO2_SESSION_RUN_MODE* run_mode,
                                              void** session_userdata, Retro2Error* error)
{
  if (!params->GamePath)
  {
    R2Error.SetErrorString(error, "Game path is required.");
    return false;
  }

  R2Session = *params->ISession;

  Error merror;
  std::unique_ptr<Cartridge> cart = Cartridge::Load(params->GamePath, &merror);
  if (!cart)
  {
    R2Error.SetErrorString(error, merror.GetDescription().c_str());
    return false;
  }

  std::unique_ptr<System> system = std::make_unique<System>();
  system->Initialize(std::move(cart));
  system->SetController(0, std::make_unique<StandardController>());
  system->Reset();

  if (!R2Session.GetVideoInterface(RETRO2_VIDEO_INTERFACE_FRAMEBUFFER, nullptr, 0, &R2Video, sizeof(R2Video), error))
    return false;

  if (R2Session.UpdateAchievementsGameHash)
  {
    std::array<u8, RETRO2_ACHIEVEMENTS_HASH_SIZE> achievements_hash = R2CoreGetAchievementsHash(system->GetCartridge());
    R2Session.UpdateAchievementsGameHash(achievements_hash.data());
  }

  core_session->Shutdown = R2CoreShutdown;
  core_session->Reset = R2CoreReset;
  core_session->SetControllerInputValue = R2CoreSetControllerInputValue;
  core_session->RunFrame = R2CoreRunFrame;
  core_session->GetMaxStateBufferSize = R2CoreGetMaxStateBufferSize;
  // core_session->LoadStateFromBuffer = R2CoreLoadStateFromBuffer;
  // core_session->SaveStateToBuffer = R2CoreSaveStateToBuffer;
  core_session->ReadAchievementsMemory = R2CoreReadAchievementsMemory;
  core_session->WriteAchievementsMemory = R2CoreWriteAchievementsMemory;

  R2Session.SetControllerType(0, RETRO2_CONTROLLER_TYPE_GAMEPAD, RETRO2_GAMEPAD_FLAG_NO_ANALOGS);
  R2Session.SetVideoFrameRate(3579545.45 / 227.333 / 262); // TODO: PAL
  R2Session.SetVideoDisplayAspectRatio(4.0f / 3.0f);
  R2Session.SetVideoDisplayGeometry(PPU::SCREEN_WIDTH, PPU::SCREEN_HEIGHT, 0, 0, PPU::SCREEN_WIDTH, PPU::SCREEN_HEIGHT,
                                    static_cast<float>(PPU::SCREEN_WIDTH) / static_cast<float>(PPU::SCREEN_HEIGHT));

  *run_mode = RETRO2_SESSION_RUN_MODE_FRAME;
  *system_type = RETRO2_SYSTEM_TYPE_NES;
  *session_userdata = system.release();
  return true;
}

bool Retro2GetCoreSessionProvider(IRetro2CoreSessionProvider* prov, Retro2Error* error)
{
  static constexpr IRetro2CoreSessionProvider our_prov = {
    .Initialize = nullptr,
    .Shutdown = nullptr,
    .StartSession = R2CoreSessionProviderStartSession,
  };

  std::memcpy(prov, &our_prov, sizeof(our_prov));
  return true;
}

bool Retro2CoreInit(const Retro2CoreInitParams* params, Retro2CoreInitInfo* info, Retro2Error* error)
{
  R2Error = *params->IError;
  R2Settings = *params->ISettings;
  R2FileSystem = *params->IFileSystem;
  Log::Initialize(params->ILog);

  info->CoreName = "r2nes";
  info->MinimumSaveStateVersion = 0;
  info->MaximumSaveStateVersion = 0;

  return true;
}

void Retro2CoreShutdown()
{
}
