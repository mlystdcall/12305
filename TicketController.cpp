#include "includes.hpp"

int TicketController::query_pass( const char station[], std::pair<Char, int> *&result ) {
	std::pair<int, int> id = Hash().hash(station);
	std::pair<std::pair<int, int>, std::pair<int, int> > l = std::make_pair(id, std::make_pair(0, 0));
	std::pair<std::pair<int, int>, std::pair<int, int> > r = std::make_pair(id, std::make_pair(2e9, 2e9));
	std::pair<std::pair<std::pair<int, int>, std::pair<int, int> >, std::pair<Char, int> > *value;
	int cnt = btree.query_list(l, r, btree_file, info_file, value);
	if(cnt) result = new std::pair<Char, int>[cnt];
	for (int i = 0; i < cnt; ++i) {
		result[i] = value[i].second;
		//printf("query_pass %d: %s\n", i, value[i].second.first.str); //////////////////////////
	}
	if(cnt) delete []value;
	return cnt;
}

int TicketController::get_all(const Train &train, const char from[], const char to[], Time &leaving_time, Time &arriving_time) {
	int price = 0;
	leaving_time = train.start_time;
	leaving_time.date = train.sale_date_begin;
	int j = 0;
	for (; j < train.station_num; ++j) {
		if (strcmp(train.stations[j], from) == 0) break;
		leaving_time = leaving_time + train.travel_times[j];
		leaving_time = leaving_time + train.stopover_times[j];
	}
	arriving_time = leaving_time + train.travel_times[j];
	price += train.prices[j];
	for (++j; j < train.station_num; ++j) {//
		if (strcmp(train.stations[j], to) == 0) break;
		arriving_time = arriving_time + train.stopover_times[j - 1];
		arriving_time = arriving_time + train.travel_times[j];
		price += train.prices[j];
	}
	return price;
}

