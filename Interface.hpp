#ifndef INTERFACE_HPP
#define INTERFACE_HPP

#include "UserController.hpp"
#include "OrderController.hpp"
#include "ReleasedTrainController.hpp"
#include "TrainController.hpp"
#include "TicketController.hpp"
#include "Time.hpp"
#include "Date.hpp"

class Interface {
public:
	void start();
	void run();
	void exit();
	
	UserController user_controller;
	OrderController order_controller;
	ReleasedTrainController released_train_controller;
	TrainController train_controller_unreleased;
	TrainController train_controller_released;
	TicketController ticket_controller;
	
	void create_file();
	void add_user( std::string cmd );
	void login( std::string cmd );
	void logout( std::string cmd );
	void query_profile( std::string cmd );
	void modify_profile( std::string cmd );
	void add_train( std::string cmd );
	void release_train( std::string cmd );
	void query_train( std::string cmd );
	void delete_train( std::string cmd );
	void query_ticket( std::string cmd );
	void query_transfer( std::string cmd );
	void buy_ticket( std::string cmd );
	void query_order( std::string cmd );
	void refund_ticket( std::string cmd );
	void clean();
	int split( std::string str, char ch, std::string *&ans );
	int read_num( std::string str );
	Time read_time( std::string str );
	Date read_date( std::string str );
};

#endif // INTERFACE_HPP
