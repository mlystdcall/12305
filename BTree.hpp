#ifndef BTREE_HPP
#define BTREE_HPP

#include <fstream>
#include <utility>
#include "FileOperator.hpp"

template<typename Key, typename Value, int BUFSZ>
class LRU {
public:
	
	struct Node {
		Key key;
		Value value;
		Node *pre, *nxt;
		
		Node() {
			pre = nxt = nullptr;
		}
	};
	
	enum {
		BASE = 19260817,
		MOD = 998244353,
		MAX_TRY = 10,
		MULTI = 5,
		MAXSZ = (BUFSZ - sizeof(Node*) * 2 - sizeof(int)) / (sizeof(Node*) * MULTI + sizeof(Node)),
		HASHSZ = MAXSZ * MULTI,
	};
	
	Node *h[HASHSZ], *head, *tail;
	int cnt;
	
	LRU() {
		head = tail = nullptr;
		cnt = 0;
		memset(h, 0, sizeof h);
	}
	
	bool eq( const Key &k1, const Key &k2 ) {
		return !(k1 < k2) && !(k2 < k1);
	}

	int query_pt( Key key ) {
		for( int i = 0, hv = hsh(&key); i < MAX_TRY; ++i, hv = adv(hv) ) {
			if( h[hv % HASHSZ] && eq( h[hv % HASHSZ]->key, key ) ) {
				return hv % HASHSZ;
			}
		}
		return -1;
	}
	
	std::pair<bool,Value> query( const Key &key ) {
		int pos = query_pt(key);
		if( pos == -1 ) {
			return std::make_pair(false, Value());
		} else {
			Value value = h[pos]->value;
			remove(key);
			insert(key, value);
			return std::make_pair(true, value);
		}
	}
	
	void insert( Key key, const Value &value ) {
		int pos = -1;
		for( int i = 0, hv = hsh(&key); i < MAX_TRY; ++i, hv = adv(hv) ) {
			if( h[hv % HASHSZ] == nullptr ) {
				pos = hv % HASHSZ;
				break;
			}
		}
		if( pos == -1 ) {
			return;
			assert( pos != -1 ); // Hash Confliction
		}
		h[pos] = new Node();
		h[pos]->key = key;
		h[pos]->value = value;
		++cnt;
		if( head == nullptr && tail == nullptr ) {
			head = h[pos];
			tail = h[pos];
		} else {
			tail->nxt = h[pos];
			h[pos]->pre = tail;
			tail = h[pos];
		}
		if( cnt == MAXSZ + 1 ) {
			pos = query_pt( head->key );
			head = head->nxt;
			head->pre = nullptr;
			--cnt;
			delete h[pos];
			h[pos] = nullptr;
		}
	}
	
	void remove( const Key &key ) {
		int pos = query_pt(key);
		if( pos == -1 ) {
			return;
		} else {
			Node *p = h[pos];
			h[pos] = nullptr;
			--cnt;
			if( p == head && p == tail ) {
				head = tail = nullptr;
				delete p;
			} else if( p == head ) {
				head = p->nxt;
				head->pre = nullptr;
				delete p;
			} else if( p == tail ) {
				tail = p->pre;
				tail->nxt = nullptr;
				delete p;
			} else {
				Node *pre = p->pre, *nxt = p->nxt;
				pre->nxt = nxt;
				nxt->pre = pre;
				delete p;
			}
		}
	}
	
	template<typename T>
	int hsh( T *data ) {
		unsigned char *p = reinterpret_cast<unsigned char*>(data);
		int len = sizeof(T);
		int ans = 0;
		for( int i = 0; i < len; ++i, ++p )
			ans = int((1LL * ans * BASE + *p) % MOD);
		return ans;
	}
	
	int adv( int x ) {
		// return (x + 1) % MOD;
		return int((1LL * x * x + 1) % MOD);
	}
};

template<typename Key, typename Value, int BLKSZ = 4096, int BUFSZ = 800000>
class BTree {
public:
	enum {
		KeyCnt = (BLKSZ - sizeof(bool) - sizeof(int)) / (sizeof(Key) + sizeof(int)),
		ValueCnt = (BLKSZ - sizeof(int) * 3) / sizeof(std::pair<Key,int>),
		KeyLim = KeyCnt / 2 - 1,
		ValueLim = ValueCnt / 2 - 1,
	};
	
