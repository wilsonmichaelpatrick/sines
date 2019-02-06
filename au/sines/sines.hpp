#ifndef B31E_HPP
#define B31E_HPP

#include "IPlug_include_in_plug_hdr.h"
#include "IMidiQueue.h"

class sines : public IPlug
{
public:
  sines(IPlugInstanceInfo instanceInfo);
  ~sines();
  
  void Reset();
  void OnParamChange(int paramIdx);
  void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames);
  void ProcessMidiMsg(IMidiMsg* pMsg);
  
private:
  void advance();
  IMidiQueue mMidiQueue;
  struct sines_state* mB31EState;
  int mOffset;
  int m_bass_note_number;
  float m_amplitude_decay_constant;
  float m_trigger_delay_ms;
};

#endif

