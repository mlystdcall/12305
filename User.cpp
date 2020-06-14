#include "includes.hpp"

User::User( const char _username[],
            const char _password[],
            const char _name[],
            const char _mail_addr[],
            int _privilege,
			int _create_time,
			int _order_cnt,
			int _is_login ):
			privilege(_privilege),
			create_time(_create_time),
			order_cnt(_order_cnt),
			is_login(_is_login) {
    strcpy(username,  _username);
    strcpy(password,  _password);
    strcpy(name,      _name);
    strcpy(mail_addr, _mail_addr);
}

/*User::User( const User &other ):
            privilege(other.privilege),
            create_time(other.create_time),
            order_cnt(other.order_cnt),
            is_login(other.is_login)
                {
    strcpy(username,  other.username);
    strcpy(password,  other.password);
    strcpy(name,      other.name);
    strcpy(mail_addr, other.mail_addr);
}*/

User::~User() {}

/*User &User::operator=( const User &other ) {
    if (this == &other) return *this;
    privilege   = other.privilege;
    create_time = other.create_time;
    order_cnt   = other.order_cnt;
    is_login    = other.is_login;
    strcpy(username,  other.username);
    strcpy(password,  other.password);
    strcpy(name,      other.name);
    strcpy(mail_addr, other.mail_addr);
}*/