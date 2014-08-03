#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cstddef>
#include <string.h>

using namespace std;

string hex_byte(string hex, size_t len)
{
	size_t shift = len - hex.size();
	string head = "";
	for (size_t i = 0; i < shift; i++) {
		head += "0";
	}
	return head + hex;
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

unsigned int get_two_byte_hex(unsigned int tail, unsigned int head)
{
	unsigned int head_p = (head + 256) % 256;
	unsigned int tail_p = (tail + 256) % 256;
	return (head_p << 8) + tail_p;
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

void parse_header(ifstream* file, size_t code_size, size_t data_size)
{
	char* oData = 0;
	oData = new char[ 16+1 ]; //  for the '\0'
	file->read(oData, 16);
	size_t i = 0;
	unsigned int unknown = get_two_byte_hex(oData[i], oData[i+1]);
	i += 2;
	code_size = get_two_byte_hex(oData[i], oData[i+1]);
	i += 2;
	data_size = get_two_byte_hex(oData[i], oData[i+1]);
	//cout << "code(" << code_size << "), data(" << data_size << ")" << endl;
}

int main(int argc, char* argv[])
{
	size_t HEADER_SIZE = 16;
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

	size_t code_size = 0;
	size_t data_size = 0;

	size_t line = 0;
	oData = new char[ 16+1 ]; //  for the '\0'
	parse_header(&file, code_size, data_size);
	size_t cnt = 0;
	while (cnt < HEADER_SIZE + code_size) {
		size_t width = (size - cnt) / 16 == 0 ? (size-cnt) % 16: 16;
		file.read(oData, width);
		oData[width] = '\0' ; // set '\0' 

		size_t i = 0;
		for ( i = 0; i < width; i += 2 ) {
			cout << hex_byte(dec_to_hex(line * 16 + i), 4) << ": ";
			unsigned int opecode = get_two_byte_hex(oData[i], oData[i+1]);
			cout << hex_byte(dec_to_hex(opecode), 4) << " ";
			unsigned int data;
			size_t key, value, sys_idx;
			switch (opecode) {
			case 0x15c0:
				i += 2;
				key = oData[i];
				value = oData[i+1];
				data = get_two_byte_hex(value, key);
				cout << hex_byte(dec_to_hex(data), 4);
				cout << '\t' << '\t';
				cout << "mov " << "$" << key << ", " << "r" << value;
				break;
			case 0x8901: case 0x8904:
				sys_idx = oData[i];
				cout << '\t' << '\t';
				cout << "sys " << sys_idx << " ; ";
				switch (sys_idx) {
				case 1:
					cout << "exit";
					break;
				case 4:
					cout << "write";
					break;
				default:
					break;
				}
				break;
			default:
				cout << '\t' << '\t';
				cout << "; arg";
				break;
			}
			cout << endl;
		}
		line++;
		cnt = file.tellg();
	}
	file.close();
	return 0;
}
