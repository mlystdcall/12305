#ifndef TRAIN_CONTROLLER_HPP
#define TRAIN_CONTROLLER_HPP

#include <fstream>
#include <iostream>
#include "const_variable.hpp"
#include "Time.hpp"
#include "Date.hpp"
#include "Train.hpp"
#include "BTree.hpp"

class Interface;

class TrainController {
public:
	int train_cnt;
	std::fstream info_file; // updated
	std::fstream btree_file; // updated
	Interface *interface;
	BTree<std::pair<int, int>, Train> btree;
	
	void add_train( const char train_id[],
					int station_num,
					int seat_num,
					const char stations[][STATION_LEN],
					int prices[],
					Time start_time,
					int travel_times[],
					int stopover_times[],
					Date sale_date_begin,
					Date sale_date_end,
					char type );
	void delete_train( const char train_id[] );
	
	void load( Interface *ifs, const char *id_filename, const char *info_filename );
	// pos == 8(unreleased train) or 12(released train)
	void save( int pos );
};

#endif // TRAIN_CONTROLLER_HPP
