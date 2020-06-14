#include "includes.hpp"

// BTree named as `btree`
// the operating user named as `cur_user`
// the operated user named as `todo_user`

void UserController::add_user( const char cur_username[],
				               const char username[],
				               const char password[],
				               const char name[],
				               const char mail_addr[],
				               int privilege ) {
    // FAILURE if username exists
    if ( btree.exist(Hash().hash(username), btree_file, info_file)) {printf("-1\n"); return;}
	
    if (this->user_cnt == 0) {
        User todo_user(username, password, name, mail_addr, 10, ++user_cnt, 0, 0);
        btree.insert(Hash().hash(username), todo_user, btree_file, info_file);
    }
    else {
        // FAILURE if cur_username doesn't exist
        if (!btree.exist(Hash().hash(cur_username), btree_file, info_file)) {printf("-1\n"); return;}
        User cur_user = btree.query(Hash().hash(cur_username), btree_file, info_file);
        if (is_online[cur_user.create_time] && (cur_user.privilege > privilege)) {
            User todo_user(username, password, name, mail_addr, privilege, ++user_cnt, 0, 0);
            btree.insert(Hash().hash(username), todo_user, btree_file, info_file);
        }
        else {
            printf("-1\n");
            return;
        }
    }
    printf("0\n");
}

void UserController::login( const char username[],
                            const char password[] ) {
    // FAILURE if username doesn't exist
    if (!btree.exist(Hash().hash(username), btree_file, info_file)) {printf("-1\n"); return;}
    User todo_user = btree.query(Hash().hash(username), btree_file, info_file);

    if (strcmp(todo_user.password, password) == 0 && is_online[todo_user.create_time] == false) {
        is_online[todo_user.create_time] = true;
        printf("0\n");
    }
    else {
        // FAILURE if password is wrong OR MULTIPLE LOGINS
        printf("-1\n");
    }
}

void UserController::logout( const char username[] ) {
    // FAILURE if username doesn't exist
    if (!btree.exist(Hash().hash(username), btree_file, info_file)) {printf("-1\n"); return;}
    User todo_user = btree.query(Hash().hash(username), btree_file, info_file);

    if (is_online[todo_user.create_time] == true) {
		is_online[todo_user.create_time] = false;
        printf("0\n");
    }
    else {
        // FAILURE if MULTIPLE LOGOUTS
        printf("-1\n");
    }
}

void UserController::query_profile( const char cur_username[],
                                    const char username[] ) {
    // FAILURE if cur_username doesn't exist or username doesn't exist
    if (!btree.exist(Hash().hash(cur_username), btree_file, info_file)) {printf("-1\n"); return;}
    if (!btree.exist(Hash().hash(    username), btree_file, info_file)) {printf("-1\n"); return;}
    User cur_user  = btree.query(Hash().hash(cur_username), btree_file, info_file);
    User todo_user = btree.query(Hash().hash(username), btree_file, info_file);

    if (!is_online[cur_user.create_time]) {printf("-1\n"); return;}
    if ((strcmp(cur_username, username) == 0) || (cur_user.privilege > todo_user.privilege)) {
        print_profile(username);
    }
    else {printf("-1\n");}
}

void UserController::modify_profile( const char cur_username[],
                                     const char username[],
                                     const char password[], // empty is ""
                                     const char name[], // empty is ""
                                     const char mail_addr[], // empty is ""
                                     int privilege ) { // empty is -1
    // FAILURE if cur_username doesn't exist or username doesn't exist
    if (!btree.exist(Hash().hash(cur_username), btree_file, info_file)) {printf("-1\n"); return;}
    if (!btree.exist(Hash().hash(    username), btree_file, info_file)) {printf("-1\n"); return;}
    User cur_user  = btree.query(Hash().hash(cur_username), btree_file, info_file);
    User todo_user = btree.query(Hash().hash(username), btree_file, info_file);

    if (!is_online[cur_user.create_time]) {printf("-1\n"); return;}
    if (((strcmp(cur_username, username) == 0) || (cur_user.privilege > todo_user.privilege)) && privilege < cur_user.privilege) {
        // WARNING: not quite sure with the following 4 lines...担心strcpy会不会溢出
        if (strcmp(password, "") != 0) strcpy(todo_user.password, password);
        if (strcmp(name, "") != 0) strcpy(todo_user.name, name);
        if (strcmp(mail_addr, "") != 0) strcpy(todo_user.mail_addr, mail_addr);
        if (privilege != -1) todo_user.privilege = privilege;
        btree.insert(Hash().hash(username), todo_user, btree_file, info_file);
		printf("%s %s %s %d\n", username, todo_user.name, todo_user.mail_addr, todo_user.privilege);
    }
    else {printf("-1\n");}
}

