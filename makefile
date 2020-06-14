main : BTree.cpp \
		Date.cpp \
		FileOperator.cpp \
		Interface.cpp \
		main.cpp \
		Order.cpp \
		OrderController.cpp \
		ReleasedTrain.cpp \
		ReleasedTrainController.cpp \
		TicketController.cpp \
		Time.cpp \
		Train.cpp \
		TrainController.cpp \
		User.cpp \
		UserController.cpp \
		Hash.cpp
	g++ BTree.cpp \
		Date.cpp \
		FileOperator.cpp \
		Interface.cpp \
		main.cpp \
		Order.cpp \
		OrderController.cpp \
		ReleasedTrain.cpp \
		ReleasedTrainController.cpp \
		TicketController.cpp \
		Time.cpp \
		Train.cpp \
		TrainController.cpp \
		User.cpp \
		UserController.cpp \
		Hash.cpp \
		-o code -Wall -Wextra -Wconversion -Wshadow \
		-std=c++11 -O2
