#include "md5.h"
#include <iostream>
#include <windows.h>
using namespace std;

void PrintMD5(const string &str, MD5 &md5) {
	cout << "MD5(\"" << str << "\") = " << md5.toString() << endl;
}

string FileDigest(const string &file) {

	ifstream in(file.c_str(), ios::binary);
	if (!in)
		return "";

	MD5 md5;
	std::streamsize length;
	char buffer[1024];
	while (!in.eof()) {
		in.read(buffer, 1024);
		length = in.gcount();
		if (length > 0)
			md5.update(buffer, length);
	}
	in.close();
	return md5.toString();
}

int main() {
	while (1) {
		cout << "加密文本输入1——————加密文件输入2" << endl;
		int cint;
		cin >> cint;
		switch (cint)
		{
		case 1:char DATA[1024]; cout << "输入文本：";
			cin >> DATA;
			cout << MD5(DATA).toString() << endl;
			Sleep(500); break;
		case 2: {string DATA2;
			cout << "输入文件地址：";
			cin >> DATA2;
			cout << FileDigest(DATA2) << endl;
			Sleep(500); } break;
		default:
			break;
		}
	}

	return 0;
}

