#ifndef FILE_OPERATOR_HPP
#define FILE_OPERATOR_HPP

#include <fstream>

class FileOperator {
public:
	bool create_file( const char *str ) const { // return false if already exist
		std::ifstream file(str);
		if( file ) {
			file.close();
			return false;
		} else {
			std::ofstream of(str);
			of.close();
			return true;
		}
	}
	
	void create_new_file( const char *str ) const {
		std::ofstream file(str);
		file.close();
	}
	
	template<typename T>
	void read( std::fstream &file, int pos, T *p, int cnt ) const {
		file.clear();
		file.seekg(pos);
		file.read( reinterpret_cast<char*>(p), int(cnt * sizeof(T)) );
	}
	
	template<typename T>
	int write( std::fstream &file, int pos, T *p, int cnt ) const { // pos == -1 for the end of file
		file.clear();
		if( pos == -1 ) {
			file.seekp(0, std::ios::end);
		} else {
			file.seekp(pos);
		}
		pos = (int)file.tellp();
		file.write( reinterpret_cast<char*>(p), int(cnt * sizeof(T)) );
		return pos;
	}
	
	int end_pos( std::fstream &file ) const {
		file.clear();
		file.seekg(0, std::ios::end);
		return (int)file.tellg();
	}
};

template<int AVESZ, int BUFSZ>
class LRU {
public:
	
	struct Node {
		std::pair<int,int> info; // pos sz
		char *value;
		Node *pre, *nxt;
		bool dirty;
		
		Node() {
			pre = nxt = nullptr;
			value = nullptr;
			info.first = info.second = -1;
			dirty = false;
		}
		
		void write_file( std::fstream &file ) {
			if( dirty ) {
				file.clear();
				file.seekp( info.first );
				file.write( value, info.second );
				dirty = false;
			}
		}
	};
	
	enum {
		BASE = 19260817,
		MOD = 998244353,
		MAX_TRY = 15,
		MULTI = 19,
		HASHSZ = BUFSZ / (AVESZ + sizeof(Node) + sizeof(Node*) * MULTI) * MULTI,
		MAXSZ = BUFSZ - HASHSZ * sizeof(Node*),
	};
	
	Node *h[HASHSZ], *head, *tail;
	int sz;
	
	LRU() {
		std::cerr << "HASHSZ : " << HASHSZ << std::endl;
		std::cerr << "MAXSZ / AVESZ : " << MAXSZ / (AVESZ + sizeof(Node)) << std::endl;
		head = tail = nullptr;
		sz = 0;
		memset(h, 0, sizeof h);
	}
	
	bool eq( std::pair<int,int> info1, std::pair<int,int> info2 ) {
		if( info1.first == info2.first ) {
			assert( info1.second == info2.second );
			return true;
		} else {
			return false;
		}
	}
	
	int query_pt( std::pair<int,int> info ) {
		for( int i = 0, hv = hsh(info); i < MAX_TRY; ++i, hv = adv(hv) ) {
			if( h[hv % HASHSZ] && eq( h[hv % HASHSZ]->info, info ) ) {
				return hv % HASHSZ;
			}
		}
		return -1;
	}
	
	Node *query( std::pair<int,int> info ) {
		int pos = query_pt(info);
		if( pos == -1 ) {
			return nullptr;
		} else {
			Node *p = h[pos];
			
			// remove
			if( p == head && p == tail ) {
				head = tail = nullptr;
			} else if( p == head ) {
				head = p->nxt;
				head->pre = nullptr;
			} else if( p == tail ) {
				tail = p->pre;
				tail->nxt = nullptr;
			} else {
				Node *pre = p->pre, *nxt = p->nxt;
				pre->nxt = nxt;
				nxt->pre = pre;
			}
			p->pre = p->nxt = nullptr;
			
			// insert
			if( head == nullptr && tail == nullptr ) {
				head = h[pos];
				tail = h[pos];
			} else {
				tail->nxt = h[pos];
				h[pos]->pre = tail;
				tail = h[pos];
			}
			
			return p;
		}
	}
	
