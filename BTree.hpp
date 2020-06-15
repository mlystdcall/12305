#ifndef BTREE_HPP
#define BTREE_HPP

#include <fstream>
#include <utility>
#include "FileOperator.hpp"

template<typename Key, typename Value, int BLKSZ = 4096>
class BTree {
public:
	enum {
		KeyCnt = (BLKSZ - sizeof(bool) - sizeof(int)) / (sizeof(Key) + sizeof(int)),
		ValueCnt = (BLKSZ - sizeof(int) * 3) / sizeof(std::pair<Key,int>),
		KeyLim = KeyCnt / 2 - 1,
		ValueLim = ValueCnt / 2 - 1,
		INF = 1000000000,
	};
	
	void insert( const Key &key,
				 const Value &value,
				 std::fstream &btree_file,
				 std::fstream &value_file );
	void remove( const Key &key,
				 std::fstream &btree_file );
	void modify( const Key &key,
				 const Value &value,
				 std::fstream &btree_file,
				 std::fstream &value_file );
	int query_list( const Key &key_left,
					const Key &key_right,
					std::fstream &btree_file,
					std::fstream &value_file,
					std::pair<Key, Value> *&value ); // inside new outside delete
	Value query( const Key &key,
				 std::fstream &btree_file,
				 std::fstream &value_file );
	bool exist( const Key &key,
				std::fstream &btree_file );
	void write_cache( std::fstream &btree_file,
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
	CachedFileOperator<BLKSZ> cfop_btree;
	CachedFileOperator<sizeof(Value)> cfop_value;
	
	std::tuple<bool,int,int> insert_leaf( int leaf_pos,
										  const Key &key,
										  const Value &value,
										  std::fstream &btree_file,
										  std::fstream &value_file );
	std::tuple<bool,int,int> insert_node( int node_pos,
										  const Key &key,
										  const Value &value,
										  std::fstream &btree_file,
										  std::fstream &value_file );
	Key get_min_leaf( int leaf_pos, std::fstream &btree_file );
	Key get_min_node( int node_pos, std::fstream &btree_file );
	bool remove_leaf( int leaf_pos, const Key &key, std::fstream &btree_file );
	bool remove_node( int node_pos, const Key &key, std::fstream &btree_file );
	int query_leaf_pos( const Key &key, std::fstream &btree_file );
	int query_leaf_pos_helper( int node_pos, const Key &key, std::fstream &btree_file );
	Value query_leaf( int leaf_pos,
					  const Key &key,
					  std::fstream &btree_file,
					  std::fstream &value_file );
	Value query_node( int node_pos,
					  const Key &key,
					  std::fstream &btree_file,
					  std::fstream &value_file );
	bool exist_leaf( int leaf_pos, const Key &key, std::fstream &btree_file );
	bool exist_node( int node_pos, const Key &key, std::fstream &btree_file );
	bool between( const Key &key, const Key &left_key, const Key &right_key );
};

template<typename Key, typename Value, int BLKSZ>
void BTree<Key, Value, BLKSZ> :: insert( const Key &key,
										 const Value &value,
										 std::fstream &btree_file,
										 std::fstream &value_file ) {
	if( fop.end_pos( btree_file ) == 0 ) { // new file
		// std::cerr << "new file" << std::endl;
		Node root;
		cfop_btree.write( btree_file, -1, &root, 1 );
		assert( fop.end_pos( btree_file ) == sizeof(Node) );
		
		Leaf root_leaf;
		cfop_btree.write( btree_file, -1, &root_leaf, 1 );
		assert( fop.end_pos( btree_file ) == sizeof(Node) + sizeof(Leaf) );
	}
	
	Node root;
	cfop_btree.read( btree_file, 0, &root, 1 );
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
			cfop_btree.write( btree_file, 0, &root, 1 );
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
			cfop_btree.write( btree_file, 0, &root, 1 );
		}
	}
}

