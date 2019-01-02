#include "md5.h"

using namespace std;

/* Constants for MD5Transform routine. */
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21


/* F, G, H and I are basic MD5 functions.
*/
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))
/* 左移*/
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

#define FF(a, b, c, d, x, s, ac) { \
	(a) += F ((b), (c), (d)) + (x) + ac; \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
}
#define GG(a, b, c, d, x, s, ac) { \
	(a) += G ((b), (c), (d)) + (x) + ac; \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
}
#define HH(a, b, c, d, x, s, ac) { \
	(a) += H ((b), (c), (d)) + (x) + ac; \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
}
#define II(a, b, c, d, x, s, ac) { \
	(a) += I ((b), (c), (d)) + (x) + ac; \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
}


const byte MD5::PADDING[64] = { 0x80 };
const char MD5::HEX[16] = {
	'0', '1', '2', '3',
	'4', '5', '6', '7',
	'8', '9', 'a', 'b',
	'c', 'd', 'e', 'f'
};

/* 错误构造 */
MD5::MD5() {
	reset();
}

/* 用输入缓冲区构造MD5对象. */
MD5::MD5(const void *input, size_t length) {
	reset();
	update(input, length);
}

/* 用输入流构造MD5对象. */
MD5::MD5(const string &str) {
	reset();
	update(str);
}

/* 用文件构造MD5对象。 */
MD5::MD5(ifstream &in) {
	reset();
	update(in);
}

/*返回消息摘要 */
const byte* MD5::digest() {
	if (!_finished) {
		_finished = true;
		final();
	}
	return _digest;
}

/* 重置计算状态 */
void MD5::reset() {

	_finished = false;
	/* 复位位数. */
	_count[0] = _count[1] = 0;
	/* 加载神奇初始化常量. */
	_state[0] = 0x67452301;
	_state[1] = 0xefcdab89;
	_state[2] = 0x98badcfe;
	_state[3] = 0x10325476;
}

/* Updating the context with a input buffer. */
void MD5::update(const void *input, size_t length) {
	update((const byte*)input, length);
}

/* Updating the context with a string. */
void MD5::update(const string &str) {
	update((const byte*)str.c_str(), str.length());
}

/* Updating the context with a file. */
void MD5::update(ifstream &in) {

	if (!in)
		return;

	std::streamsize length;
	char buffer[BUFFER_SIZE];
	while (!in.eof()) {
		in.read(buffer, BUFFER_SIZE);
		length = in.gcount();
		if (length > 0)
			update(buffer, length);
	}
	in.close();
}

/*MD5块更新操作。继续MD5消息摘要
操作，处理另一个消息块，并更新
语境。
*/
void MD5::update(const byte *input, size_t length) {

	ulong i, index, partLen;

	_finished = false;

	/* 计算字节数余 64 */
	index = (ulong)((_count[0] >> 3) & 0x3f);

	/* 更新位数 */
	if ((_count[0] += ((ulong)length << 3)) < ((ulong)length << 3))
		_count[1]++;
	_count[1] += ((ulong)length >> 29);

	partLen = 64 - index;

	/* 多次变化 */
	if (length >= partLen) {

		memcpy(&_buffer[index], input, partLen);
		transform(_buffer);

		for (i = partLen; i + 63 < length; i += 64)
			transform(&input[i]);
		index = 0;

	}
	else {
		i = 0;
	}

	/* Buffer remaining input */
	memcpy(&_buffer[index], &input[i], length - i);
}

/* 结束MD5消息摘要操作，写入
消息消化并归零上下文。
*/
void MD5::final() {

	byte bits[8];
	ulong oldState[4];
	ulong oldCount[2];
	ulong index, padLen;

	/*保存当前状态并计数。 */
	memcpy(oldState, _state, 16);
	memcpy(oldCount, _count, 8);

	/* 保存位数 */
	encode(_count, bits, 8);

	/* Pad out to 56 mod 64. */
	index = (ulong)((_count[0] >> 3) & 0x3f);
	padLen = (index < 56) ? (56 - index) : (120 - index);
	update(PADDING, padLen);

	/*附加长度（填充前）*/
	update(bits, 8);

	/* Store state in digest */
	encode(_state, _digest, 16);

	/* 还原当前状态并计数. */
	memcpy(_state, oldState, 16);
	memcpy(_count, oldCount, 8);
}