	void insert( Key key,
				 Value value,
				 std::fstream &btree_file,
				 std::fstream &value_file );
	void remove( Key key,
				 std::fstream &btree_file );
	void modify( Key key,
				 Value value,
				 std::fstream &btree_file,
				 std::fstream &value_file );
	int query_list( Key key_left,
					Key key_right,
					std::fstream &btree_file,
					std::fstream &value_file,
					std::pair<Key, Value> *&value ); // inside new outside delete
	Value query( Key key,
				 std::fstream &btree_file,
				 std::fstream &value_file );
	bool exist( Key key,
				std::fstream &btree_file,
				std::fstream &value_file );
	
private:
	
	class Node {
	public:
		Key key[KeyCnt];
		int ch[KeyCnt];
		bool leaf;
		int cnt;
		
		Node() {
			leaf = false;
			cnt = 0;
		}
	};
	
	class Leaf {
	public:
		std::pair<Key,int> value[ValueCnt];
		int cnt;
		int pre, nxt;
		
		Leaf() {
			cnt = 0;
			pre = nxt = -1;
		}
	};
	
	FileOperator fop;
	LRU<Key, Value, BUFSZ> lru;
	
	std::tuple<bool,int,int> insert_leaf( int leaf_pos,
										  Key key,
										  Value value,
										  std::fstream &btree_file,
										  std::fstream &value_file );
	std::tuple<bool,int,int> insert_node( int node_pos,
										  Key key,
										  Value value,
										  std::fstream &btree_file,
										  std::fstream &value_file );
	Key get_min_leaf( int leaf_pos, std::fstream &btree_file );
	Key get_min_node( int node_pos, std::fstream &btree_file );
	bool remove_leaf( int leaf_pos, Key key, std::fstream &btree_file );
	bool remove_node( int node_pos, Key key, std::fstream &btree_file );
	int query_leaf_pos( Key key, std::fstream &btree_file );
	int query_leaf_pos_helper( int node_pos, Key key, std::fstream &btree_file );
	Value query_leaf( int leaf_pos,
					  Key key,
					  std::fstream &btree_file,
					  std::fstream &value_file );
	Value query_node( int node_pos,
					  Key key,
					  std::fstream &btree_file,
					  std::fstream &value_file );
	bool exist_leaf( int leaf_pos, Key key, std::fstream &btree_file );
	bool exist_node( int node_pos, Key key, std::fstream &btree_file );
	bool between( Key key, Key left_key, Key right_key );
};

template<typename Key, typename Value, int BLKSZ, int BUFSZ>
void BTree<Key, Value, BLKSZ, BUFSZ> :: insert( Key key,
										 Value value,
										 std::fstream &btree_file,
										 std::fstream &value_file ) {
	if( fop.end_pos( btree_file ) == 0 ) { // new file
		// std::cerr << "new file" << std::endl;
		Node root;
		fop.write( btree_file, -1, &root, 1 );
		assert( fop.end_pos( btree_file ) == sizeof(Node) );
		
		Leaf root_leaf;
		fop.write( btree_file, -1, &root_leaf, 1 );
		assert( fop.end_pos( btree_file ) == sizeof(Node) + sizeof(Leaf) );
	}
	
	Node root;
	fop.read( btree_file, 0, &root, 1 );
	if( root.cnt == 0 ) { // root is leaf
		
		std::tuple<bool,int,int> tmp = insert_leaf( sizeof(Node),
													key,
													value,
													btree_file,
													value_file );
		if( std::get<0>(tmp) == false ) {
			root.cnt = 2;
			root.leaf = true;
			root.ch[0] = std::get<1>(tmp);
			root.ch[1] = std::get<2>(tmp);
			root.key[0] = get_min_leaf( root.ch[0], btree_file );
			root.key[1] = get_min_leaf( root.ch[1], btree_file );
			// std::cerr << "split root " << root.key[0] << ' ' << root.ch[0] << std::endl;
			// std::cerr << "split root " << root.key[1] << ' ' << root.ch[1] << std::endl;
			fop.write( btree_file, 0, &root, 1 );
		}
		
	} else { // root is node
		
		std::tuple<bool,int,int> tmp = insert_node( 0,
													key,
													value,
													btree_file,
													value_file );
		if( std::get<0>(tmp) == false ) {
			root.cnt = 2;
			root.leaf = false;
			root.ch[0] = std::get<1>(tmp);
			root.ch[1] = std::get<2>(tmp);
			root.key[0] = get_min_node( root.ch[0], btree_file );
			root.key[1] = get_min_node( root.ch[1], btree_file );
			fop.write( btree_file, 0, &root, 1 );
		}
	}
	
	// buffer operation
	lru.remove(key);
	lru.insert(key, value);
}