	Node *insert( std::pair<int,int> info, std::fstream &file ) {
		while( sz + info.second + int(sizeof(Node)) > MAXSZ ) {
			assert( head != nullptr );
			remove( head->info, file );
		}
		int pos = -1;
		for( int i = 0, hv = hsh(info); i < MAX_TRY; ++i, hv = adv(hv) ) {
			if( h[hv % HASHSZ] == nullptr ) {
				pos = hv % HASHSZ;
				break;
			}
		}
		if( pos == -1 ) { // Hash Confliction
			// std::cerr << "hash confliction" << std::endl;
			return nullptr;
		}
		// std::cerr << "hash success" << std::endl;
		h[pos] = new Node();
		h[pos]->info = info;
		h[pos]->value = new char[info.second];
		sz += int(info.second + sizeof(Node));
		if( head == nullptr ) {
			assert( tail == nullptr );
			head = h[pos];
			tail = h[pos];
		} else {
			tail->nxt = h[pos];
			h[pos]->pre = tail;
			tail = h[pos];
		}
		return tail;
	}
	
	void remove( std::pair<int,int> info, std::fstream &file ) {
		int pos = query_pt(info);
		if( pos == -1 ) {
			return;
		} else {
			Node *p = h[pos];
			h[pos] = nullptr;
			sz -= int(p->info.second + sizeof(Node));
			if( p == head && p == tail ) {
				head = tail = nullptr;
				p->write_file(file);
				delete []p->value;
				delete p;
			} else if( p == head ) {
				head = p->nxt;
				head->pre = nullptr;
				p->write_file(file);
				delete []p->value;
				delete p;
			} else if( p == tail ) {
				tail = p->pre;
				tail->nxt = nullptr;
				p->write_file(file);
				delete []p->value;
				delete p;
			} else {
				Node *pre = p->pre, *nxt = p->nxt;
				pre->nxt = nxt;
				nxt->pre = pre;
				p->write_file(file);
				delete []p->value;
				delete p;
			}
		}
	}
	
	int hsh( std::pair<int,int> info ) {
		return info.first;
	}
	
	int adv( int x ) {
		// return x+1;
		return int((1LL * x * x + 1) % MOD);
	}

	void read( std::fstream &file, int pos, char *p, int len ) {
		std::pair<int,int> info(pos, len);
		Node *src = query(info);
		if( src ) {
			memcpy(p, src->value, len);
		} else {
			file.clear();
			file.seekg(pos);
			file.read(p, len);
			src = insert(info, file);
			if( src ) {
				memcpy(src->value, p, len);
			}
		}
	}
	
	void write( std::fstream &file, int pos, char *p, int len ) {
		std::pair<int,int> info(pos, len);
		Node *src = query(info);
		if( src ) {
			src->dirty = true;
			memcpy(src->value, p, len);
		} else {
			src = insert(info, file);
			if( src ) {
				src->dirty = true;
				memcpy(src->value, p, len);
			} else {
				file.clear();
				file.seekp(pos);
				file.write(p, len);
			}
		}
	}
	
	void write_cache( std::fstream &file ) {
		for( Node *p = head; p; p = p->nxt )
			if( p->dirty ) {
				file.clear();
				file.seekp( p->info.first );
				file.write( p->value, p->info.second );
				p->dirty = false;
			}
	}
};

template<int AVESZ, int BUFSZ = 250250>
class CachedFileOperator {
public:
	LRU<AVESZ, BUFSZ> lru;
	
	template<typename T>
	void read( std::fstream &file, int pos, T *pt, int cnt ) {
		char *p = reinterpret_cast<char*>(pt);
		int sz = int(cnt * sizeof(T));
		lru.read(file, pos, p, sz);
	}
	
	template<typename T>
	int write( std::fstream &file, int pos, T *pt, int cnt ) { // pos == -1 for the end of file
		if( pos == -1 || pos == FileOperator().end_pos(file) ) {
			file.clear();
			file.seekp(0, std::ios::end);
			pos = (int)file.tellp();
			file.write( reinterpret_cast<char*>(pt), int(cnt * sizeof(T)) );
			return pos;
		} else {
			char *p = reinterpret_cast<char*>(pt);
			int sz = int(cnt * sizeof(T));
			lru.write(file, pos, p, sz);
			return pos;
		}
	}
	
	void write_cache( std::fstream &file ) {
		lru.write_cache(file);
	}
};

#endif // FILE_OPERATOR_HPP
