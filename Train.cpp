#include "includes.hpp"

Train::Train(const char _train_id[],
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
		    //  bool _deleted,
		     int _create_time):
             station_num(_station_num),
             seat_num(_seat_num),
             type(_type),
            //  deleted(_deleted),
             create_time(_create_time) {
    strcpy(train_id, _train_id);
    for (int i = 0; i < STATION_NUM_MAX; ++i) {
        strcpy(stations[i], _stations[i]);
        prices[i] = _prices[i];
        travel_times[i] = _travel_times[i];
        stopover_times[i] = _stopover_times[i];
    }
    start_time = _start_time;
    sale_date_begin = _sale_date_begin;
    sale_date_end = _sale_date_end;
}

/*Train::Train( const Train &other ) {
}*/

Train::~Train() {}

/*Train &Train::operator=( const User &other ) {
}*/