template<typename Key, typename Value, int BLKSZ, int BUFSZ>
void BTree<Key, Value, BLKSZ, BUFSZ> :: remove( Key key,
										 std::fstream &btree_file ) {
	if( fop.end_pos( btree_file ) == 0 ) { // new file
		return;
	} else {
		Node root;
		fop.read( btree_file, 0, &root, 1 );
		if( root.cnt == 0 ) { // root is leaf
			remove_leaf( sizeof(Node), key, btree_file );
		} else {
			remove_node( 0, key, btree_file );
			fop.read( btree_file, 0, &root, 1 );
			if( root.cnt == 1 ) {
				if( root.leaf ) {
					Leaf leaf;
					fop.read( btree_file, root.ch[0], &leaf, 1 );
					fop.write( btree_file, sizeof(Node), &leaf, 1 );
					assert( leaf.pre == -1 && leaf.nxt == -1 );
					root.cnt = 0;
					fop.write( btree_file, 0, &root, 1 );
				} else {
					fop.read( btree_file, root.ch[0], &root, 1 );
					fop.write( btree_file, 0, &root, 1 );
				}
			} else {
				;
			}
		}
		
		// buffer operation
		lru.remove(key);
	}
}

template<typename Key, typename Value, int BLKSZ, int BUFSZ>
void BTree<Key, Value, BLKSZ, BUFSZ> :: modify( Key key,
										 Value value,
										 std::fstream &btree_file,
										 std::fstream &value_file ) {
	insert(key, value, btree_file, value_file);
}

template<typename Key, typename Value, int BLKSZ, int BUFSZ>
int BTree<Key, Value, BLKSZ, BUFSZ> :: query_list( Key key_left,
											Key key_right,
											std::fstream &btree_file,
											std::fstream &value_file,
											std::pair<Key, Value> *&value ) { // inside new outside delete
	if( fop.end_pos( btree_file ) == 0 ) { // new file
		value = nullptr;
		return 0;
	} else {
		if( key_right < key_left ) {
			value = nullptr;
			return 0;
		} else {
			int left_leaf = query_leaf_pos( key_left, btree_file );
			int right_leaf = query_leaf_pos( key_right, btree_file );
			
			int cnt = 0;
			while(1) {
				Leaf leaf;
				fop.read( btree_file, left_leaf, &leaf, 1 );
				for( int i = 0; i < leaf.cnt; ++i ) {
					// std::cerr << "left_leaf = " << left_leaf << " key = " << leaf.value[i].first << std::endl;
					if( between( leaf.value[i].first, key_left, key_right ) ) {
						++cnt;
					}
				}
				// std::cerr << "nxt = " << leaf.nxt << std::endl;
				if( left_leaf == right_leaf ) {
					break;
				} else {
					assert( leaf.nxt != -1 );
					left_leaf = leaf.nxt;
				}
			}
			
			if( cnt == 0 ) {
				value = nullptr;
				return 0;
			} else {
				value = new std::pair<Key,Value>[cnt];
				int now = 0;
				
				left_leaf = query_leaf_pos( key_left, btree_file );
				while(1) {
					Leaf leaf;
					fop.read( btree_file, left_leaf, &leaf, 1 );
					for( int i = 0; i < leaf.cnt; ++i ) {
						if( between( leaf.value[i].first, key_left, key_right ) ) {
							value[now].first = leaf.value[i].first;
							fop.read( value_file, leaf.value[i].second, &value[now].second, 1 );
							++now;
						}
					}
					if( left_leaf == right_leaf ) {
						break;
					} else {
						assert( leaf.nxt != -1 );
						left_leaf = leaf.nxt;
					}
				}
				assert( now == cnt );
				return cnt;
			}
		}
	}
}

template<typename Key, typename Value, int BLKSZ, int BUFSZ>
Value BTree<Key, Value, BLKSZ, BUFSZ> :: query( Key key,
										 std::fstream &btree_file,
										 std::fstream &value_file ) {
	if( fop.end_pos( btree_file ) == 0 ) { // new file
		assert(0); // not existed
		return Value();
	} else {
		// buffer operation
		auto buffer_ans = lru.query(key);
		if( buffer_ans.first )
			return buffer_ans.second;
		
		Node root;
		fop.read( btree_file, 0, &root, 1 );
		Value ans;
		if( root.cnt == 0 ) { // root is leaf
			ans = query_leaf( sizeof(Node),
							  key,
							  btree_file,
							  value_file );
		} else { // root is node
			ans = query_node( 0,
							  key,
							  btree_file,
							  value_file );
		}
		
		// buffer operation
		lru.insert(key, ans);
		
		return ans;
	}
}

