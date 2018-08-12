#pragma once
#include "types.h"

class Controller
{
public:
  virtual void WriteStrobe(bool active) = 0;
  virtual uint8 ReadData() = 0;
};

class StandardController final : public Controller
{
public:
  static const uint8 NUM_BUTTONS = 8;
  enum : uint8
  {
    Button_A,
    Button_B,
    Button_Select,
    Button_Start,
    Button_Up,
    Button_Down,
    Button_Left,
    Button_Right
  };

  StandardController();

  void WriteStrobe(bool active) override final;
  uint8 ReadData() override final;

  bool GetButtonState(uint8 button) const { return m_button_states[button]; }
  void SetButtonState(uint8 button, bool state) { m_button_states[button] = state; }

private:
  void ReloadShiftRegister();

  uint16 m_shift_register = 0;
  bool m_button_states[NUM_BUTTONS] = {};
  bool m_strobe = false;
};
