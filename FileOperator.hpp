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
		file.read( reinterpret_cast<char*>(p), cnt * sizeof(T) );
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
		file.write( reinterpret_cast<char*>(p), cnt * sizeof(T) );
		return pos;
	}
	
	int end_pos( std::fstream &file ) const {
		file.clear();
		file.seekg(0, std::ios::end);
		return (int)file.tellg();
	}
};

#endif // FILE_OPERATOR_HPP