template<typename Key, typename Value, int BLKSZ, int BUFSZ>
bool BTree<Key, Value, BLKSZ, BUFSZ> :: exist( Key key,
											   std::fstream &btree_file,
											   std::fstream &value_file ) {
	if( fop.end_pos( btree_file ) == 0 ) { // new file
		return false;
	} else {
		// buffer operation
		auto buffer_ans = lru.query(key);
		if( buffer_ans.first )
			return true;
		
		Node root;
		fop.read( btree_file, 0, &root, 1 );
		bool ans;
		if( root.cnt == 0 ) { // root is leaf
			ans = exist_leaf( sizeof(Node), key, btree_file );
		} else { // root is node
			ans = exist_node( 0, key, btree_file );
		}
		
		// buffer operation
		if( ans ) query(key, btree_file, value_file); // load to buffer
		
		return ans;
	}
}

template<typename Key, typename Value, int BLKSZ, int BUFSZ>
std::tuple<bool,int,int> BTree<Key, Value, BLKSZ, BUFSZ> :: insert_leaf( int leaf_pos,
																  Key key,
																  Value value,
																  std::fstream &btree_file,
																  std::fstream &value_file ) {
	Leaf leaf;
	fop.read( btree_file, leaf_pos, &leaf, 1 );
	assert( leaf.cnt < ValueCnt );
	for( int i = 0; i < leaf.cnt; ++i ) {
		if( between( leaf.value[i].first, key, key ) ) {
			fop.write( value_file, leaf.value[i].second, &value, 1 );
			return std::make_tuple(true, -1, -1);
		}
	}
	bool biggest = true;
	for( int i = 0; i < leaf.cnt; ++i ) {
		if( key < leaf.value[i].first ) {
			for( int j = leaf.cnt-1; j >= i; --j )
				leaf.value[j+1] = leaf.value[j];
			leaf.value[i].first = key;
			leaf.value[i].second = fop.write( value_file, -1, &value, 1 );
			++leaf.cnt;
			biggest = false;
			break;
		}
	}
	if( biggest == true ) {
		leaf.value[leaf.cnt].first = key;
		leaf.value[leaf.cnt].second = fop.write( value_file, -1, &value, 1 );
		++leaf.cnt;
	}
	fop.write( btree_file, leaf_pos, &leaf, 1 );
	// std::cerr << "leaf.cnt = " << leaf.cnt << std::endl;
	if( leaf.cnt == ValueCnt ) {
		Leaf left_part;
		Leaf right_part;
		int mid = ValueCnt / 2;
		for( int i = 0; i < mid; ++i )
			left_part.value[i] = leaf.value[i];
		for( int i = mid; i < leaf.cnt; ++i )
			right_part.value[i-mid] = leaf.value[i];
		left_part.cnt = mid;
		right_part.cnt = leaf.cnt - mid;
		int left_pos = fop.write( btree_file, -1, &left_part, 1 );
		int right_pos = fop.write( btree_file, -1, &right_part, 1 );
		left_part.pre = leaf.pre;
		left_part.nxt = right_pos;
		right_part.pre = left_pos;
		right_part.nxt = leaf.nxt;
		fop.write( btree_file, left_pos, &left_part, 1 );
		fop.write( btree_file, right_pos, &right_part, 1 );
		if( left_part.pre != -1 ) {
			Leaf tmp;
			fop.read( btree_file, left_part.pre, &tmp, 1 );
			tmp.nxt = left_pos;
			fop.write( btree_file, left_part.pre, &tmp, 1 );
		}
		if( right_part.nxt != -1 ) {
			Leaf tmp;
			fop.read( btree_file, right_part.nxt, &tmp, 1 );
			tmp.pre = right_pos;
			fop.write( btree_file, right_part.nxt, &tmp, 1 );
		}
		return std::make_tuple(false, left_pos, right_pos);
	} else {
		return std::make_tuple(true, -1, -1);
	}
}

