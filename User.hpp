#ifndef USER_HPP
#define USER_HPP

#include "const_variable.hpp"
#include "Order.hpp"

class User {
public:
	char username[USERNAME_LEN];
	char password[PASSWORD_LEN];
	char name[NAME_LEN];
	char mail_addr[MAIL_ADDR_LEN];
	int privilege;
	int create_time; //
	int order_cnt;
	int is_login; //

	User( const char _username[],
		const char _password[],
		const char _name[],
		const char _mail_addr[],
		int _privilege,
		int _create_time,
		int _order_cnt,
		int _is_login );
	User() {}
	// User( const User &other );
	~User();
	// User &operator=( const User &other );
};

#endif // USER_HPP
