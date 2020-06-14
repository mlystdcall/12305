#ifndef RELEASED_TRAIN_CONTROLLER_HPP
#define RELEASED_TRAIN_CONTROLLER_HPP

#include <fstream>
#include <iostream>
#include "const_variable.hpp"
#include "Date.hpp"
#include "Train.hpp"
#include "BTree.hpp"

class Interface;

class ReleasedTrainController {
	Interface *itf;
	enum {SUM = 100};

	struct Char{
		char str[USERNAME_LEN];
		Char() {
			;
		}
		Char(const char username[]) {
			strcpy(str, username);
		}
		void copy(char username[]) const{
			strcpy(username, str);
		}
	};

public:
	std::fstream ticket_file;
	std::fstream que_btree_file;
	std::fstream que_info_file;
	BTree<std::tuple<std::pair<int, int>, Date, int>, std::pair<Char, int> > btree;
	CachedFileOperator<SUM * SUM> file_operator;

	void release_train( const char train_id[] );
	void query_train( const char train_id[], Date date );
	void modify_ticket( const Train &train,
						Date date,
						const char from[],
						const char to[],
						int num ); // positive for increasing and vice versa
	void add_order( const char train_id[],
					Date date,
					const char username[],
					int order_id );
	void delete_order( const char train_id[],
					   Date date,
					   const char username[],
					   int order_id,
					   int buy_time = -1 );
	void adjust_order( const char train_id[],
					   Date date );
	int query_ticket( const Train &train,
					  Date date,
					  const char from[],
					  const char to[] );
	
	void load(Interface *interface);
	void save();
};

#endif // RELEASED_TRAIN_CONTROLLER_HPP