/* MD5基本转换。基于块的转换状态. */
void MD5::transform(const byte block[64]) {

	ulong a = _state[0], b = _state[1], c = _state[2], d = _state[3], x[16];

	decode(block, x, 64);

	/* Round 1 */
	FF(a, b, c, d, x[0], S11, 0xd76aa478); /* 1 */
	FF(d, a, b, c, x[1], S12, 0xe8c7b756); /* 2 */
	FF(c, d, a, b, x[2], S13, 0x242070db); /* 3 */
	FF(b, c, d, a, x[3], S14, 0xc1bdceee); /* 4 */
	FF(a, b, c, d, x[4], S11, 0xf57c0faf); /* 5 */
	FF(d, a, b, c, x[5], S12, 0x4787c62a); /* 6 */
	FF(c, d, a, b, x[6], S13, 0xa8304613); /* 7 */
	FF(b, c, d, a, x[7], S14, 0xfd469501); /* 8 */
	FF(a, b, c, d, x[8], S11, 0x698098d8); /* 9 */
	FF(d, a, b, c, x[9], S12, 0x8b44f7af); /* 10 */
	FF(c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
	FF(b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
	FF(a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
	FF(d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
	FF(c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
	FF(b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

	/* Round 2 */
	GG(a, b, c, d, x[1], S21, 0xf61e2562); /* 17 */
	GG(d, a, b, c, x[6], S22, 0xc040b340); /* 18 */
	GG(c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
	GG(b, c, d, a, x[0], S24, 0xe9b6c7aa); /* 20 */
	GG(a, b, c, d, x[5], S21, 0xd62f105d); /* 21 */
	GG(d, a, b, c, x[10], S22, 0x2441453); /* 22 */
	GG(c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
	GG(b, c, d, a, x[4], S24, 0xe7d3fbc8); /* 24 */
	GG(a, b, c, d, x[9], S21, 0x21e1cde6); /* 25 */
	GG(d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
	GG(c, d, a, b, x[3], S23, 0xf4d50d87); /* 27 */
	GG(b, c, d, a, x[8], S24, 0x455a14ed); /* 28 */
	GG(a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
	GG(d, a, b, c, x[2], S22, 0xfcefa3f8); /* 30 */
	GG(c, d, a, b, x[7], S23, 0x676f02d9); /* 31 */
	GG(b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

	/* Round 3 */
	HH(a, b, c, d, x[5], S31, 0xfffa3942); /* 33 */
	HH(d, a, b, c, x[8], S32, 0x8771f681); /* 34 */
	HH(c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
	HH(b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
	HH(a, b, c, d, x[1], S31, 0xa4beea44); /* 37 */
	HH(d, a, b, c, x[4], S32, 0x4bdecfa9); /* 38 */
	HH(c, d, a, b, x[7], S33, 0xf6bb4b60); /* 39 */
	HH(b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
	HH(a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
	HH(d, a, b, c, x[0], S32, 0xeaa127fa); /* 42 */
	HH(c, d, a, b, x[3], S33, 0xd4ef3085); /* 43 */
	HH(b, c, d, a, x[6], S34, 0x4881d05); /* 44 */
	HH(a, b, c, d, x[9], S31, 0xd9d4d039); /* 45 */
	HH(d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
	HH(c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
	HH(b, c, d, a, x[2], S34, 0xc4ac5665); /* 48 */

	/* Round 4 */
	II(a, b, c, d, x[0], S41, 0xf4292244); /* 49 */
	II(d, a, b, c, x[7], S42, 0x432aff97); /* 50 */
	II(c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
	II(b, c, d, a, x[5], S44, 0xfc93a039); /* 52 */
	II(a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
	II(d, a, b, c, x[3], S42, 0x8f0ccc92); /* 54 */
	II(c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
	II(b, c, d, a, x[1], S44, 0x85845dd1); /* 56 */
	II(a, b, c, d, x[8], S41, 0x6fa87e4f); /* 57 */
	II(d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
	II(c, d, a, b, x[6], S43, 0xa3014314); /* 59 */
	II(b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
	II(a, b, c, d, x[4], S41, 0xf7537e82); /* 61 */
	II(d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
	II(c, d, a, b, x[2], S43, 0x2ad7d2bb); /* 63 */
	II(b, c, d, a, x[9], S44, 0xeb86d391); /* 64 */

	_state[0] += a;
	_state[1] += b;
	_state[2] += c;
	_state[3] += d;
}

/* 将输入（ulong）编码为输出（byte）。假设长度为
4的倍数。
*/
void MD5::encode(const ulong *input, byte *output, size_t length) {

	for (size_t i = 0, j = 0; j < length; i++, j += 4) {
		output[j] = (byte)(input[i] & 0xff);
		output[j + 1] = (byte)((input[i] >> 8) & 0xff);
		output[j + 2] = (byte)((input[i] >> 16) & 0xff);
		output[j + 3] = (byte)((input[i] >> 24) & 0xff);
	}
}

/* 将输入（byte）解码为输出（ulong）。假设长度为
4的倍数。
*/
void MD5::decode(const byte *input, ulong *output, size_t length) {

	for (size_t i = 0, j = 0; j < length; i++, j += 4) {
		output[i] = ((ulong)input[j]) | (((ulong)input[j + 1]) << 8) |
			(((ulong)input[j + 2]) << 16) | (((ulong)input[j + 3]) << 24);
	}
}

/* 将字节数组转换为十六进制字符串. */
string MD5::bytesToHexString(const byte *input, size_t length) {
	string str;
	str.reserve(length << 1);
	for (size_t i = 0; i < length; i++) {
		int t = input[i];
		int a = t / 16;
		int b = t % 16;
		str.append(1, HEX[a]);
		str.append(1, HEX[b]);
	}
	return str;
}

/* 将摘要转换为字符串值 */
string MD5::toString() {
	return bytesToHexString(digest(), 16);
}
