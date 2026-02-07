typedef unsigned int uint;

#define _TEST_2_ 111
#ifdef _TEST_
int b = 1;
#if _TEST_2_ >= 111
int c = 1;
#else
int d = 1;
#endif
int e = 1;
#endif

#if _TEST_2_ < 1
int f = 1;
#elif _TEST_2_ >= 112
int g = 1;
#else
int h = 1;
#endif

uint8_t
eight_qubit_bell_state ()
{
  qubit q[8];
  int a = _TEST_;
  apply_H (q[0]);
  for (int i = 0; i < 7; i++)
    {
      apply_CX (q[i], q[i + 1]);
      apply_RX (q[1], PI);
      i = 1;
    }

  uint8_t meas = measure (q[10]);

  if (meas == 1)
    {
      meas = measure (q);
    }
  else
    {
      if (meas == 2)
        {
          meas = measure (q);
        }
      meas = measure (q);
    }

  // Expected to return 0b00000000 == 0 in 50% and 0b11111111 == 255 in 50%.
  return meas;
}
