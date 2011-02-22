#ifndef TIMESYNC_H
#define TIMESYNC_H

#include <stdint.h>

/**

	\file timesync.h
	\brief time synchronization library

	The purpose of this library is to keep an inaccurate time base
	(as an MCU timer) in sync with jittery time stamp broadcasts
	from an accurate time base.

	The implementation is based on adding or "removing" (flagging,
	then skipping the next tick) additional "sync" ticks (in the
	library code simply referred to as "ticks").

	The amount of ticks to be added is tracked as the distance
	between ticks and its remainder. Hence, the maximum deviation
	ideally never exceeds 1 ms (the unit of time).

	To "iron out" the jitter on time stamps, a number (SLOT_COUNT)
	of time stamps are recorded and the mean value of them is used
	to calculate the distance between ticks.

*/


#define SLOT_LEN_MS 1000
#define SLOT_COUNT  120
#define TIME_MAX_MS 60000 // the modulus value of the time counter
#define AVG_PERIOD  (((int32_t) SLOT_LEN_MS) * SLOT_COUNT)
#define MSCOUNT_MAX UINT16_MAX
#define I_FACTOR 100

typedef uint16_t mscount_t;
typedef int16_t filter_t;
typedef int32_t mssum_t; ///< SLOT_COUNT * ~ 100 must fit here
typedef uint8_t filterindex_t; ///< SLOT_COUNT must fit here


/**

	\brief Initialize time synchronization

	Resets all variables used.

*/
void ts_init();

/**

	\brief Check for a tick to be added or "removed"

	Should be called periodically, and the appropriate action
	(which is returned) should be taken care of.
	Also keeps track of the current tick remainder.

	Care must be taken when removing a tick - some applications
	might not "like it" if time goes backward. A proposed solution
	is to set a flag to ignore the next periodic clock increment.

	\param[in] ms millisecond counter (current clock value)

	\return 1 to add a tick, -1 to "remove" a tick, 0 otherwise

*/
int8_t ts_tick(mscount_t ms);

/**

	\brief Handle a synchronization time stamp

	Updates the current mean value of drift with a new sync time
	stamp.

	Must be called far less often than the amount of actual ticks
	occuring, otherwise precision might be lost (because distance
	between ticks is recalculated)

	\param[in] ms millisecond counter (current clock value)
	\param[in] corr_ms corrected millisecond counter (where the ticks are added or "removed")
	\param[in] real_ms milliseconds from the synchronization time stamp

*/
void ts_slot(mscount_t ms, mscount_t corr_ms, mscount_t real_ms);


#endif /* TIMESYNC_H */

