
/*
 * Measurements that are reported back are generally in the display
 * representation.  So when doing arbitrary readings you have to
 * know both the units and the scale but for simplicity we will
 * track both in the units member for mm_t data variables.  Note that
 * in the conversion routines below, many overflow conditions are
 * not inherently handled so make sure you allocate appropriate data
 * types as needed.
 */
#define MUNITS_AC                       0x000001
#define MUNITS_DC                       0x000002
#define MUNITS_AMPS                     0x000004
#define MUNITS_VOLTS                    0x000008
#define MUNITS_CELCIUS                  0x000010
#define MUNITS_FAHRENHEIT               0x000020
#define MUNITS_DECIBELS                 0x000040
#define MUNITS_OHMS                     0x000080
#define MUNITS_HZ                       0x000100
#define MUNITS_JOULES                   0x000200
#define MUNITS_WATTS                    0x000400
#define MUNITS_POWERFACTOR              0x000800
#define MUNITS_NANO                     0x010000
#define MUNITS_MICRO                    0x020000
#define MUNITS_MILLI                    0x040000
#define MUNITS_MEGA                     0x080000
#define MUNITS_GIGA                     0x100000

#define NUM_NIDQ_CHANNELS	16
#define NUM_ACTIVE_NIDQ_CHANNELS	16

//extern unsigned int nidaq_volts[NUM_NIDQ_CHANNELS];
extern unsigned int nidaq_milli_powers[NUM_NIDQ_CHANNELS];
extern HANDLE hMutex;
extern int running;

int gettimeofday(struct timeval *tv, struct timezone *tz);
int start_nidaq_meter();

