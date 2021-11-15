#include <avr/io.h>
#include <avr/interrupt.h>

#define SIG_IN A9
#define SIG_OUT 1

IntervalTimer mainInterval; // software interrupt for Real time operation

/* volatile, enable Direct Memory Access
   (memory is available directly to registers)
*/
volatile long long ticker = 0;
volatile double offSet = 0;
volatile float diff = 0;
volatile float tickTime;
volatile bool Q1;
volatile bool SLEW = true;
volatile int flipTick, tickAve, OutTick = 0, tickWidth;
volatile int fliptime = 0;
volatile int pwmtime = 0;

int  dif = 0;

void setup() {

  mainInterval.priority(1); // allow nothing to delay the timed function, will execute exactly every (microsec)

  pinMode(SIG_IN, INPUT);
  pinMode(SIG_OUT, OUTPUT);
  //pinMode(A3, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(250000);

  mainInterval.begin(PLL, 1); // run PLL() function every 1 microsecond(s)

}

void loop() {

  while (1) // eliminate scan() functions, effectively escape the loop() function to avoid interrupt delay
  {
    //          Serial.print(offSet); // debug only
    //          Serial.print(',');
    //          Serial.println(dif);

    delay(10); // uses different clock than interrupts, uses no system resources.
  }
}

void PLL() // timed function, ececutes 10^6 times per second
{
  bool Q2 = Q1;
  Q1 = digitalReadFast(SIG_IN); // read signal from CAB 920 (high Z voltage div.)

  if ((Q1 != Q2) && (Q1 == true)) // if rising edge
  {
    int flippedTick = flipTick; // compute average input signal period (robust to short time jitter)
    flipTick = ticker;
    tickWidth = flipTick - flippedTick;
    if (SLEW == true)
    {
      tickTime = (.99 * tickTime) + (.01 * tickWidth);
    }
    else
    {
      tickTime = (.999 * tickTime) + (.001 * tickWidth);
    }
  }

  if (fliptime >= (int)(tickTime + offSet)) // output square wave
  {

    fliptime = offSet ; //shift output in phase if not in phase with input av.
    OutTick = ticker;

    if (abs(tickTime - tickWidth) < 40)
    {
      dif = (flipTick - (OutTick - (int)(tickTime / 2))); // hack to avoid zero crossing
      diff = 0.05 * (float)(dif) + 0.95 * diff;

      SLEW = false;

      offSet += (0.1 * diff);
      digitalWriteFast(LED_BUILTIN, HIGH);
    }

    if (abs(tickTime - tickWidth) > 65)
    {
      SLEW = true;
      digitalWriteFast(LED_BUILTIN, LOW);
      offSet = 0;
      dif = 0;
    }

    digitalWriteFast(SIG_OUT, LOW);

  }
  if (fliptime >= (int)((tickTime * .5) + offSet))
  {
    digitalWriteFast(SIG_OUT, HIGH);
  }

  ticker++;
  fliptime++;
  pwmtime++;
}
