#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cstddef>
#include <string.h>

using namespace std;

string hex_byte(string hex)
{
	if (hex.size() == 1) {
		return "0" + hex;
	}
	return hex;
}

string dec_to_hex(unsigned int dec)
{
	const char hex_map[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
	int quot = dec / 16;
	int remd = dec % 16;
	string ret = "";
	ret = string(1, hex_map[remd]) + ret;
	if (quot != 0) {
		ret = dec_to_hex(quot) + ret;
	}
	return ret;
}

string get_rawdata(char* buf, size_t len)
{
	string ret = ">";
	//cout << len;
	for (size_t i = 0; i < len; i++) {
		if (buf[i] > 'a') {
			ret += buf[i];
		}
		else {
			ret += ".";
		}
	}
	ret += "<";
	return ret;
}

int main(int argc, char* argv[])
{
	if ( argc < 2 ) {
		cerr << "Illegal Option" << endl;
		cerr << "Usage: " << argv[0] << " <target_file>" << endl;
		return EXIT_FAILURE;
	}
	ifstream file;
	file.open( argv[1], ios::in|ios::binary|ios::ate );
	if (!file.is_open()) {
		cerr << "Fail to read" << endl;
		return EXIT_FAILURE;
	}
	file.seekg(0, ios::beg);
	size_t size = 0; // here
	char* oData = 0;

	file.seekg(0, ios::end); // set the pointer to the end
	size = file.tellg() ; // get the length of the file
	file.seekg(0, ios::beg); // set the pointer to the beginning

	size_t line = 0;
	size_t cnt = file.tellg();
	oData = new char[ 16+1 ]; //  for the '\0'
	while (cnt < size) {
		size_t width = (size - cnt) / 16 == 0 ? (size-cnt) % 16: 16;
		file.read(oData, width);
		oData[width] = '\0' ; // set '\0' 

		size_t i = 0;
		cout << "00" << hex_byte(dec_to_hex(line * 16 + width)) << " ";
		for ( i = 0; i < width+1; i++ ) {
			cout << ( hex_byte(dec_to_hex((oData[i] + 256) % 256)) );
			cout << " ";
		}
		cout << get_rawdata(oData, width) << endl;
		line++;
		cnt = file.tellg();
	}
	file.close();
	return 0;
}
