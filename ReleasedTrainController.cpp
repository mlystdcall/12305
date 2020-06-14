#include "includes.hpp"

#define controller_unreleased itf->train_controller_unreleased
#define controller_released itf->train_controller_released

void ReleasedTrainController::release_train( const char train_id[] ) {
	std::pair<int, int> id = Hash().hash(train_id);
	if(!controller_unreleased.btree.exist(id, controller_unreleased.btree_file)) {
		printf("-1\n");
		return;
	}
	printf("0\n");
	Train train = controller_unreleased.btree.query(id, controller_unreleased.btree_file, controller_unreleased.info_file);
	controller_unreleased.btree.remove(id, controller_unreleased.btree_file);
	controller_released.train_cnt++;
	train.create_time = controller_released.train_cnt; //
	controller_released.btree.insert(id, train, controller_released.btree_file, controller_released.info_file);
	//ticket_file
	for (int i = 0; i < SUM * SUM; ++i) 
		file_operator.write(ticket_file, -1, &train.seat_num, 1);
	//station_btree (haven't done in TrainController::add_train())
	for (int i = 0; i < train.station_num; ++i) {
		std::pair<int, int> station_id = Hash().hash(train.stations[i]);
		itf->ticket_controller.btree.insert(std::make_pair(station_id, id), std::make_pair(TicketController::Char(train_id), i), itf->ticket_controller.btree_file, itf->ticket_controller.info_file);
	}
}

void ReleasedTrainController::query_train( const char train_id[], Date date ) {
	if(!date.exist()) {
		printf("-1\n"); return;
	}
	bool fl = 0;
	//train_id exist
	std::pair<int, int> id = Hash().hash(train_id);
	Train train;
	if(controller_released.btree.exist(id, controller_released.btree_file)) {
		train = controller_released.btree.query(id, controller_released.btree_file, controller_released.info_file);
	}
	else if(controller_unreleased.btree.exist(id, controller_unreleased.btree_file)) {
		train = controller_unreleased.btree.query(id, controller_unreleased.btree_file, controller_unreleased.info_file);
		fl = 1;
	}
	else {
		printf("-1\n"); return;
	}
	//check date
	if(date < train.sale_date_begin || train.sale_date_end < date) {
		printf("-1\n"); return;
	}
	//read ticket_file
	int *ticket = new int[SUM + 7];
	if(fl) {
		for (int i = 0; i < train.station_num; ++i) ticket[i] = train.seat_num;
	}
	else file_operator.read(ticket_file, ((train.create_time - 1) * SUM * SUM + (date - train.sale_date_begin) * SUM) * (int)sizeof(int), ticket, SUM);

	printf("%s %c\n", train_id, train.type);
	Time now = train.start_time; now.date = date;
	int sum = 0;//total price
	for (int i = 0; i < train.station_num; ++i) {
		printf("%s ", train.stations[i]);
		if(!i) printf("xx-xx xx:xx");
		else std::cout << now.date << " " << now;
		
		printf(" -> ");

		if(i == train.station_num - 1) printf("xx-xx xx:xx");
		else {
			if(i) now = now + train.stopover_times[i - 1];
			std::cout << now.date << " " << now;
		}

		printf(" %d ", sum);

		if(i == train.station_num - 1) printf("x\n");
		else printf("%d\n", ticket[i]);

		if(i != train.station_num - 1) {
			sum += train.prices[i];
			now = now + train.travel_times[i];
		}
	}
	delete []ticket;
}