template<typename Key, typename Value, int BLKSZ, int BUFSZ>
std::tuple<bool,int,int> BTree<Key, Value, BLKSZ, BUFSZ> :: insert_node( int node_pos,
																  Key key,
																  Value value,
																  std::fstream &btree_file,
																  std::fstream &value_file ) {
	Node node;
	fop.read( btree_file, node_pos, &node, 1 );
	assert( node.cnt < KeyCnt );
	bool biggest = true;
	for( int i = 1; i < node.cnt; ++i ) {
		if( key < node.key[i] ) {
			std::tuple<bool,int,int> tmp;
			if( node.leaf ) {
				tmp = insert_leaf( node.ch[i-1], key, value, btree_file, value_file );
			} else {
				tmp = insert_node( node.ch[i-1], key, value, btree_file, value_file );
			}
			biggest = false;
			if( std::get<0>(tmp) == true ) {
				;
			} else {
				for( int j = node.cnt-1; j >= i; --j ) {
					node.key[j+1] = node.key[j];
					node.ch[j+1] = node.ch[j];
				}
				++node.cnt;
				node.ch[i-1] = std::get<1>(tmp);
				node.ch[i] = std::get<2>(tmp);
				if( node.leaf ) {
					node.key[i-1] = get_min_leaf( node.ch[i-1], btree_file );
					node.key[i] = get_min_leaf( node.ch[i], btree_file );
				} else {
					node.key[i-1] = get_min_node( node.ch[i-1], btree_file );
					node.key[i] = get_min_node( node.ch[i], btree_file );
				}
			}
			break;
		}
	}
	if( biggest == true ) {
		std::tuple<bool,int,int> tmp;
		if( node.leaf ) {
			tmp = insert_leaf( node.ch[node.cnt-1], key, value, btree_file, value_file );
		} else {
			tmp = insert_node( node.ch[node.cnt-1], key, value, btree_file, value_file );
		}
		if( std::get<0>(tmp) == true ) {
			;
		} else {
			node.ch[node.cnt-1] = std::get<1>(tmp);
			node.ch[node.cnt] = std::get<2>(tmp);
			if( node.leaf ) {
				node.key[node.cnt-1] = get_min_leaf( node.ch[node.cnt-1], btree_file );
				node.key[node.cnt] = get_min_leaf( node.ch[node.cnt], btree_file );
			} else {
				node.key[node.cnt-1] = get_min_node( node.ch[node.cnt-1], btree_file );
				node.key[node.cnt] = get_min_node( node.ch[node.cnt], btree_file );
			}
			++node.cnt;
		}
	}
	fop.write( btree_file, node_pos, &node, 1 );
	if( node.cnt == KeyCnt ) {
		Node left_part;
		Node right_part;
		int mid = KeyCnt / 2;
		for( int i = 0; i < mid; ++i ) {
			left_part.key[i] = node.key[i];
			left_part.ch[i] = node.ch[i];
		}
		for( int i = mid; i < node.cnt; ++i ) {
			right_part.key[i-mid] = node.key[i];
			right_part.ch[i-mid] = node.ch[i];
		}
		left_part.cnt = mid;
		right_part.cnt = node.cnt - mid;
		left_part.leaf = right_part.leaf = node.leaf;
		int left_pos = fop.write( btree_file, -1, &left_part, 1 );
		int right_pos = fop.write( btree_file, -1, &right_part, 1 );
		return std::make_tuple(false, left_pos, right_pos);
	} else {
		return std::make_tuple(true, -1, -1);
	}
}

template<typename Key, typename Value, int BLKSZ, int BUFSZ>
Key BTree<Key, Value, BLKSZ, BUFSZ> :: get_min_leaf( int leaf_pos, std::fstream &btree_file ) {
	Leaf leaf;
	fop.read( btree_file, leaf_pos, &leaf, 1 );
	return leaf.value[0].first;
}

template<typename Key, typename Value, int BLKSZ, int BUFSZ>
Key BTree<Key, Value, BLKSZ, BUFSZ> :: get_min_node( int node_pos, std::fstream &btree_file ) {
	Node node;
	fop.read( btree_file, node_pos, &node, 1 );
	return node.key[0];
}

template<typename Key, typename Value, int BLKSZ, int BUFSZ>
bool BTree<Key, Value, BLKSZ, BUFSZ> :: remove_leaf( int leaf_pos, Key key, std::fstream &btree_file ) {
	Leaf leaf;
	fop.read( btree_file, leaf_pos, &leaf, 1 );
	for( int i = 0; i < leaf.cnt; ++i ) {
		if( between( leaf.value[i].first, key, key ) ) {
			for( int j = i+1; j < leaf.cnt; ++j )
				leaf.value[j-1] = leaf.value[j];
			--leaf.cnt;
			break;
		}
	}
	fop.write( btree_file, leaf_pos, &leaf, 1 );
	return leaf.cnt <= ValueLim;
}

