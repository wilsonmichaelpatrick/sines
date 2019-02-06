#include "sines.hpp"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "resource.h"
#include <sys/time.h>
extern "C"
{
#include "../../sines.h"
}

const int kNumPrograms = 1;

enum EParams
{
  param_amplitude_decay_constant,
  param_trigger_delay_ms,
  num_params
};

enum ELayout
{
  kWidth = GUI_WIDTH,
  kHeight = GUI_HEIGHT,
  
  kGainX = 100,
  kGainY = 100,
  kKnobFrames = 60
};

sines::sines(IPlugInstanceInfo instanceInfo)
:  IPLUG_CTOR(num_params, kNumPrograms, instanceInfo), mOffset(0),
m_bass_note_number(0), m_amplitude_decay_constant(0), m_trigger_delay_ms(0)
{
  TRACE;
#define TODO123 0
  //arguments are: name, defaultVal, minVal, maxVal, step, label
  
  mB31EState = sines_alloc(mSampleRate);
  
  if(NULL != mB31EState) {
    GetParam(param_amplitude_decay_constant)->InitDouble
    ("Amplitude decay constant", m_amplitude_decay_constant,
     SINES_AMPLITUDE_DECAY_MIN, SINES_AMPLITUDE_DECAY_MAX, 0.01, "", "Bass");
    GetParam(param_trigger_delay_ms)->InitDouble
    ("Trigger delay ms", m_trigger_delay_ms, 0, 100, 0.1, "", "Trigger delay ms");
    MakeDefaultPreset((char *) "-", kNumPrograms);
  }
  else {
    sines_free(mB31EState);
    mB31EState = NULL;
  }
  
  // TODO We should get some actual presets in here!  MakePreset("preset 1", ... );
  
  /*
   IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
   pGraphics->AttachPanelBackground(&COLOR_RED);
   
   IBitmap knob = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, kKnobFrames);
   
   pGraphics->AttachControl(new IKnobMultiControl(this, kGainX, kGainY, kGain, &knob));
   
   AttachGraphics(pGraphics);
   */
  
  //MakePreset("preset 1", ... );
}

sines::~sines() {}

void sines::advance() {
  while (!mMidiQueue.Empty()) {
    IMidiMsg* midiMessage = mMidiQueue.Peek();
    if (midiMessage->mOffset > mOffset) break;
    
    IMidiMsg::EStatusMsg status = midiMessage->StatusMsg();
    int noteNumber = midiMessage->NoteNumber();
    int velocity = midiMessage->Velocity();
    // There are only note on/off messages in the queue, see ::OnMessageReceived
    if (status == IMidiMsg::kNoteOff && m_bass_note_number == noteNumber) {
      sines_mute_bass(mB31EState);
    }
    else if (status == IMidiMsg::kNoteOn && velocity) {
      m_bass_note_number = noteNumber;
      // http://www.martin-finke.de/blog/articles/audio-plugins-009-receiving-midi
      float freq = 440.0 * pow(2.0, (m_bass_note_number - 69.0) / 12.0);
      sines_trigger_bass(mB31EState, freq, (velocity / 127.0),
                         m_amplitude_decay_constant, m_trigger_delay_ms / 1000.0);
    }
    mMidiQueue.Remove();
  }
  mOffset++;
}

void sines::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
  // Mutex is already locked for us.
  double *leftOutput = outputs[0];
  double *rightOutput = outputs[1];
  
  for (int i = 0; i < nFrames; ++i) {
    advance();
    leftOutput[i] = rightOutput[i] = sines_get_next_sample(mB31EState);
  }
}

void sines::ProcessMidiMsg(IMidiMsg* pMsg) {
  IMidiMsg::EStatusMsg status = pMsg->StatusMsg();
  // We're only interested in Note On/Off messages (not CC, pitch, etc.)
  if(status == IMidiMsg::kNoteOn || status == IMidiMsg::kNoteOff) {
    mMidiQueue.Add(pMsg);
  }
}

void sines::Reset()
{
  TRACE;
  IMutexLock lock(this);
  sines_free(mB31EState);
  mB31EState = sines_alloc(mSampleRate);
}

void sines::OnParamChange(int paramIdx)
{
  IMutexLock lock(this);
  switch (paramIdx)
  {
    case param_amplitude_decay_constant:
      m_amplitude_decay_constant = GetParam(param_amplitude_decay_constant)->Value();
      break;
    case param_trigger_delay_ms:
      m_trigger_delay_ms = GetParam(param_trigger_delay_ms)->Value();
      break;
    default:
      break;
  }
}

