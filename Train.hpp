#ifndef TRAIN_HPP
#define TRAIN_HPP

#include "const_variable.hpp"
#include "Date.hpp"
#include "Time.hpp"

class Train {
public:
	char train_id[TRAIN_ID_LEN];
	int station_num;
	int seat_num;
	char stations[STATION_NUM_MAX][STATION_LEN];
	int prices[STATION_NUM_MAX];
	Time start_time;
	int travel_times[STATION_NUM_MAX];
	int stopover_times[STATION_NUM_MAX];
	Date sale_date_begin, sale_date_end;
	char type;
	// bool deleted;
	int create_time;

	Train(const char _train_id[],
		  int _station_num,
		  int _seat_num,
		  const char _stations[][STATION_LEN],
		  int _prices[],
		  Time _start_time,
		  int _travel_times[],
		  int _stopover_times[],
		  Date _sale_date_begin,
		  Date _sale_date_end,
		  char _type,
		//   bool _deleted,
		  int _create_time);
	Train() {}
	// Train(const Train &other);
	~Train();
	// Train &operator=(const Train &other);
};

#endif // TRAIN_HPP
