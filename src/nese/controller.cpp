#include "controller.h"

StandardController::StandardController() = default;

void StandardController::WriteStrobe(bool active)
{
  if (m_strobe && !active)
    ReloadShiftRegister();

  m_strobe = active;
}

uint8 StandardController::ReadData()
{
  uint8 value = uint8(m_shift_register & 0x01);
  m_shift_register >>= 1;

  // If the strobe is set, reload the shift register.
  // Otherwise, set the high bit, so subsequent reads return 1.
  if (m_strobe)
    ReloadShiftRegister();
  else
    m_shift_register |= 0x100;

  return value;
}

void StandardController::ReloadShiftRegister()
{
  m_shift_register = 0;
  for (uint8 i = 0; i < NUM_BUTTONS; i++)
  {
    if (m_button_states[i])
      m_shift_register |= (1 << i);
  }
}
