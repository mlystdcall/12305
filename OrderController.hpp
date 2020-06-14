#ifndef ORDER_CONTROLLER_HPP
#define ORDER_CONTROLLER_HPP

#include <fstream>
#include <iostream>
#include "BTree.hpp"
#include "Order.hpp"

class Interface;

class OrderController {
public:
	int order_cnt;
	std::fstream btree_file;
	std::fstream info_file;
	Interface *interface;
	BTree<std::pair<std::pair<int, int>, int>, Order> btree;

	void load( Interface *ifs );
	void save();
};

#endif // ORDER_CONTROLLER_HPP