template<typename Key, typename Value, int BLKSZ, int BUFSZ>
bool BTree<Key, Value, BLKSZ, BUFSZ> :: remove_node( int node_pos, Key key, std::fstream &btree_file ) {
	Node node;
	fop.read( btree_file, node_pos, &node, 1 );
	assert( node.cnt > 1 );
	bool biggest = true;
	for( int i = 1; i < node.cnt; ++i ) {
		if( key < node.key[i] ) {
			bool need_op;
			if( node.leaf ) {
				need_op = remove_leaf( node.ch[i-1], key, btree_file );
			} else {
				need_op = remove_node( node.ch[i-1], key, btree_file );
			}
			biggest = false;
			if( need_op ) {
				if( node.leaf ) {
					Leaf now_leaf;
					Leaf right_leaf;
					fop.read( btree_file, node.ch[i-1], &now_leaf, 1 );
					fop.read( btree_file, node.ch[i], &right_leaf, 1 );
					if( right_leaf.cnt == ValueLim + 1 ) { // merge
						assert( now_leaf.cnt == ValueLim );
						for( int j = 0; j < right_leaf.cnt; ++j )
							now_leaf.value[now_leaf.cnt + j] = right_leaf.value[j];
						now_leaf.cnt += right_leaf.cnt;
						assert( now_leaf.cnt == ValueLim + ValueLim + 1 );
						now_leaf.nxt = right_leaf.nxt;
						if( now_leaf.nxt != -1 ) {
							Leaf tmp;
							fop.read( btree_file, now_leaf.nxt, &tmp, 1 );
							tmp.pre = node.ch[i-1];
							fop.write( btree_file, now_leaf.nxt, &tmp, 1 );
						}
						fop.write( btree_file, node.ch[i-1], &now_leaf, 1 );
						node.key[i-1] = get_min_leaf( node.ch[i-1], btree_file );
						for( int j = i+1; j < node.cnt; ++j ) {
							node.key[j-1] = node.key[j];
							node.ch[j-1] = node.ch[j];
						}
						--node.cnt;
					} else if( right_leaf.cnt > ValueLim + 1 ) { // borrow
						now_leaf.value[now_leaf.cnt++] = right_leaf.value[0];
						for( int j = 1; j < right_leaf.cnt; ++j )
							right_leaf.value[j-1] = right_leaf.value[j];
						--right_leaf.cnt;
						fop.write( btree_file, node.ch[i-1], &now_leaf, 1 );
						fop.write( btree_file, node.ch[i], &right_leaf, 1 );
						node.key[i-1] = get_min_leaf( node.ch[i-1], btree_file );
						node.key[i] = get_min_leaf( node.ch[i], btree_file );
					} else {
						assert(0); // right_leaf.cnt <= ValueLim
					}
				} else {
					Node now_node;
					Node right_node;
					fop.read( btree_file, node.ch[i-1], &now_node, 1 );
					fop.read( btree_file, node.ch[i], &right_node, 1 );
					if( right_node.cnt == KeyLim + 1 ) { // merge
						assert( now_node.cnt == KeyLim );
						for( int j = 0; j < right_node.cnt; ++j ) {
							now_node.key[now_node.cnt + j] = right_node.key[j];
							now_node.ch[now_node.cnt + j] = right_node.ch[j];
						}
						now_node.cnt += right_node.cnt;
						assert( now_node.cnt == KeyLim + KeyLim + 1 );
						fop.write( btree_file, node.ch[i-1], &now_node, 1 );
						node.key[i-1] = get_min_node( node.ch[i-1], btree_file );
						for( int j = i+1; j < node.cnt; ++j ) {
							node.key[j-1] = node.key[j];
							node.ch[j-1] = node.ch[j];
						}
						--node.cnt;
					} else if( right_node.cnt > KeyLim + 1 ) { // borrow
						now_node.key[now_node.cnt] = right_node.key[0];
						now_node.ch[now_node.cnt] = right_node.ch[0];
						++now_node.cnt;
						for( int j = 1; j < right_node.cnt; ++j ) {
							right_node.key[j-1] = right_node.key[j];
							right_node.ch[j-1] = right_node.ch[j];
						}
						--right_node.cnt;
						fop.write( btree_file, node.ch[i-1], &now_node, 1 );
						fop.write( btree_file, node.ch[i], &right_node, 1 );
						node.key[i-1] = get_min_node( node.ch[i-1], btree_file );
						node.key[i] = get_min_node( node.ch[i], btree_file );
					} else {
						assert(0); // right_node.cnt <= KeyLim
					}
				}
			} else {
				;
			}
			break;
		}
	}
	if( biggest == true ) {
		bool need_op;
		if( node.leaf ) {
			need_op = remove_leaf( node.ch[node.cnt-1], key, btree_file );
		} else {
			need_op = remove_node( node.ch[node.cnt-1], key, btree_file );
		}
		if( need_op ) {
			if( node.leaf ) {
				Leaf now_leaf;
				Leaf left_leaf;
				fop.read( btree_file, node.ch[node.cnt-1], &now_leaf, 1 );
				fop.read( btree_file, node.ch[node.cnt-2], &left_leaf, 1 );
				if( left_leaf.cnt == ValueLim + 1 ) { // merge
					assert( now_leaf.cnt == ValueLim );
					for( int j = 0; j < now_leaf.cnt; ++j )
						left_leaf.value[left_leaf.cnt + j] = now_leaf.value[j];
					left_leaf.cnt += now_leaf.cnt;
					assert( left_leaf.cnt == ValueLim + ValueLim + 1 );
					left_leaf.nxt = now_leaf.nxt;
					if( left_leaf.nxt != -1 ) {
						Leaf tmp;
						fop.read( btree_file, left_leaf.nxt, &tmp, 1 );
						tmp.pre = node.ch[node.cnt-2];
						fop.write( btree_file, left_leaf.nxt, &tmp, 1 );
					}
					fop.write( btree_file, node.ch[node.cnt-2], &left_leaf, 1 );
					--node.cnt;
				} else if( left_leaf.cnt > ValueLim + 1 ) { // borrow
					for( int j = now_leaf.cnt-1; j >= 0; --j )
						now_leaf.value[j+1] = now_leaf.value[j];
					now_leaf.value[0] = left_leaf.value[left_leaf.cnt-1];
					--left_leaf.cnt;
					++now_leaf.cnt;
					fop.write( btree_file, node.ch[node.cnt-2], &left_leaf, 1 );
					fop.write( btree_file, node.ch[node.cnt-1], &now_leaf, 1 );
					node.key[node.cnt-2] = get_min_leaf( node.ch[node.cnt-2], btree_file );
					node.key[node.cnt-1] = get_min_leaf( node.ch[node.cnt-1], btree_file );
				} else {
					assert(0); // left_leaf.cnt <= ValueLim
				}
			} else {
				Node now_node;
				Node left_node;
				fop.read( btree_file, node.ch[node.cnt-1], &now_node, 1 );
				fop.read( btree_file, node.ch[node.cnt-2], &left_node, 1 );
				if( left_node.cnt == KeyLim + 1 ) { // merge
					assert( now_node.cnt == KeyLim );
					for( int j = 0; j < now_node.cnt; ++j ) {
						left_node.key[left_node.cnt + j] = now_node.key[j];
						left_node.ch[left_node.cnt + j] = now_node.ch[j];
					}
					left_node.cnt += now_node.cnt;
					assert( left_node.cnt == KeyLim + KeyLim + 1 );
					fop.write( btree_file, node.ch[node.cnt-2], &left_node, 1 );
					--node.cnt;
				} else if( left_node.cnt > KeyLim + 1 ) { // borrow
					for( int j = now_node.cnt-1; j >= 0; --j ) {
						now_node.key[j+1] = now_node.key[j];
						now_node.ch[j+1] = now_node.ch[j];
					}
					now_node.key[0] = left_node.key[left_node.cnt-1];
					now_node.ch[0] = left_node.ch[left_node.cnt-1];
					--left_node.cnt;
					++now_node.cnt;
					fop.write( btree_file, node.ch[node.cnt-2], &left_node, 1 );
					fop.write( btree_file, node.ch[node.cnt-1], &now_node, 1 );
					node.key[node.cnt-2] = get_min_node( node.ch[node.cnt-2], btree_file );
					node.key[node.cnt-1] = get_min_node( node.ch[node.cnt-1], btree_file );
				} else {
					assert(0); // left_node.cnt <= KeyLim
				}
			}
		} else {
			;
		}
	}
	fop.write( btree_file, node_pos, &node, 1 );
	return node.cnt <= KeyLim;
}

