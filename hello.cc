// Program zaświeca pojedynczą diodę LED
// Autor: Przemysław Czechowski

#include <avr/io.h>
#include <util/delay.h>
#include <math.h>

#define TIME_STEP_MS 100
template <class T>
class unique_ptr
{
 public:

  unique_ptr() : payload(nullptr) {}

  unique_ptr(T * t): payload(t) {}

  T * get() { return payload; }

  T * reset(T * new_ptr = nullptr)
  {
    T * old_ptr = payload;
    payload = new_ptr;
    if(old_ptr != nullptr) delete old_ptr;
  }

  T * release(T * t = nullptr)
  {
    T* tmp = payload;
    payload = t;
    return t;
  }

  bool isNull() { return payload == nullptr;}

  ~unique_ptr(){ delete payload; }

 private:
  T * payload;
};

class Sound
{
 public:
  int * data;
  bool prepared;
  int length;
  double amplitude; // in V
  double frequency; // in Hz
  double start; // in ms
  double end; // in ms
  unique_ptr<Sound> child; // when the destructor is called, also all the children will be destroyed

  Sound& add( Sound* sound )
  {
    child = unique_ptr<Sound>{sound};
    return *child.get();
  }

  Sound(int length, double amplitude, double duration):
    prepared(false), length(length), amplitude(amplitude), start(0.), end(duration)
  {
  }

  bool prepare()
  {
    return false; //not implemented
  }

  ~Sound()
  {
    if(prepared)
    {
      for(int i  = 0; i< length; ++i)
      {
        delete &(data[i]);
      }
    }
  }
};

//---------------------------------------------------------------------------
void configure_pins()
{
    DDRB |= (1 << DDB1)|(1 << DDB2);
    // PB1 and PB2 is now an output

    ICR1 = (1 << 3); //sets top 16, update every 1 us

    OCR1A = 0x3FFF;
    // set PWM for 25% duty cycle @ 16bit

    OCR1B = 0xBFFF;
    // set PWM for 75% duty cycle @ 16bit

    TCCR1A |= (1 << COM1A1)|(1 << COM1B1);
    // set none-inverting mode

    TCCR1A |= (1 << WGM11);
    TCCR1B |= (1 << WGM12)|(1 << WGM13);
    // set Fast PWM mode using ICR1 as TOP
    
    TCCR1B |= (1 << CS10);
    // START the timer with no prescaler

}


void play(int * signal)
{
  for(int i=0; i < 32; i+= 1)
  {
    OCR1A = signal[i];
    OCR1B = signal[i];
    _delay_us(100);
  }

}

void short_flash()
{
    _delay_ms(140);
    PORTB |= 1 << PB5; // Włączam diodę.
    _delay_ms(20);
    PORTB &= ~(1 << PB5);
    _delay_ms(140);
}
void long_flash()
{
    _delay_ms(50);
    PORTB |= 1 << PB5; // Włączam diodę.
    _delay_ms(200);
    PORTB &= ~(1 << PB5);
    _delay_ms(50);
}

template<int delay>
void play_sound(long long int count = 500000)
{
  for(long long int i = 0; i<count; i+=2*delay)
  {
    PORTD |= 1 << PD5; // Włączam diodę.
    PORTB |= 1 << PD2; // Włączam diodę.
    PORTB |= 1 << PB5; // Włączam diodę.
    _delay_us(delay);
    PORTD &= ~(1 << PD5);
    PORTB &= ~(1 << PB2);
    PORTB &= ~(1 << PB5);
    _delay_us(delay);
  }
}

int normalize(double max, double value)
{
  return int(16*((value + max)/max) + 0.5);
}


//---------------------------------------------------------------------------
int main()
{
  configure_pins();
  //int signal[32] =  {0,  0,  1,  1,  2,  4,  5,  7,  8, 10, 12, 13, 14, 15, 16, 16, 16, 16, 15, 14, 13, 12, 10,  8,  7,  5,  4,  2,  1,  1,  0,  0};
  int sig2[32];
  for( int i = 0; i<32; i++ )
  {
    //sig2[i] = normalize(2, cos(-M_PI + (M_2_PI*i)/32) + cos(-M_PI + (M_2_PI*i)/48));
    sig2[i] = normalize(1, cos(-M_PI + (M_2_PI*i)/32));
  }
  while ( true )
  {
    play(sig2);
  }

  return 0;
}
//---------------------------------------------------------------------------