void TicketController::query_ticket(const char from[], const char to[], Date date, const char priority[]) {
	std::pair<Char, int> *from_result, *to_result;
	int A = query_pass(from, from_result);
	int B = query_pass(to, to_result);
	if(A == 0 || B == 0) {
		if(A) delete []from_result;
		if(B) delete []to_result;
		printf("0\n");
		return;
	}
	sort(from_result, from_result + A);
	sort(to_result, to_result + B);
	char train_id[TRAIN_ID_LEN];
	//calc cnt
	int pos1 = 0, pos2 = 0, cnt = 0;
	while(pos1 < A && pos2 < B) {
		if(from_result[pos1].first == to_result[pos2].first) {
			from_result[pos1].first.copy(train_id);
			if(from_result[pos1].second >= to_result[pos2].second) {
				++pos1; ++pos2; continue;
			}
			//printf("pos1 = %d, A = %d, train_id: %s\n", pos1, A, train_id); /////////////////////
			Train train = itf->train_controller_released.btree.query(Hash().hash(train_id), itf->train_controller_released.btree_file, itf->train_controller_released.info_file);
			Time leaving_time, arriving_time; 
			get_all(train, from, to, leaving_time, arriving_time);
			//check between sale_date_begin and sale_date_end 
			int p = date - leaving_time.date;
			if(p < 0 || p > (train.sale_date_end - train.sale_date_begin)) {
				++pos1; ++pos2; continue;
			}
			++cnt;
			++pos1; ++pos2; continue;
		}
		if(from_result[pos1].first < to_result[pos2].first)  ++pos1;
		else ++pos2;
	}

	printf("%d\n", cnt);
	if(!cnt) {
		delete []from_result;
		delete []to_result;
		return;
	}
	//train_id, price, ticket, leaving_time, arriving_time
	#define ANS_TYPE std::pair<std::tuple<std::string, int, int>, std::pair<Time, Time> >
	ANS_TYPE *ans = new ANS_TYPE[cnt];
	//calc ans
	pos1 = 0; pos2 = 0; int d = 0;
	while(pos1 < A && pos2 < B) {
		if(from_result[pos1].first == to_result[pos2].first) {
			from_result[pos1].first.copy(train_id);
			if(from_result[pos1].second >= to_result[pos2].second) {
				++pos1; ++pos2; continue;
			}
			//printf("%d, train_id: %s\n", pos1, train_id); /////////////////////
			Train train = itf->train_controller_released.btree.query(Hash().hash(train_id), itf->train_controller_released.btree_file, itf->train_controller_released.info_file);
			int price, ticket;
			Time leaving_time, arriving_time; 
			price = get_all(train, from, to, leaving_time, arriving_time);
			//check between sale_date_begin and sale_date_end 
			int p = date - leaving_time.date;
			if(p < 0 || p > (train.sale_date_end - train.sale_date_begin)) {
				++pos1; ++pos2; continue;
			}

			leaving_time.date = date;
			arriving_time.date = arriving_time.date + p;
			ticket = itf->released_train_controller.query_ticket(train, train.sale_date_begin + p, from, to);
			ans[d++] = std::make_pair( std::make_tuple(std::string(train.train_id), price, ticket), 
										std::make_pair(leaving_time, arriving_time) );
			++pos1; ++pos2; continue;
		}

		if(from_result[pos1].first < to_result[pos2].first)  ++pos1;
		else ++pos2;
	}
	//sort
	if(priority[0] == 't') sort(ans, ans + cnt, [](const ANS_TYPE &a, const ANS_TYPE &b)
		->bool{return ( (a.second.second - a.second.first < b.second.second - b.second.first)
					|| (a.second.second - a.second.first == b.second.second - b.second.first 
							&& std::get<0>(a.first) < std::get<0>(b.first)) );} );
	else sort(ans, ans + cnt, [](const ANS_TYPE &a, const ANS_TYPE &b)
		->bool{return ( (std::get<1>(a.first) < std::get<1>(b.first) )
					|| (std::get<1>(a.first) == std::get<1>(b.first) 
							&& std::get<0>(a.first) < std::get<0>(b.first)) );} );

	for (int i = 0; i < cnt; ++i) {
		std::string tmp = std::get<0>(ans[i].first);
		int price = std::get<1>(ans[i].first);
		int ticket = std::get<2>(ans[i].first);
		Time leaving_time = ans[i].second.first;
		Time arriving_time = ans[i].second.second;
		std::cout << tmp;
		printf(" %s ", from);
		std::cout << leaving_time.date << " " << leaving_time;
		printf(" -> %s ", to);
		std::cout << arriving_time.date << " " << arriving_time;
		printf(" %d %d\n", price, ticket);
	}
	delete []from_result;
	delete []to_result;
	delete []ans;
	#undef ANS_TYPE
}

