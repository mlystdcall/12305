#include "includes.hpp"

void Interface :: create_file() {
	FileOperator().create_file("user_btree");
	FileOperator().create_file("user_info");
	FileOperator().create_file("order_btree");
	FileOperator().create_file("order_info");
	FileOperator().create_file("que_btree");
	FileOperator().create_file("que_info");
	FileOperator().create_file("train_info_unreleased");
	FileOperator().create_file("train_id_unreleased");
	FileOperator().create_file("train_info_released");
	FileOperator().create_file("train_id_released");
	FileOperator().create_file("ticket_info");
	FileOperator().create_file("station_btree");
	FileOperator().create_file("station_info");
	FileOperator().create_file("counts");
	std::fstream counts("counts");
	if( FileOperator().end_pos(counts) == 0 ) {
		int tmp[4] = {0};
		FileOperator().write( counts, 0, tmp, 4 );
	}
	counts.close();

}

void Interface :: start() {
	create_file();
	user_controller.load(this);
	order_controller.load(this);
	released_train_controller.load(this);
	train_controller_unreleased.load(this, "train_id_unreleased", "train_info_unreleased");
	train_controller_released.load(this, "train_id_released", "train_info_released");
	ticket_controller.load(this);
}

void Interface :: run() {
	using std::string;
	using std::cin;
	
	string cmd;
	
	while(1) {
		while( isspace( cin.peek() ) )
			cin.ignore();
		getline(cin, cmd);
		
		if( cmd.substr(0, 8) == "add_user" ) {
			//std::cerr<<"this is add_user\n";
			add_user(cmd);
		} else if( cmd.substr(0, 5) == "login" ) {
			//std::cerr<<"this is login\n";
			login(cmd);
		} else if( cmd.substr(0, 6) == "logout" ) {
			logout(cmd);
		} else if( cmd.substr(0, 13) == "query_profile" ) {
			query_profile(cmd);
		} else if( cmd.substr(0, 14) == "modify_profile" ) {
			modify_profile(cmd);
		} else if( cmd.substr(0, 9) == "add_train" ) {
			add_train(cmd);
		} else if( cmd.substr(0, 13) == "release_train" ) {
			release_train(cmd);
		} else if( cmd.substr(0, 11) == "query_train" ) {
			query_train(cmd);
		} else if( cmd.substr(0, 12) == "delete_train" ) {
			delete_train(cmd);
		} else if( cmd.substr(0, 12) == "query_ticket" ) {
			query_ticket(cmd);
		} else if( cmd.substr(0, 14) == "query_transfer" ) {
			query_transfer(cmd);
		} else if( cmd.substr(0, 10) == "buy_ticket" ) {
			buy_ticket(cmd);
		} else if( cmd.substr(0, 11) == "query_order" ) {
			query_order(cmd);
		} else if( cmd.substr(0, 13) == "refund_ticket" ) {
			refund_ticket(cmd);
		} else if( cmd.substr(0, 5) == "clean" ) {
			clean();
		} else if( cmd.substr(0, 4) == "exit" ) {
			exit();
		} else {
			assert(0); // wtf
		}
	}
}

void Interface :: exit() {
	user_controller.save();
	order_controller.save();
	released_train_controller.save();
	train_controller_unreleased.save(2 * sizeof(int));
	train_controller_released.save(3 * sizeof(int));
	ticket_controller.save();
	puts("bye");
	::exit(0);
}

void Interface :: add_user( std::string cmd ) {
	std::string *argv = nullptr;
	int argc = split(cmd, ' ', argv);
	
	std::string cur_username;
	std::string username;
	std::string password;
	std::string name;
	std::string mail_addr;
	int privilege;
	
	for( int i = 1; i < argc; i += 2 ) {
		if( argv[i] == "-c" ) {
			cur_username = argv[i+1];
		} else if( argv[i] == "-u" ) {
			username = argv[i+1];
		} else if( argv[i] == "-p" ) {
			password = argv[i+1];
		} else if( argv[i] == "-n" ) {
			name = argv[i+1];
		} else if( argv[i] == "-m" ) {
			mail_addr = argv[i+1];
		} else if( argv[i] == "-g" ) {
			privilege = read_num( argv[i+1] );
		} else {
			assert(0); // wtf
		}
	}
	
	user_controller.add_user( cur_username.c_str(),
							  username.c_str(),
							  password.c_str(),
							  name.c_str(),
							  mail_addr.c_str(),
							  privilege );
	
	delete []argv;
}