template<typename Key, typename Value, int BLKSZ>
void BTree<Key, Value, BLKSZ> :: remove( const Key &key,
										 std::fstream &btree_file ) {
	if( fop.end_pos( btree_file ) == 0 ) { // new file
		return;
	} else {
		Node root;
		cfop_btree.read( btree_file, 0, &root, 1 );
		if( root.cnt == 0 ) { // root is leaf
			remove_leaf( sizeof(Node), key, btree_file );
		} else {
			remove_node( 0, key, btree_file );
			cfop_btree.read( btree_file, 0, &root, 1 );
			if( root.cnt == 1 ) {
				if( root.leaf ) {
					Leaf leaf;
					cfop_btree.read( btree_file, root.ch[0], &leaf, 1 );
					cfop_btree.write( btree_file, sizeof(Node), &leaf, 1 );
					assert( leaf.pre == -1 && leaf.nxt == -1 );
					root.cnt = 0;
					cfop_btree.write( btree_file, 0, &root, 1 );
				} else {
					cfop_btree.read( btree_file, root.ch[0], &root, 1 );
					cfop_btree.write( btree_file, 0, &root, 1 );
				}
			} else {
				;
			}
		}
	}
}

template<typename Key, typename Value, int BLKSZ>
void BTree<Key, Value, BLKSZ> :: modify( const Key &key,
										 const Value &value,
										 std::fstream &btree_file,
										 std::fstream &value_file ) {
	insert(key, value, btree_file, value_file);
}