void UserController::query_order( const char username[] ) {
    // FAILURE if username doesn't exist
    if (!btree.exist(Hash().hash(username), btree_file, info_file)) {printf("-1\n"); return;}
    User todo_user = btree.query(Hash().hash(username), btree_file, info_file);
	if(!is_online[todo_user.create_time]) {printf("-1\n"); return;}
	print_order(username);
}

void UserController::load( Interface *ifs ) {
	memset(is_online, 0, sizeof(is_online));
    interface = ifs;
    btree_file.open("user_btree");
    info_file.open("user_info");
    FileOperator fop;
    std::fstream cnt_file;
    cnt_file.open("counts");
    fop.read(cnt_file, 0, &user_cnt, 1);
    cnt_file.close();
}

void UserController::save() {
	//memset(is_online, 0, sizeof(is_online));
    btree_file.close();
    info_file.close();
    FileOperator fop;
    std::fstream cnt_file;
    cnt_file.open("counts");
    fop.write(cnt_file, 0, &user_cnt, 1);
    cnt_file.close();
}

void UserController::modify_order( const char username[], int order_id, Order order ) {
    // WARNING: no checking whether username or order exists
    std::pair<std::pair<int, int>, int> temp(Hash().hash(username), order_id);
    interface->order_controller.btree.insert(temp, order, interface->order_controller.btree_file, interface->order_controller.info_file);
}

Order UserController::get_order( const char username[], int order_id ) {
    // WARNING: no checking whether username or order exists
    std::pair<std::pair<int, int>, int> temp(Hash().hash(username), order_id);
    return interface->order_controller.btree.query(temp, interface->order_controller.btree_file, interface->order_controller.info_file);
}

void UserController::print_profile( const char username[] ) {
    // WARNING: no checking whether username exists
    User todo_user = btree.query(Hash().hash(username), btree_file, info_file);
    
    printf("%s %s %s %d\n", username, todo_user.name, todo_user.mail_addr, todo_user.privilege);
}

void UserController::print_order( const char username[] ) {
    // WARNING: no checking whether username or order exists
    std::pair<std::pair<int, int>, int> tem_l(Hash().hash(username), 0);
    // WARNING: const `1e8` is the INF of order_id; it's not saved as a const
    std::pair<std::pair<int, int>, int> tem_r(Hash().hash(username), 1e8);
    std::pair<std::pair<std::pair<int, int>, int>, Order> *value;
    int cnt = interface->order_controller.btree.query_list(tem_l, tem_r, interface->order_controller.btree_file, interface->order_controller.info_file, value);
    printf("%d\n", cnt);
    for (int i = 0; i < cnt; ++i) {
        Order todo_order = value[cnt - i - 1].second;
        
        // calc leaving_time and arriving_time
        Time leaving_time, arriving_time;
        Train todo_train = interface->train_controller_released.btree.query(Hash().hash(todo_order.train_id), interface->train_controller_released.btree_file, interface->train_controller_released.info_file);
        int j = 0;
		leaving_time = todo_train.start_time;
		leaving_time.date = todo_order.sale_date;
        for (; j < todo_train.station_num; ++j) {
            if (strcmp(todo_train.stations[j], todo_order.from) == 0) break;
            leaving_time = leaving_time + todo_train.travel_times[j];
            leaving_time = leaving_time + todo_train.stopover_times[j];
        }
        arriving_time = leaving_time + todo_train.travel_times[j];
        for (++j; j < todo_train.station_num; ++j) {//
            if (strcmp(todo_train.stations[j], todo_order.to) == 0) break;
            arriving_time = arriving_time + todo_train.stopover_times[j - 1];
            arriving_time = arriving_time + todo_train.travel_times[j];
        }

        // print: [<STATUS>] <trainID> <FROM> <LEAVING_TIME> -> <TO> <ARRIVING_TIME> <PRICE> <NUM>
        switch (todo_order.status) {
            case STATUS_SUCCESS:
                printf("[success]");
                break;
            case STATUS_PENDING:
                printf("[pending]");
                break;
            case STATUS_REFUNDED:
                printf("[refunded]");
                break;
        }
        printf(" %s %s ", todo_order.train_id, todo_order.from);
        std::cout << leaving_time.date << ' ' << leaving_time;
        printf(" -> %s ", todo_order.to);
        std::cout << arriving_time.date << ' '  << arriving_time << ' ' << todo_order.price << ' ' << todo_order.num << std::endl;
    }
    if(cnt) delete [] value;
}

void UserController::add_order( const char username[], Order order ) {
    // WARNING: no checking whether username exists
    std::pair<std::pair<int, int>, int> temp(Hash().hash(username), order.order_id);
    interface->order_controller.btree.insert(temp, order, interface->order_controller.btree_file, interface->order_controller.info_file);
    interface->order_controller.order_cnt++;
}