template<typename Key, typename Value, int BLKSZ, int BUFSZ>
int BTree<Key, Value, BLKSZ, BUFSZ> :: query_leaf_pos( Key key, std::fstream &btree_file ) {
	assert( fop.end_pos( btree_file ) != 0 );
	Node root;
	fop.read( btree_file, 0, &root, 1 );
	if( root.cnt == 0 ) { // root is leaf
		return sizeof(Node);
	} else { // root is node
		return query_leaf_pos_helper( 0, key, btree_file );
	}
}

template<typename Key, typename Value, int BLKSZ, int BUFSZ>
int BTree<Key, Value, BLKSZ, BUFSZ> :: query_leaf_pos_helper( int node_pos, Key key, std::fstream &btree_file ) {
	Node node;
	fop.read( btree_file, node_pos, &node, 1 );
	for( int i = 1; i < node.cnt; ++i ) {
		if( key < node.key[i] ) {
			if( node.leaf ) {
				return node.ch[i-1];
			} else {
				return query_leaf_pos_helper( node.ch[i-1], key, btree_file );
			}
		}
	}
	if( node.leaf ) {
		return node.ch[node.cnt-1];
	} else {
		return query_leaf_pos_helper( node.ch[node.cnt-1], key, btree_file );
	}
}

template<typename Key, typename Value, int BLKSZ, int BUFSZ>
Value BTree<Key, Value, BLKSZ, BUFSZ> :: query_leaf( int leaf_pos,
											  Key key,
											  std::fstream &btree_file,
											  std::fstream &value_file ) {
	Leaf leaf;
	fop.read( btree_file, leaf_pos, &leaf, 1 );
	for( int i = 0; i < leaf.cnt; ++i ) {
		if( between( leaf.value[i].first, key, key ) ) {
			Value ans;
			fop.read( value_file, leaf.value[i].second, &ans, 1 );
			return ans;
		}
	}
	return assert(0), Value();
}