template<typename Key, typename Value, int BLKSZ>
int BTree<Key, Value, BLKSZ> :: query_list( const Key &key_left,
											const Key &key_right,
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
				cfop_btree.read( btree_file, left_leaf, &leaf, 1 );
				int lb = int(lower_bound(leaf.value,
										 leaf.value + leaf.cnt,
										 std::make_pair(key_left, -1)) - leaf.value);
				int rb = int(upper_bound(leaf.value,
										 leaf.value + leaf.cnt,
										 std::make_pair(key_right, (int)INF)) - leaf.value - 1);
				cnt += rb - lb + 1;
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
					cfop_btree.read( btree_file, left_leaf, &leaf, 1 );
					int lb = int(lower_bound(leaf.value,
											 leaf.value + leaf.cnt,
											 std::make_pair(key_left, -1)) - leaf.value);
					int rb = int(upper_bound(leaf.value,
											 leaf.value + leaf.cnt,
											 std::make_pair(key_right, (int)INF)) - leaf.value - 1);
					for( int i = lb; i <= rb; ++i ) {
						value[now].first = leaf.value[i].first;
						cfop_value.read( value_file, leaf.value[i].second, &value[now].second, 1 );
						++now;
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

template<typename Key, typename Value, int BLKSZ>
Value BTree<Key, Value, BLKSZ> :: query( const Key &key,
										 std::fstream &btree_file,
										 std::fstream &value_file ) {
	if( fop.end_pos( btree_file ) == 0 ) { // new file
		assert(0); // not existed
		return Value();
	} else {
		Node root;
		cfop_btree.read( btree_file, 0, &root, 1 );
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
		return ans;
	}
}

template<typename Key, typename Value, int BLKSZ>
bool BTree<Key, Value, BLKSZ> :: exist( const Key &key,
										std::fstream &btree_file ) {
	if( fop.end_pos( btree_file ) == 0 ) { // new file
		return false;
	} else {
		Node root;
		cfop_btree.read( btree_file, 0, &root, 1 );
		bool ans;
		if( root.cnt == 0 ) { // root is leaf
			ans = exist_leaf( sizeof(Node), key, btree_file );
		} else { // root is node
			ans = exist_node( 0, key, btree_file );
		}		
		return ans;
	}
}

template<typename Key, typename Value, int BLKSZ>
std::tuple<bool,int,int> BTree<Key, Value, BLKSZ> :: insert_leaf( int leaf_pos,
																  const Key &key,
																  const Value &value,
																  std::fstream &btree_file,
																  std::fstream &value_file ) {
	Leaf leaf;
	cfop_btree.read( btree_file, leaf_pos, &leaf, 1 );
	assert( leaf.cnt < ValueCnt );
	int pos = int(lower_bound( leaf.value,
							   leaf.value + leaf.cnt,
							   std::make_pair(key, -1) ) - leaf.value);
	if( pos < leaf.cnt && between( leaf.value[pos].first, key, key ) ) {
		int i = pos;
		cfop_value.write( value_file, leaf.value[i].second, &value, 1 );
		return std::make_tuple(true, -1, -1);
	}
	bool biggest = true;
	pos = int(upper_bound( leaf.value,
						   leaf.value + leaf.cnt,
						   std::make_pair(key, (int)INF) ) - leaf.value);
	if( pos < leaf.cnt ) {
		int i = pos;
		for( int j = leaf.cnt-1; j >= i; --j )
			leaf.value[j+1] = leaf.value[j];
		leaf.value[i].first = key;
		leaf.value[i].second = cfop_value.write( value_file, -1, &value, 1 );
		++leaf.cnt;
		biggest = false;
	}
	if( biggest == true ) {
		leaf.value[leaf.cnt].first = key;
		leaf.value[leaf.cnt].second = cfop_value.write( value_file, -1, &value, 1 );
		++leaf.cnt;
	}
	cfop_btree.write( btree_file, leaf_pos, &leaf, 1 );
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
		int left_pos = cfop_btree.write( btree_file, -1, &left_part, 1 );
		int right_pos = cfop_btree.write( btree_file, -1, &right_part, 1 );
		left_part.pre = leaf.pre;
		left_part.nxt = right_pos;
		right_part.pre = left_pos;
		right_part.nxt = leaf.nxt;
		cfop_btree.write( btree_file, left_pos, &left_part, 1 );
		cfop_btree.write( btree_file, right_pos, &right_part, 1 );
		if( left_part.pre != -1 ) {
			Leaf tmp;
			cfop_btree.read( btree_file, left_part.pre, &tmp, 1 );
			tmp.nxt = left_pos;
			cfop_btree.write( btree_file, left_part.pre, &tmp, 1 );
		}
		if( right_part.nxt != -1 ) {
			Leaf tmp;
			cfop_btree.read( btree_file, right_part.nxt, &tmp, 1 );
			tmp.pre = right_pos;
			cfop_btree.write( btree_file, right_part.nxt, &tmp, 1 );
		}
		return std::make_tuple(false, left_pos, right_pos);
	} else {
		return std::make_tuple(true, -1, -1);
	}
}

template<typename Key, typename Value, int BLKSZ>
std::tuple<bool,int,int> BTree<Key, Value, BLKSZ> :: insert_node( int node_pos,
																  const Key &key,
																  const Value &value,
																  std::fstream &btree_file,
																  std::fstream &value_file ) {
	Node node;
	cfop_btree.read( btree_file, node_pos, &node, 1 );
	assert( node.cnt < KeyCnt );
	bool biggest = true;
	int pos = int(upper_bound( node.key + 1,
							   node.key + node.cnt,
							   key ) - node.key );
	if( pos < node.cnt ) {
		int i = pos;
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
	cfop_btree.write( btree_file, node_pos, &node, 1 );
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
		int left_pos = cfop_btree.write( btree_file, -1, &left_part, 1 );
		int right_pos = cfop_btree.write( btree_file, -1, &right_part, 1 );
		return std::make_tuple(false, left_pos, right_pos);
	} else {
		return std::make_tuple(true, -1, -1);
	}
}

template<typename Key, typename Value, int BLKSZ>
Key BTree<Key, Value, BLKSZ> :: get_min_leaf( int leaf_pos, std::fstream &btree_file ) {
	Leaf leaf;
	cfop_btree.read( btree_file, leaf_pos, &leaf, 1 );
	return leaf.value[0].first;
}

template<typename Key, typename Value, int BLKSZ>
Key BTree<Key, Value, BLKSZ> :: get_min_node( int node_pos, std::fstream &btree_file ) {
	Node node;
	cfop_btree.read( btree_file, node_pos, &node, 1 );
	return node.key[0];
}

template<typename Key, typename Value, int BLKSZ>
bool BTree<Key, Value, BLKSZ> :: remove_leaf( int leaf_pos, const Key &key, std::fstream &btree_file ) {
	Leaf leaf;
	cfop_btree.read( btree_file, leaf_pos, &leaf, 1 );
	int pos = int(lower_bound( leaf.value,
							   leaf.value + leaf.cnt,
							   std::make_pair(key, -1) ) - leaf.value);
	if( pos < leaf.cnt && between( leaf.value[pos].first, key, key ) ) {
		int i = pos;
		for( int j = i+1; j < leaf.cnt; ++j )
			leaf.value[j-1] = leaf.value[j];
		--leaf.cnt;
	}
	cfop_btree.write( btree_file, leaf_pos, &leaf, 1 );
	return leaf.cnt <= ValueLim;
}

template<typename Key, typename Value, int BLKSZ>
bool BTree<Key, Value, BLKSZ> :: remove_node( int node_pos, const Key &key, std::fstream &btree_file ) {
	Node node;
	cfop_btree.read( btree_file, node_pos, &node, 1 );
	assert( node.cnt > 1 );
	bool biggest = true;
	int pos = int(upper_bound( node.key + 1,
							   node.key + node.cnt,
							   key ) - node.key);
	if( pos < node.cnt ) {
		int i = pos;
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
				cfop_btree.read( btree_file, node.ch[i-1], &now_leaf, 1 );
				cfop_btree.read( btree_file, node.ch[i], &right_leaf, 1 );
				if( right_leaf.cnt == ValueLim + 1 ) { // merge
					assert( now_leaf.cnt == ValueLim );
					for( int j = 0; j < right_leaf.cnt; ++j )
						now_leaf.value[now_leaf.cnt + j] = right_leaf.value[j];
					now_leaf.cnt += right_leaf.cnt;
					assert( now_leaf.cnt == ValueLim + ValueLim + 1 );
					now_leaf.nxt = right_leaf.nxt;
					if( now_leaf.nxt != -1 ) {
						Leaf tmp;
						cfop_btree.read( btree_file, now_leaf.nxt, &tmp, 1 );
						tmp.pre = node.ch[i-1];
						cfop_btree.write( btree_file, now_leaf.nxt, &tmp, 1 );
					}
					cfop_btree.write( btree_file, node.ch[i-1], &now_leaf, 1 );
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
					cfop_btree.write( btree_file, node.ch[i-1], &now_leaf, 1 );
					cfop_btree.write( btree_file, node.ch[i], &right_leaf, 1 );
					node.key[i-1] = get_min_leaf( node.ch[i-1], btree_file );
					node.key[i] = get_min_leaf( node.ch[i], btree_file );
				} else {
					assert(0); // right_leaf.cnt <= ValueLim
				}
			} else {
				Node now_node;
				Node right_node;
				cfop_btree.read( btree_file, node.ch[i-1], &now_node, 1 );
				cfop_btree.read( btree_file, node.ch[i], &right_node, 1 );
				if( right_node.cnt == KeyLim + 1 ) { // merge
					assert( now_node.cnt == KeyLim );
					for( int j = 0; j < right_node.cnt; ++j ) {
						now_node.key[now_node.cnt + j] = right_node.key[j];
						now_node.ch[now_node.cnt + j] = right_node.ch[j];
					}
					now_node.cnt += right_node.cnt;
					assert( now_node.cnt == KeyLim + KeyLim + 1 );
					cfop_btree.write( btree_file, node.ch[i-1], &now_node, 1 );
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
					cfop_btree.write( btree_file, node.ch[i-1], &now_node, 1 );
					cfop_btree.write( btree_file, node.ch[i], &right_node, 1 );
					node.key[i-1] = get_min_node( node.ch[i-1], btree_file );
					node.key[i] = get_min_node( node.ch[i], btree_file );
				} else {
					assert(0); // right_node.cnt <= KeyLim
				}
			}
		} else {
			;
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
				cfop_btree.read( btree_file, node.ch[node.cnt-1], &now_leaf, 1 );
				cfop_btree.read( btree_file, node.ch[node.cnt-2], &left_leaf, 1 );
				if( left_leaf.cnt == ValueLim + 1 ) { // merge
					assert( now_leaf.cnt == ValueLim );
					for( int j = 0; j < now_leaf.cnt; ++j )
						left_leaf.value[left_leaf.cnt + j] = now_leaf.value[j];
					left_leaf.cnt += now_leaf.cnt;
					assert( left_leaf.cnt == ValueLim + ValueLim + 1 );
					left_leaf.nxt = now_leaf.nxt;
					if( left_leaf.nxt != -1 ) {
						Leaf tmp;
						cfop_btree.read( btree_file, left_leaf.nxt, &tmp, 1 );
						tmp.pre = node.ch[node.cnt-2];
						cfop_btree.write( btree_file, left_leaf.nxt, &tmp, 1 );
					}
					cfop_btree.write( btree_file, node.ch[node.cnt-2], &left_leaf, 1 );
					--node.cnt;
				} else if( left_leaf.cnt > ValueLim + 1 ) { // borrow
					for( int j = now_leaf.cnt-1; j >= 0; --j )
						now_leaf.value[j+1] = now_leaf.value[j];
					now_leaf.value[0] = left_leaf.value[left_leaf.cnt-1];
					--left_leaf.cnt;
					++now_leaf.cnt;
					cfop_btree.write( btree_file, node.ch[node.cnt-2], &left_leaf, 1 );
					cfop_btree.write( btree_file, node.ch[node.cnt-1], &now_leaf, 1 );
					node.key[node.cnt-2] = get_min_leaf( node.ch[node.cnt-2], btree_file );
					node.key[node.cnt-1] = get_min_leaf( node.ch[node.cnt-1], btree_file );
				} else {
					assert(0); // left_leaf.cnt <= ValueLim
				}
			} else {
				Node now_node;
				Node left_node;
				cfop_btree.read( btree_file, node.ch[node.cnt-1], &now_node, 1 );
				cfop_btree.read( btree_file, node.ch[node.cnt-2], &left_node, 1 );
				if( left_node.cnt == KeyLim + 1 ) { // merge
					assert( now_node.cnt == KeyLim );
					for( int j = 0; j < now_node.cnt; ++j ) {
						left_node.key[left_node.cnt + j] = now_node.key[j];
						left_node.ch[left_node.cnt + j] = now_node.ch[j];
					}
					left_node.cnt += now_node.cnt;
					assert( left_node.cnt == KeyLim + KeyLim + 1 );
					cfop_btree.write( btree_file, node.ch[node.cnt-2], &left_node, 1 );
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
					cfop_btree.write( btree_file, node.ch[node.cnt-2], &left_node, 1 );
					cfop_btree.write( btree_file, node.ch[node.cnt-1], &now_node, 1 );
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
	cfop_btree.write( btree_file, node_pos, &node, 1 );
	return node.cnt <= KeyLim;
}

template<typename Key, typename Value, int BLKSZ>
int BTree<Key, Value, BLKSZ> :: query_leaf_pos( const Key &key, std::fstream &btree_file ) {
	assert( fop.end_pos( btree_file ) != 0 );
	Node root;
	cfop_btree.read( btree_file, 0, &root, 1 );
	if( root.cnt == 0 ) { // root is leaf
		return sizeof(Node);
	} else { // root is node
		return query_leaf_pos_helper( 0, key, btree_file );
	}
}

template<typename Key, typename Value, int BLKSZ>
int BTree<Key, Value, BLKSZ> :: query_leaf_pos_helper( int node_pos, const Key &key, std::fstream &btree_file ) {
	Node node;
	cfop_btree.read( btree_file, node_pos, &node, 1 );
	int pos = int(upper_bound( node.key + 1,
							   node.key + node.cnt,
							   key ) - node.key);
	if( pos < node.cnt ) {
		int i = pos;
		if( node.leaf ) {
			return node.ch[i-1];
		} else {
			return query_leaf_pos_helper( node.ch[i-1], key, btree_file );
		}
	}
	if( node.leaf ) {
		return node.ch[node.cnt-1];
	} else {
		return query_leaf_pos_helper( node.ch[node.cnt-1], key, btree_file );
	}
}

template<typename Key, typename Value, int BLKSZ>
Value BTree<Key, Value, BLKSZ> :: query_leaf( int leaf_pos,
											  const Key &key,
											  std::fstream &btree_file,
											  std::fstream &value_file ) {
	Leaf leaf;
	cfop_btree.read( btree_file, leaf_pos, &leaf, 1 );
	int pos = int(lower_bound( leaf.value,
							   leaf.value + leaf.cnt,
							   std::make_pair(key, -1) ) - leaf.value);
	if( pos < leaf.cnt && between( leaf.value[pos].first, key, key ) ) {
		int i = pos;
		Value ans;
		cfop_value.read( value_file, leaf.value[i].second, &ans, 1 );
		return ans;
	}
	return assert(0), Value();
}

template<typename Key, typename Value, int BLKSZ>
Value BTree<Key, Value, BLKSZ> :: query_node( int node_pos,
											  const Key &key,
											  std::fstream &btree_file,
											  std::fstream &value_file ) {
	Node node;
	cfop_btree.read( btree_file, node_pos, &node, 1 );
	/*
	for( int i = 0; i < node.cnt; ++i )
		std::cerr << "node_pos = " << node_pos << " key = " << node.key[i] << std::endl;
	*/
	int pos = int(upper_bound( node.key + 1,
							   node.key + node.cnt,
							   key ) - node.key);
	if( pos < node.cnt ) {
		int i = pos;
		if( node.leaf ) {
			return query_leaf( node.ch[i-1], key, btree_file, value_file );
		} else {
			return query_node( node.ch[i-1], key, btree_file, value_file );
		}
	}
	if( node.leaf ) {
		return query_leaf( node.ch[node.cnt-1], key, btree_file, value_file );
	} else {
		return query_node( node.ch[node.cnt-1], key, btree_file, value_file );
	}
}

template<typename Key, typename Value, int BLKSZ>
bool BTree<Key, Value, BLKSZ> :: exist_leaf( int leaf_pos, const Key &key, std::fstream &btree_file ) {
	Leaf leaf;
	cfop_btree.read( btree_file, leaf_pos, &leaf, 1 );
	int pos = int(lower_bound( leaf.value,
							   leaf.value + leaf.cnt,
							   std::make_pair(key, -1) ) - leaf.value);
	if( pos < leaf.cnt && between( leaf.value[pos].first, key, key ) ) {
		return true;
	}
	return false;
}

template<typename Key, typename Value, int BLKSZ>
bool BTree<Key, Value, BLKSZ> :: exist_node( int node_pos, const Key &key, std::fstream &btree_file ) {
	Node node;
	cfop_btree.read( btree_file, node_pos, &node, 1 );
	int pos = int(upper_bound( node.key + 1,
							   node.key + node.cnt,
							   key ) - node.key);
	if( pos < node.cnt ) {
		int i = pos;
		if( node.leaf ) {
			return exist_leaf( node.ch[i-1], key, btree_file );
		} else {
			return exist_node( node.ch[i-1], key, btree_file );
		}
	}
	if( node.leaf ) {
		return exist_leaf( node.ch[node.cnt-1], key, btree_file );
	} else {
		return exist_node( node.ch[node.cnt-1], key, btree_file );
	}
}

template<typename Key, typename Value, int BLKSZ>
bool BTree<Key, Value, BLKSZ> :: between( const Key &key, const Key &left_key, const Key &right_key ) {
	return !(key < left_key) && !(right_key < key);
}

template<typename Key, typename Value, int BLKSZ>
void BTree<Key, Value, BLKSZ> :: write_cache( std::fstream &btree_file,
											  std::fstream &value_file ) {
	cfop_btree.write_cache(btree_file);
	cfop_value.write_cache(value_file);
}

#endif // BTREE_HPP