void Interface :: login( std::string cmd ) {
	std::string *argv = nullptr;
	int argc = split(cmd, ' ', argv);
	
	std::string username;
	std::string password;
	
	for( int i = 1; i < argc; i += 2 ) {
		if( argv[i] == "-u" ) {
			username = argv[i+1];
		} else if( argv[i] == "-p" ) {
			password = argv[i+1];
		} else {
			assert(0); // wtf
		}
	}
	
	user_controller.login( username.c_str(),
						   password.c_str() );
	
	delete []argv;
}

void Interface :: logout( std::string cmd ) {
	std::string *argv = nullptr;
	int argc = split(cmd, ' ', argv);
	
	std::string username;
	
	for( int i = 1; i < argc; i += 2 ) {
		if( argv[i] == "-u" ) {
			username = argv[i+1];
		} else {
			assert(0); // wtf
		}
	}
	
	user_controller.logout( username.c_str() );
	
	delete []argv;
}

void Interface :: query_profile( std::string cmd ) {
	std::string *argv = nullptr;
	int argc = split(cmd, ' ', argv);
	
	std::string cur_username;
	std::string username;
	
	for( int i = 1; i < argc; i += 2 ) {
		if( argv[i] == "-c" ) {
			cur_username = argv[i+1];
		} else if( argv[i] == "-u" ) {
			username = argv[i+1];
		} else {
			assert(0); // wtf
		}
	}
	
	user_controller.query_profile( cur_username.c_str(),
								   username.c_str() );
	
	delete []argv;
}

void Interface :: modify_profile( std::string cmd ) {
	std::string *argv = nullptr;
	int argc = split(cmd, ' ', argv);
	
	std::string cur_username;
	std::string username;
	std::string password;
	std::string name;
	std::string mail_addr;
	int privilege = -1;
	
	for( int i = 1; i < argc; i += 2 ) {
		if( argv[i] == "-c" ) {
			cur_username = argv[i+1];
		} else if( argv[i] == "-u" ) {
			username = argv[i+1];
		} else if( argv[i] == "-p" ) {
			password = argv[i+1];
		} else if( argv[i] == "-n" ) {
			name = argv[i+1];
		} else if( argv[i] == "-m" ) {
			mail_addr = argv[i+1];
		} else if( argv[i] == "-g" ) {
			privilege = read_num( argv[i+1] );
		} else {
			assert(0); // wtf
		}
	}
	
	user_controller.modify_profile( cur_username.c_str(),
									username.c_str(),
									password.c_str(),
									name.c_str(),
									mail_addr.c_str(),
									privilege );
	
	delete []argv;
}

void Interface :: add_train( std::string cmd ) {
	std::string *argv = nullptr;
	int argc = split(cmd, ' ', argv);
	
	std::string train_id;
	int station_num;
	int seat_num;
	std::string stations;
	std::string prices;
	Time start_time;
	std::string travel_times;
	std::string stopover_times;
	std::string sale_date;
	char type;
	
	for( int i = 1; i < argc; i += 2 ) {
		if( argv[i] == "-i" ) {
			train_id = argv[i+1];
		} else if( argv[i] == "-n" ) {
			station_num = read_num( argv[i+1] );
		} else if( argv[i] == "-m" ) {
			seat_num = read_num( argv[i+1] );
		} else if( argv[i] == "-s" ) {
			stations = argv[i+1];
		} else if( argv[i] == "-p" ) {
			prices = argv[i+1];
		} else if( argv[i] == "-x" ) {
			start_time = read_time( argv[i+1] );
		} else if( argv[i] == "-t" ) {
			travel_times = argv[i+1];
		} else if( argv[i] == "-o" ) {
			stopover_times = argv[i+1];
		} else if( argv[i] == "-d" ) {
			sale_date = argv[i+1];
		} else if( argv[i] == "-y" ) {
			type = argv[i+1][0];
		} else {
			assert(0);
		}
	}
	
	char stations_real[STATION_NUM_MAX][STATION_LEN];
	int prices_real[STATION_NUM_MAX];
	int travel_times_real[STATION_NUM_MAX];
	int stopover_times_real[STATION_NUM_MAX];
	Date sale_date_begin;
	Date sale_date_end;
	
	sale_date_begin = read_date( sale_date.substr(0, 5) );
	sale_date_end = read_date( sale_date.substr(6, 5) );
	
	std::string *tmp = nullptr;
	int tn;
	
	tn = split( prices, '|', tmp );
	assert( tn == station_num - 1 );
	for( int i = 0; i < tn; ++i )
		prices_real[i] = read_num( tmp[i] );
	delete []tmp;
	
	tn = split( travel_times, '|', tmp );
	assert( tn == station_num - 1 );
	for( int i = 0; i < tn; ++i )
		travel_times_real[i] = read_num( tmp[i] );
	delete []tmp;
	
	if( station_num > 2 ) {
		tn = split( stopover_times, '|', tmp );
		assert( tn == station_num - 2 );
		for( int i = 0; i < tn; ++i )
			stopover_times_real[i] = read_num( tmp[i] );
		delete []tmp;
	}
	
	tn = split( stations, '|', tmp );
	assert( tn == station_num );
	for( int i = 0; i < tn; ++i )
		strcpy( stations_real[i], tmp[i].c_str() );
	delete []tmp;
	
	train_controller_unreleased.add_train( train_id.c_str(),
										   station_num,
										   seat_num,
										   stations_real,
										   prices_real,
										   start_time,
										   travel_times_real,
										   stopover_times_real,
										   sale_date_begin,
										   sale_date_end,
										   type );
	
	delete []argv;
}

