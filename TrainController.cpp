#include "includes.hpp"

void TrainController::add_train( const char train_id[],
                                 int station_num,
                                 int seat_num,
                                 const char stations[][STATION_LEN],
                                 int prices[],
                                 Time start_time,
                                 int travel_times[],
                                 int stopover_times[],
                                 Date sale_date_begin,
                                 Date sale_date_end,
                                 char type ) {
    // FAILURE if train_id exists
    if (btree.exist(Hash().hash(train_id), btree_file, info_file)) {printf("-1\n"); return;}
    if (interface->train_controller_released.btree.exist(Hash().hash(train_id), interface->train_controller_released.btree_file, interface->train_controller_released.info_file)) {printf("-1\n"); return;}

	train_cnt++;
    Train todo_train(train_id, station_num, seat_num, stations, prices, start_time, travel_times, stopover_times, sale_date_begin, sale_date_end, type, train_cnt);
    btree.insert(Hash().hash(train_id), todo_train, btree_file, info_file);
    
    printf("0\n");
}

void TrainController::delete_train( const char train_id[] ) {
    // FAILURE if train_id doesn't exist
    if (!btree.exist(Hash().hash(train_id), btree_file, info_file)) {printf("-1\n"); return;}

    btree.remove(Hash().hash(train_id), btree_file);
    //train_cnt--;
    printf("0\n");
}

void TrainController::load( Interface *ifs, const char *id_filename, const char *info_filename ) {
    interface = ifs;
    info_file.open(info_filename);
    btree_file.open(id_filename);
    FileOperator fop;
    std::fstream cnt_file;
    cnt_file.open("counts");
    if (strcmp(id_filename, "train_id_unreleased") == 0) {// train_cnt(unreleased)
        fop.read(cnt_file, 8, &train_cnt, 1);
    }
    else {// train_cnt(released)
        fop.read(cnt_file, 12, &train_cnt, 1);
    }
    cnt_file.close();
}

void TrainController::save( int pos ) {
    info_file.close();
    btree_file.close();
    FileOperator fop;
    std::fstream cnt_file;
    cnt_file.open("counts");
    fop.write(cnt_file, pos, &train_cnt, 1);
    cnt_file.close();
}