template<typename Key, typename Value, int BLKSZ, int BUFSZ>
Value BTree<Key, Value, BLKSZ, BUFSZ> :: query_node( int node_pos,
											  Key key,
											  std::fstream &btree_file,
											  std::fstream &value_file ) {
	Node node;
	fop.read( btree_file, node_pos, &node, 1 );
	/*
	for( int i = 0; i < node.cnt; ++i )
		std::cerr << "node_pos = " << node_pos << " key = " << node.key[i] << std::endl;
	*/
	for( int i = 1; i < node.cnt; ++i ) {
		if( key < node.key[i] ) {
			if( node.leaf ) {
				return query_leaf( node.ch[i-1], key, btree_file, value_file );
			} else {
				return query_node( node.ch[i-1], key, btree_file, value_file );
			}
		}
	}
	if( node.leaf ) {
		return query_leaf( node.ch[node.cnt-1], key, btree_file, value_file );
	} else {
		return query_node( node.ch[node.cnt-1], key, btree_file, value_file );
	}
}

template<typename Key, typename Value, int BLKSZ, int BUFSZ>
bool BTree<Key, Value, BLKSZ, BUFSZ> :: exist_leaf( int leaf_pos, Key key, std::fstream &btree_file ) {
	Leaf leaf;
	fop.read( btree_file, leaf_pos, &leaf, 1 );
	for( int i = 0; i < leaf.cnt; ++i ) {
		if( between( leaf.value[i].first, key, key ) ) {
			return true;
		}
	}
	return false;
}

template<typename Key, typename Value, int BLKSZ, int BUFSZ>
bool BTree<Key, Value, BLKSZ, BUFSZ> :: exist_node( int node_pos, Key key, std::fstream &btree_file ) {
	Node node;
	fop.read( btree_file, node_pos, &node, 1 );
	for( int i = 1; i < node.cnt; ++i ) {
		if( key < node.key[i] ) {
			if( node.leaf ) {
				return exist_leaf( node.ch[i-1], key, btree_file );
			} else {
				return exist_node( node.ch[i-1], key, btree_file );
			}
		}
	}
	if( node.leaf ) {
		return exist_leaf( node.ch[node.cnt-1], key, btree_file );
	} else {
		return exist_node( node.ch[node.cnt-1], key, btree_file );
	}
}

template<typename Key, typename Value, int BLKSZ, int BUFSZ>
bool BTree<Key, Value, BLKSZ, BUFSZ> :: between( Key key, Key left_key, Key right_key ) {
	return !(key < left_key) && !(right_key < key);
}

#endif // BTREE_HPP