void Interface :: release_train( std::string cmd ) {
	std::string *argv = nullptr;
	int argc = split(cmd, ' ', argv);
	
	std::string train_id;
	
	for( int i = 1; i < argc; i += 2 ) {
		if( argv[i] == "-i" ) {
			train_id = argv[i+1];
		} else {
			assert(0); // wtf
		}
	}
	
	released_train_controller.release_train( train_id.c_str() );
	
	delete []argv;
}

void Interface :: query_train( std::string cmd ) {
	std::string *argv = nullptr;
	int argc = split(cmd, ' ', argv);
	
	std::string train_id;
	Date date;
	
	for( int i = 1; i < argc; i += 2 ) {
		if( argv[i] == "-i" ) {
			train_id = argv[i+1];
		} else if( argv[i] == "-d" ) {
			date = read_date( argv[i+1] );
		} else {
			assert(0); // wtf
		}
	}
	
	released_train_controller.query_train( train_id.c_str(),
										   date );
	
	delete []argv;
}

void Interface :: delete_train( std::string cmd ) {
	std::string *argv = nullptr;
	int argc = split(cmd, ' ', argv);
	
	std::string train_id;
	
	for( int i = 1; i < argc; i += 2 ) {
		if( argv[i] == "-i" ) {
			train_id = argv[i+1];
		} else {
			assert(0); // wtf
		}
	}
	
	train_controller_unreleased.delete_train( train_id.c_str() );
	
	delete []argv;
}

void Interface :: query_ticket( std::string cmd ) {
	std::string *argv = nullptr;
	int argc = split(cmd, ' ', argv);
	
	std::string from;
	std::string to;
	Date date;
	std::string priority = "time";
	
	for( int i = 1; i < argc; i += 2 ) {
		if( argv[i] == "-s" ) {
			from = argv[i+1];
		} else if( argv[i] == "-t" ) {
			to = argv[i+1];
		} else if( argv[i] == "-d" ) {
			date = read_date( argv[i+1] );
		} else if( argv[i] == "-p" ) {
			priority = argv[i+1];
		} else {
			assert(0); // wtf
		}
	}
	
	ticket_controller.query_ticket( from.c_str(),
									to.c_str(),
									date,
									priority.c_str() );
	
	delete []argv;
}

void Interface :: query_transfer( std::string cmd ) {
	std::string *argv = nullptr;
	int argc = split(cmd, ' ', argv);
	
	std::string from;
	std::string to;
	Date date;
	std::string priority = "time";
	
	for( int i = 1; i < argc; i += 2 ) {
		if( argv[i] == "-s" ) {
			from = argv[i+1];
		} else if( argv[i] == "-t" ) {
			to = argv[i+1];
		} else if( argv[i] == "-d" ) {
			date = read_date( argv[i+1] );
		} else if( argv[i] == "-p" ) {
			priority = argv[i+1];
		} else {
			assert(0); // wtf
		}
	}
	
	ticket_controller.query_transfer( from.c_str(),
									  to.c_str(),
									  date,
									  priority.c_str() );
	
	delete []argv;
}

