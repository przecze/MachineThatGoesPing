#pragma once
#include <defines.h>

#ifdef TEST
  #include <hardware_mock.h>
#else
  #include <hardware.h>
#endif

#ifdef TEST
  #include <cmath>
  #ifndef M_PI
      #define M_PI 3.14159265358979323846
void sei(){}
void cli(){}
  #endif
#else
  #include <math.h>
  #include <avr/interrupt.h>
#endif

class Sound
{
 public:
  //unique_array_ptr<short> data;
  double frequency; // in Hz
  double amplitude; // in V
  double scale_factor;
  Sound * child;

  Sound& add( Sound * sound )
  {
    child = sound;
    return *sound;
  }

  Sound(double frequency, double amplitude = 1.):
    frequency(frequency), amplitude(amplitude), child(nullptr)
  {
    scale_factor = frequency * TIME_RES_US/(1000000.) *2*M_PI;
  }

  double localAmplitude(int x) const
  {
    auto ret = amplitude*cos(scale_factor *x);
    if (child != nullptr) ret+= child->localAmplitude(x);
    return ret;
  }
};

class Signal
{
 public:
  unsigned short data[SIGNAL_LENGTH];
  unsigned short datatwo[SIGNAL_LENGTH];
  int phase_i = 0;
  volatile int position = 0;
  volatile unsigned short * current;
  double max_amp = 1;
  double min_amp = -1;
  Sound sound;
  volatile bool prepared;

  Signal(): sound(0.,0.), prepared(false){}
  Signal(Sound sound): sound(sound), prepared(false)
  {
    current = datatwo;
    prepare(); //data is prepared
    swap_table(); //current changed to data, prepared to false
  }

  inline bool needs_prepare()
  {
    return !prepared;
  }

  void swap_table()
  {
    position = 0;
    if (current == data)
    {
      current = datatwo;
    }
    else
    {
      current = data;
    }
    prepared = false;
  }

  void prepare()
  {
    phase_i+=SIGNAL_LENGTH;
    unsigned short * pnew;
    if (current == data) pnew = datatwo;
    else pnew = data;

    for (int i = 0; i < SIGNAL_LENGTH; ++i)
    {
      auto amp = sound.localAmplitude(phase_i+i);
      auto tamp = (amp - min_amp)/(max_amp - min_amp);
      short norm_amp = short(tamp * ANALOG_RANGE + 0.5);
      pnew[i] = norm_amp;
    }

    prepared = true;
  }

  void swap_if_reached_end()
  {
    if(position==SIGNAL_LENGTH)
    {
      HW::toggle_led_if(100);
      swap_table();
    }
  }
  inline unsigned short next()
  {
      return current[position];
  }
};
