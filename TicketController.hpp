#ifndef TICKET_CONTROLLER_HPP
#define TICKET_CONTROLLER_HPP

#include <fstream>
#include <string>
#include <iostream>
#include "const_variable.hpp"
#include "Time.hpp"
#include "Hash.hpp"
#include "BTree.hpp"

class Interface;

class TicketController {
	Interface *itf;
	int get_all( const Train &train, const char from[], const char to[], Time &leaving_time, Time &arriving_time );
public:
	std::fstream btree_file;
	std::fstream info_file;
	
	struct Char{
		char str[TRAIN_ID_LEN];
		Char() {
			;
		}
		Char(const char train_id[]) {
			strcpy(str, train_id);
		}
		void copy(char train_id[]) const{
			strcpy(train_id, str);
		}
		bool operator < (const Char &b) const{
			return Hash().hash(str) < Hash().hash(b.str);	
		}
		bool operator == (const Char &b) const{
			return Hash().hash(str) == Hash().hash(b.str);
		}
	};

	BTree<std::pair<std::pair<int, int>, std::pair<int, int> >, std::pair<Char,int> > btree;
	
	int query_pass( const char station[], std::pair<Char, int> *&result );
	void query_ticket( const char from[],
					   const char to[],
					   Date date,
					   const char priority[] );
	void query_transfer( const char from[],
						 const char to[],
						 Date date,
						 const char priority[] );
	void buy_ticket( const char username[],
					 const char train_id[],
					 Date date,
					 const char from[],
					 const char to[],
					 int num,
					 bool que );
	void refund_ticket( const char username[],
						int order_id );
	
	void load( Interface *interface );
	void save();
};

#endif // TICKET_CONTROLLER_HPP
