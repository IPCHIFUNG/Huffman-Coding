#ifndef COMPRESSER_H
#define COMPRESSER_H

#include <string>

using namespace std;

class FileProperties
{
public:
	string fileName;
};

class Node
{
public:
	long weight;
	unsigned char ch;
	int parent;
	int lc, rc;
	char byte[256];
	int codeLength;
};

class Compresser
{
public:
	static double compress(string inPath, string outPath);
	static bool decompress(string path);


private:
	static int findMin(Node *tree, int n);
	static void getByte(Node *node, int n);
};

#endif // COMPRESSER_H