void Interface :: buy_ticket( std::string cmd ) {
	std::string *argv = nullptr;
	int argc = split(cmd, ' ', argv);
	
	std::string username;
	std::string train_id;
	Date date;
	int num;
	std::string from;
	std::string to;
	bool que = false;
	
	for( int i = 1; i < argc; i += 2 ) {
		if( argv[i] == "-u" ) {
			username = argv[i+1];
		} else if( argv[i] == "-i" ) {
			train_id = argv[i+1];
		} else if( argv[i] == "-d" ) {
			date = read_date( argv[i+1] );
		} else if( argv[i] == "-n" ) {
			num = read_num( argv[i+1] );
		} else if( argv[i] == "-f" ) {
			from = argv[i+1];
		} else if( argv[i] == "-t" ) {
			to = argv[i+1];
		} else if( argv[i] == "-q" ) {
			if( argv[i+1] == "false" ) {
				que = false;
			} else if( argv[i+1] == "true" ) {
				que = true;
			} else {
				assert(0); // wtf
			}
		} else {
			assert(0); // wtf
		}
	}
	
	ticket_controller.buy_ticket( username.c_str(),
								  train_id.c_str(),
								  date,
								  from.c_str(),
								  to.c_str(),
								  num,
								  que );
	
	delete []argv;
}

void Interface :: query_order( std::string cmd ) {
	std::string *argv = nullptr;
	int argc = split(cmd, ' ', argv);
	
	std::string username;
	
	for( int i = 1; i < argc; i += 2 ) {
		if( argv[i] == "-u" ) {
			username = argv[i+1];
		} else {
			assert(0); // wtf
		}
	}
	
	user_controller.query_order( username.c_str() );
	
	delete []argv;
}

void Interface :: refund_ticket( std::string cmd ) {
	std::string *argv = nullptr;
	int argc = split(cmd, ' ', argv);
	
	std::string username;
	int order_id = 1;
	
	for( int i = 1; i < argc; i += 2 ) {
		if( argv[i] == "-u" ) {
			username = argv[i+1];
		} else if( argv[i] == "-n" ) {
			order_id = read_num( argv[i+1] );
		} else {
			assert(0); // wtf
		}
	}
	
	ticket_controller.refund_ticket( username.c_str(),
									 order_id );
	
	delete []argv;
}

void Interface :: clean() {
	user_controller.save();
	order_controller.save();
	released_train_controller.save();
	train_controller_unreleased.save(2 * sizeof(int));
	train_controller_released.save(3 * sizeof(int));
	ticket_controller.save();
	
	FileOperator().create_new_file("user_btree");
	FileOperator().create_new_file("user_info");
	FileOperator().create_new_file("order_btree");
	FileOperator().create_new_file("order_info");
	FileOperator().create_new_file("que_btree");
	FileOperator().create_new_file("que_info");
	FileOperator().create_new_file("train_info_unreleased");
	FileOperator().create_new_file("train_id_unreleased");
	FileOperator().create_new_file("train_info_released");
	FileOperator().create_new_file("train_id_released");
	FileOperator().create_new_file("ticket_info");
	FileOperator().create_new_file("station_btree");
	FileOperator().create_new_file("station_info");
	FileOperator().create_new_file("counts"); // initialize in start()
	
	start();
	puts("0");
}

int Interface :: split( std::string str, char ch, std::string *&ans ) {
	if( ch != ' ' ) {
		int cnt = 1;
		for( int i = 0; i < (int)str.length(); ++i )
			if( str[i] == ch )
				++cnt;
		ans = new std::string[cnt];
		int now = 0;
		for( int i = 0; i < (int)str.length(); ++i ) {
			if( str[i] == ch ) {
				++now;
			} else {
				ans[now].push_back( str[i] );
			}
		}
		assert( now == cnt-1 );
		return cnt;
	} else { // ch == ' '
		int cnt = 0;
		std::stringstream ss(str);
		std::string tmp;
		while( ss >> tmp ) ++cnt;
		ans = new std::string[cnt];
		ss = std::stringstream(str);
		for( int i = 0; i < cnt; ++i )
			ss >> ans[i];
		assert( bool(ss >> tmp) == false );
		return cnt;
	}
}

int Interface :: read_num( std::string str ) {
	int x = 0;
	for( int i = 0; i < (int)str.length(); ++i )
		x = x * 10 + str[i] - '0';
	return x;
}

Time Interface :: read_time( std::string str ) {
	int hour = read_num( str.substr(0, 2) );
	int minute = read_num( str.substr(3, 2) );
	return Time( Date(), hour, minute );
}

Date Interface :: read_date( std::string str ) {
	int month = read_num( str.substr(0, 2) );
	int day = read_num( str.substr(3, 2) );
	return Date(2020, month, day);
}