void TicketController::query_transfer(const char from[], const char to[], Date date, const char priority[]) {
	std::pair<Char, int> *from_result, *to_result;
	int A = query_pass(from, from_result);
	int B = query_pass(to, to_result);
	if(A == 0 || B == 0) {
		if(A) delete []from_result;
		if(B) delete []to_result;
		printf("0\n");
		return;
	}
	
	char train_id_a[TRAIN_ID_LEN], train_id_b[TRAIN_ID_LEN];
	
	char ans_id_a[TRAIN_ID_LEN], ans_id_b[TRAIN_ID_LEN], ans_station[STATION_LEN];
	Time ans_leaving_a, ans_arriving_a, ans_leaving_b, ans_arriving_b;
	int ans_num = 2e9, ans_price_a, ans_price_b, ans_ticket_a, ans_ticket_b;

	for (int i = 0; i < A; ++i) {
		from_result[i].first.copy(train_id_a);
		Train train_a = itf->train_controller_released.btree.query(Hash().hash(train_id_a), itf->train_controller_released.btree_file, itf->train_controller_released.info_file);
		if(from_result[i].second == train_a.station_num - 1) continue;

		Time leaving_time, arriving_time;
		leaving_time = train_a.start_time;
		leaving_time.date = train_a.sale_date_begin;
		for (int j = 0; j < from_result[i].second; ++j) {
			leaving_time = leaving_time + train_a.travel_times[j];
			leaving_time = leaving_time + train_a.stopover_times[j];
		}
		//check between sale_date_begin and sale_date_end 
		int pa = date - leaving_time.date;
		if(pa < 0 || pa > (train_a.sale_date_end - train_a.sale_date_begin)) continue;
		leaving_time.date = date;

		arriving_time = leaving_time + train_a.travel_times[from_result[i].second];
		int price = train_a.prices[from_result[i].second];
		for (int j = from_result[i].second + 1; j < train_a.station_num; ++j) {
			for (int k = 0; k < B; ++k) {
				if(from_result[i].first == to_result[k].first) continue;
				if(to_result[k].second == 0) continue;
				to_result[k].first.copy(train_id_b);
				Train train_b = itf->train_controller_released.btree.query(Hash().hash(train_id_b), itf->train_controller_released.btree_file, itf->train_controller_released.info_file);
				for (int t = 0; t < to_result[k].second; ++t) {
					if(strcmp(train_a.stations[j], train_b.stations[t]) == 0) {
						Time lv, _leaving_time, _arriving_time;
						int now_price = get_all(train_b, train_b.stations[t], to, lv, _arriving_time);
						_leaving_time = lv;
						_leaving_time.date = arriving_time.date;
						if(_leaving_time < arriving_time) 
							_leaving_time.date = _leaving_time.date + 1;
						//check between sale_date_begin and sale_date_end 
						int pb = _leaving_time.date - lv.date;
						if(pb < 0) {
							_leaving_time.date = lv.date;
							pb = 0;
						}
						if(pb > (train_b.sale_date_end - train_b.sale_date_begin)) break;

						_arriving_time.date = _arriving_time.date + pb;
						
						int num;
						if(priority[0] == 't') num = _arriving_time - leaving_time;
						else num = now_price + price;
						std::string now_a(train_id_a), now_b(train_id_b), ans_a(ans_id_a), ans_b(ans_id_b);
						std::pair<std::string, std::string> now_pair = make_pair(now_a, now_b);
						std::pair<std::string, std::string> ans_pair = make_pair(ans_a, ans_b);
						if(num < ans_num || (num == ans_num && now_pair < ans_pair) || (num == ans_num && now_pair == ans_pair && arriving_time - leaving_time < ans_arriving_a - ans_leaving_a)) {
							ans_num = num;
							strcpy(ans_id_a, train_id_a);
							strcpy(ans_id_b, train_id_b);
							strcpy(ans_station, train_a.stations[j]);
							ans_leaving_a = leaving_time;
							ans_arriving_a = arriving_time;
							ans_leaving_b = _leaving_time;
							ans_arriving_b = _arriving_time;
							ans_price_a = price; ans_price_b = now_price;
							ans_ticket_a = itf->released_train_controller.query_ticket(train_a, train_a.sale_date_begin + pa, from, ans_station);
							ans_ticket_b = itf->released_train_controller.query_ticket(train_b, train_b.sale_date_begin + pb, ans_station, to);
						}
						break;
					}
				}
			}
			if(j == train_a.station_num - 1) break;
			arriving_time = arriving_time + train_a.stopover_times[j - 1];
			arriving_time = arriving_time + train_a.travel_times[j];
			price += train_a.prices[j];
		}
	}

	if(ans_num == 2e9) {
		printf("0\n"); 
		delete []from_result;
		delete []to_result;
		return;
	}
	
	printf("%s %s ", ans_id_a, from);
	std::cout << ans_leaving_a.date << " " << ans_leaving_a;
	printf(" -> %s ", ans_station);
	std::cout << ans_arriving_a.date << " " << ans_arriving_a;
	printf(" %d %d\n", ans_price_a, ans_ticket_a);
	
	printf("%s %s ", ans_id_b, ans_station);
	std::cout << ans_leaving_b.date << " " << ans_leaving_b;
	printf(" -> %s ", to);
	std::cout << ans_arriving_b.date << " " << ans_arriving_b;
	printf(" %d %d\n", ans_price_b, ans_ticket_b);
	
	delete []from_result;
	delete []to_result;
}

