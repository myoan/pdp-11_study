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

void parse_header(char* code, size_t* code_size, size_t* data_size)
{
	size_t i = 0;
	unsigned int unknown = get_two_byte_hex(code[i], code[i+1]);
	i += 2;
	*code_size = get_two_byte_hex(code[i], code[i+1]);
	i += 2;
	*data_size = get_two_byte_hex(code[i], code[i+1]);
	//cout << "code(" << *code_size << "), data(" << *data_size << ")" << endl;
}

char* create_buf(size_t size)
{
	char* ret = 0;
	ret = new char[ size + 1 ]; //  for the '\0'
	memset(ret, '\0', size+1);
	return ret;
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

	file.seekg(0, ios::end); // set the pointer to the end
	size = file.tellg() ; // get the length of the file
	file.seekg(0, ios::beg); // set the pointer to the beginning

	size_t code_size = 0;
	size_t data_size = 0;

	size_t line = 0;
	char* header = create_buf(HEADER_SIZE);
	file.read(header, HEADER_SIZE);
	parse_header(header, &code_size, &data_size);
	char* code = create_buf(size - HEADER_SIZE);
	file.read(code, size - HEADER_SIZE);
	size_t cnt = 0;
	char* linebuf = create_buf(16);
	char* r = create_buf(16);
	char* data = create_buf(256);
	while (cnt < code_size) {
		size_t line_size = (code_size - cnt) / 16 == 0 ? (code_size - cnt) % 16: 16;
		memset(linebuf, '\0', line_size+1);
		memcpy(linebuf, code + cnt, line_size);
		size_t i = 0;
		size_t r_idx, value, addr, length;
		for ( i = 0; i < line_size; i += 2 ) {
			unsigned int opecode = get_two_byte_hex(linebuf[i], linebuf[i+1]);
			size_t key, value, sys_idx;
			switch (opecode) {
			case 0x15c0: // mov value reg0
				i += 2;
				value = linebuf[i];
				r_idx = linebuf[i+1];
				r[r_idx] = value;
				//cout << "r[" << r_idx << "]: " << uint(r[r_idx]) << endl;
				break;
			case 0x8901: // system call: exit reg0
				//cout << endl;
				exit(r[0]);
			case 0x8904: // system call: write reg0 addr len
				i += 2;
				addr = get_two_byte_hex(linebuf[i], linebuf[i+1]);
				i += 2;
				length = get_two_byte_hex(linebuf[i], linebuf[i+1]);
				memcpy(data, code + addr, length);
				switch (r[0]) {
				case 1: // stdout
					cout << data;
					break;
				case 2: // stderr
					cerr << data;
					break;
				default:
					cerr << "Unknown file descriptor" << endl;
					break;
				}
				break;
			default:
				cout << "UNKNOWN OPCODE: " << opecode << endl;
			}
		}
		line++;
		cnt += line_size;
	}
	file.close();
	return 0;
}