void ReleasedTrainController::modify_ticket( const Train &train, Date date, const char from[], const char to[], int num ) {
	int *ticket = new int[SUM + 7];
	file_operator.read(ticket_file, ((train.create_time - 1) * SUM * SUM + (date - train.sale_date_begin) * SUM) * (int)sizeof(int), ticket, SUM);
	int fl = 0;//whether between from and to
	for (int i = 0; i < train.station_num; ++i) {
		if(fl == 0 && strcmp(train.stations[i], from) == 0) fl = 1;
		if(fl == 1 && strcmp(train.stations[i], to) == 0) break;
		if(fl) ticket[i] += num;
	}
file_operator.write(ticket_file, ((train.create_time - 1) * SUM * SUM + (date - train.sale_date_begin) * SUM) * (int)sizeof(int), ticket, SUM);
	delete []ticket;
}

void ReleasedTrainController::add_order( const char train_id[], Date date, const char username[], int order_id ) {
	int buy_time = itf->order_controller.order_cnt;
	std::pair<int, int> id = Hash().hash(train_id);
	btree.insert(make_tuple(id, date, buy_time), std::make_pair(Char(username), order_id), que_btree_file, que_info_file);
}

void ReleasedTrainController::delete_order( const char train_id[], Date date, const char username[], int order_id, int buy_time ) {
	if(buy_time == -1) {
		std::pair<int, int> userid = Hash().hash(username);
		Order order = itf->order_controller.btree.query(std::make_pair(userid, order_id), itf->order_controller.btree_file, itf->order_controller.info_file);
		buy_time = order.buy_time;
	}
	std::pair<int, int> id = Hash().hash(train_id);
	btree.remove(make_tuple(id, date, buy_time), que_btree_file);
}

void ReleasedTrainController::adjust_order( const char train_id[], Date date ) {
	std::pair<int, int> id = Hash().hash(train_id);
	std::tuple<std::pair<int, int>, Date, int> l = make_tuple(id, date, -1);
	std::tuple<std::pair<int, int>, Date, int> r = make_tuple(id, date, 1e8);
	std::pair<std::tuple<std::pair<int, int>, Date, int>, std::pair<Char, int> > *value;
	int cnt = btree.query_list(l, r, que_btree_file, que_info_file, value);
	std::pair<int, int> userid; int order_id;
	char username[USERNAME_LEN];
	for (int i = 0; i < cnt; ++i) {
		value[i].second.first.copy(username);
		userid = Hash().hash(username);
		order_id = value[i].second.second;
		Order order = itf->order_controller.btree.query(std::make_pair(userid, order_id), itf->order_controller.btree_file, itf->order_controller.info_file);
		Train train = controller_released.btree.query(id, controller_released.btree_file, controller_released.info_file);
		if(query_ticket(train, date, order.from, order.to) >= order.num) {
			delete_order(train_id, date, username, order_id, order.buy_time);
			modify_ticket(train, date, order.from, order.to, -order.num); //
			order.status = STATUS_SUCCESS;
			itf->user_controller.modify_order(username, order_id, order);
		}
	}
	if(cnt) delete []value;
}

int ReleasedTrainController::query_ticket( const Train &train, Date date, const char from[], const char to[] ) {
	int *ticket = new int[SUM + 7];
	file_operator.read(ticket_file, ((train.create_time - 1) * SUM * SUM + (date - train.sale_date_begin) * SUM) * (int)sizeof(int), ticket, SUM);
	int fl = 0, rs = 1e8;
	for (int i = 0; i < train.station_num && rs > 0; ++i) {
		if(fl == 0 && strcmp(train.stations[i], from) == 0) fl = 1;
		if(fl == 1 && strcmp(train.stations[i], to) == 0) break;
		if(fl && ticket[i] < rs) rs = ticket[i];
	}
	delete []ticket;
	return rs;
}

void ReleasedTrainController::load(Interface *interface) {
	itf = interface; 
	ticket_file.open("ticket_info");
	que_btree_file.open("que_btree");
	que_info_file.open("que_info");
}

void ReleasedTrainController::save() {
	file_operator.write_cache(ticket_file);
	btree.write_cache(que_btree_file, que_info_file);
	ticket_file.close();
	que_btree_file.close();
	que_info_file.close();
}

#undef controller_unreleased
#undef controller_released
