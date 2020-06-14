#include "includes.hpp"

void OrderController::load( Interface *ifs ) {
    interface = ifs;
    btree_file.open("order_btree");
    info_file.open("order_info");
    FileOperator fop;
    std::fstream cnt_file;
    cnt_file.open("counts");
    fop.read(cnt_file, 4, &order_cnt, 1);
    cnt_file.close();
}

void OrderController::save() {
    btree_file.close();
    info_file.close();
    FileOperator fop;
    std::fstream cnt_file;
    cnt_file.open("counts");
    fop.write(cnt_file, 4, &order_cnt, 1);
    cnt_file.close();
}