void TicketController::buy_ticket( const char username[], const char train_id[], Date date, const char from[], const char to[], int num, bool que ) {
	if(num <= 0) {
		printf("-1\n");
		return;
	}
	
	if(!itf->user_controller.btree.exist(Hash().hash(username), itf->user_controller.btree_file)) {
		printf("-1\n"); return;
	}
	User user = itf->user_controller.btree.query(Hash().hash(username), itf->user_controller.btree_file, itf->user_controller.info_file);
	if(!itf->user_controller.is_online[user.create_time]) {
		printf("-1\n"); return;
	}

	if(!itf->train_controller_released.btree.exist(Hash().hash(train_id), itf->train_controller_released.btree_file)) {	
		printf("-1\n"); return;
	}

	Train train = itf->train_controller_released.btree.query(Hash().hash(train_id), itf->train_controller_released.btree_file, itf->train_controller_released.info_file);
	//check from, to
	int l = -1, r = -1;
	for (int i = 0; i < train.station_num; ++i) {
		if(strcmp(from, train.stations[i]) == 0) l = i;
		if(strcmp(to, train.stations[i]) == 0) r = i;
	}
	if(l == -1 || r == -1 || l >= r) {
		printf("-1\n"); return;
	}
	
	Time leaving_time, arriving_time;
	long long price = get_all(train, from, to, leaving_time, arriving_time);
	int p = date - leaving_time.date;
	if(p < 0 || p > (train.sale_date_end - train.sale_date_begin)) {
		printf("-1\n"); return;
	}

	Date sale_date = train.sale_date_begin + p;	
	int ticket = itf->released_train_controller.query_ticket(train, sale_date, from, to);
	assert(price >= 0);
	if((ticket < num && que == false) || num > train.seat_num) { printf("-1\n"); return; }
	if(ticket < num) printf("queue\n");
	else printf("%lld\n", price * (long long)num);

	// order's price : for only one ticket
	Order order = Order(username, ++user.order_cnt, STATUS_SUCCESS, train_id, from, to, sale_date, price, num, itf->order_controller.order_cnt + 1);
	//user has been changed, so let's insert it back
	itf->user_controller.btree.insert(Hash().hash(username), user, itf->user_controller.btree_file, itf->user_controller.info_file);
	if(ticket < num) order.status = STATUS_PENDING;
	itf->user_controller.add_order(username, order);
	if(ticket < num) {
		itf->released_train_controller.add_order(train_id, sale_date, username, order.order_id);
	}
	else {
		itf->released_train_controller.modify_ticket(train, sale_date, from, to, -num);
	}
}

void TicketController::refund_ticket( const char username[], int order_id ) {
	if(!itf->user_controller.btree.exist(Hash().hash(username), itf->user_controller.btree_file)) {
		printf("-1\n"); return;
	}
	User user = itf->user_controller.btree.query(Hash().hash(username), itf->user_controller.btree_file, itf->user_controller.info_file);
	if(!itf->user_controller.is_online[user.create_time]) {
		printf("-1\n"); return;
	}

	if(order_id > user.order_cnt) { printf("-1\n"); return; }
	order_id = user.order_cnt - order_id + 1;

	Order order = itf->order_controller.btree.query(std::make_pair(Hash().hash(username), order_id), itf->order_controller.btree_file, itf->order_controller.info_file);
	if(order.status == STATUS_REFUNDED) { printf("-1\n"); return; }

	if(order.status == STATUS_SUCCESS) {
		Train train = itf->train_controller_released.btree.query(Hash().hash(order.train_id), itf->train_controller_released.btree_file, itf->train_controller_released.info_file);
		itf->released_train_controller.modify_ticket(train, order.sale_date, order.from, order.to, order.num);
		itf->released_train_controller.adjust_order(order.train_id, order.sale_date);
	}
	else {
		itf->released_train_controller.delete_order(order.train_id, order.sale_date, username, order.order_id, order.buy_time);
	}

	order.status = STATUS_REFUNDED;
	itf->order_controller.btree.insert(std::make_pair(Hash().hash(username), order_id), order, itf->order_controller.btree_file, itf->order_controller.info_file);

	printf("0\n");
}

void TicketController::load( Interface *interface ) {
	itf = interface;
	btree_file.open("station_btree");
	info_file.open("station_info");
}

void TicketController::save() {
	btree.write_cache(btree_file, info_file);
	btree_file.close();
	info_file.close();
}
