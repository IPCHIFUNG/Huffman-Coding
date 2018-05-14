#include "compresser.h"
#include <iostream>
#include <stdio.h>

using namespace std;

double Compresser::compress(string inPath, string outPath)
{
	Node hufTree[512];
	int i, j;
	for (i = 0; i < 512; i++)
		hufTree[i].weight = 0;

	// 取出文件后缀名
	char profix[10];
	for (i = inPath.size(); i >= 0 && inPath[i] != '.'; i--);
	for (j = 0; inPath[i+j] != '\0'; j++)
		profix[j] = inPath[i + j + 1];
	
	FILE  *input, *output;
	unsigned char buffer;
	char codes[256];
	long sumlength = 0;
	int count = 0;

	input = fopen(&inPath[0], "rb");
	output = fopen(&outPath[0], "wb");
	if (input == NULL || output == NULL)
	{
		cout << "Fail in opening file." << endl;
		return -1;
	}

	//用字符的ASCII码值结点下标
	long FileLength = 0;
	while (!feof(input)) // 判断是否到了文件尾部
	{
		fread(&buffer, 1, 1, input); // 读取数据
		hufTree[buffer].weight++; // 计算权值
		FileLength++;
	}

	FileLength--;
	hufTree[buffer].weight--;

	//将父节点，左孩子和右孩子全部初始化为-1
	int n = 0;
	for (i = 0; i < 256; i++)
		if (hufTree[i].weight != 0)
		{
			hufTree[i].ch = (unsigned char)i;
			n++; // 叶子数
			hufTree[i].lc = hufTree[i].rc = hufTree[i].parent = -1;
		}
	int m = 2 * n - 1;	//哈弗曼树结点总数
	j = 0;
	for (i = 0; i < 256; i++)
		if (hufTree[i].weight != 0)
		{
			hufTree[j] = hufTree[i];
			j++;
		}

	for (i = n; i < m; i++)	//初始化结点
	{
		hufTree[i].lc = hufTree[i].rc = -1;
		hufTree[i].parent = -1;
	}
	//建立哈弗曼树
	for (i = n; i < m; i++)
	{
		int min1 = findMin(hufTree, i - 1);
		hufTree[i].lc = min1;
		hufTree[min1].parent = i;
		int min2 = findMin(hufTree, i - 1);
		hufTree[i].rc = min2;
		hufTree[min2].parent = i;
		hufTree[i].weight = hufTree[min1].weight + hufTree[min2].weight;
	}

	getByte(hufTree, n);
	fseek(input, 0, SEEK_SET);
	fwrite(&FileLength, 4, 1, output);
	fseek(output, 8, SEEK_SET);	
	codes[0] = 0;
	long filelength = 0;
	//将编码信息写入目标文件
	while (!feof(input))
	{
		fread(&buffer, 1, 1, input);
		filelength++;
		for (i = 0; i < n; i++)
			if (buffer == hufTree[i].ch)
				break;
		strcat(codes, hufTree[i].byte);   //接入
		while (strlen(codes) >= 8)
		{
			// 将codes的前8位01代码表示的字符存入buffer
			for (i = 0; i < 8; i++)
				buffer = codes[i] == '1' ? (buffer << 1) | 1 : buffer << 1;
			//将新的字符写入目标文件
			fwrite(&buffer, 1, 1, output);
			sumlength++;
			strcpy(codes, codes + 8);
		}
		if (filelength == FileLength)
			break;
	}

	//再将剩余的不足8位的01代码补全8位，继续写入
	if (strlen(codes) > 0)
	{
		strcat(codes, "00000000");
		for (i = 0; i < 8; i++)
			buffer = codes[i] == '1' ? (buffer << 1) | 1 : buffer << 1;
		fwrite(&buffer, 1, 1, output);
		sumlength++;
	}

	sumlength += 8;

	fseek(output, 4, SEEK_SET);
	fwrite(&sumlength, 4, 1, output); //把sumlength写进目标文件的第5-8个字节里         
	fseek(output, sumlength, SEEK_SET);
	fwrite(&n, 4, 1, output);
	fwrite(profix, 1, 10, output);
	//把叶子数n写进编码段后面的4个字节的位置

	for (i = 0; i < n; i++)
	{
		fwrite(&(hufTree[i].ch), 1, 1, output);
		buffer = hufTree[i].codeLength;
		fwrite(&buffer, 1, 1, output);
		//写入字符的编码
		if (hufTree[i].codeLength % 8 != 0)
			//把编码不足8位的在低位补0,赋值给C，再把C写入
			for (j = hufTree[i].codeLength % 8; j < 8; j++)
				strcat(hufTree[i].byte, "0");
		
		// 开始存入编码，每8位二进制数存入一个字节
		while (hufTree[i].byte[0] != 0)
		{
			buffer = 0;
			for (j = 0; j < 8; j++)
				buffer = hufTree[i].byte[j] == '1' ? buffer = (buffer << 1) | 1 : buffer = buffer << 1;
			strcpy(hufTree[i].byte, hufTree[i].byte + 8);//编码前移8位，继续存入编码
			count++;									 //编码占的字节数的总值
			fwrite(&buffer, 1, 1, output);
		}
	}

	while (sumlength>100000)
	{
		sumlength /= 10;
		FileLength /= 10;
	}
	sumlength = sumlength + 4 + n * 2 + count; //计算压缩后文件的长度

	double tmp1 = sumlength, tmp2 = FileLength;

	double ratio = tmp1 / tmp2;

	fclose(input);
	fclose(output);

	return ratio;
}

// 从叶子向根求每个字符的哈弗曼编码
void Compresser::getByte(Node *node, int n)
{
	int start;
	int i, f, c;
	char codes[256];
	codes[n - 1] = '\0';
	for (i = 0; i < n; i++)
	{
		start = n - 1;
		for (c = i, f = node[i].parent; f != -1; c = f, f = node[f].parent)
		{
			start--;
			if (node[f].lc == c)
				codes[start] = '0';
			else 
				codes[start] = '1';
		}
		strcpy(node[i].byte, &codes[start]);
		node[i].codeLength = strlen(node[i].byte);
	}
}

// 找到无双亲结点的最小结点
int Compresser::findMin(Node *tree, int n)
{
	int i;
	int minNode = -1;
	for (i = 0; i <= n; i++)
	{
		if (tree[i].parent != -1)
			continue;
		if (minNode == -1)
			minNode = i;
		else if (tree[i].weight < tree[minNode].weight)
			minNode = i;
	}
	return minNode;
}

// 解压文件
bool Compresser::decompress(string path)
{
	Node hufTree[512];
	int i, j, k;
	for (i = 0; i < 512; i++)
		hufTree[i].weight = 0;
	FILE *input, *output;

	long FileLength, sumlength, filelength;
	int n, m;
	char buf[256], codes[256];
	unsigned char buffer;
	int maxlength;
	char profix[10];

	input = fopen(&path[0], "rb");
	if (input == NULL)
	{
		cout << "Fail in opening file." << endl;
		return false;
	}

	// 从压缩文件读出FileLength、sumlength
	fread(&FileLength, 4, 1, input);
	fread(&sumlength, 4, 1, input);
	//利用sumlength读出n的值
	fseek(input, sumlength, SEEK_SET);
	fread(&n, 4, 1, input);

	fread(profix, 1, 10, input);
	for (i = path.size(); path[i] != '.' && i >= 0; i--);
	string outPath = path;
	for (j = 0; profix[j] != '\0'; j++)
		outPath[i + j + 1] = profix[j];
	outPath[i + j + 1] = profix[j];

	output = fopen(&outPath[0], "wb");
	if (output == NULL)
	{
		cout << "Fail in opening file." << endl;
		return false;
	}

	// 读结点信息
	for (i = 0; i < n; i++)
	{
		// 字符
		fread(&hufTree[i].ch, 1, 1, input);
		//编码长度
		fread(&buffer, 1, 1, input);
		hufTree[i].codeLength = buffer;
		hufTree[i].byte[0] = 0;
		if (hufTree[i].codeLength % 8 > 0) m = hufTree[i].codeLength / 8 + 1;// m为编码占的字节数
		else m = hufTree[i].codeLength / 8;
		// 根据字节长度m读出编码
		for (j = 0; j < m; j++)
		{
			fread(&buffer, 1, 1, input);
			itoa(buffer, buf, 2);
			
			for (k = 8; k>strlen(buf); k--)
				strcat(hufTree[i].byte, "0");
			//再把二进制编码存进hfmnode.code中
			strcat(hufTree[i].byte, buf);
		}
		// 去掉编码中多余的0  
		hufTree[i].byte[hufTree[i].codeLength] = 0;
	}
	//找出编码长度的最大值
	maxlength = 0;
	for (i = 0; i < n; i++)
		if (hufTree[i].codeLength>maxlength)
			maxlength = hufTree[i].codeLength;
	//开始写入目标文件
	fseek(input, 8, SEEK_SET);
	filelength = 0;
	codes[0] = 0;
	buf[0] = 0;
	while (1)
	{
		// codes小于编码长度的最大值时，继续读码
		while (strlen(codes) < maxlength)
		{
			fread(&buffer, 1, 1, input);
			itoa(buffer, buf, 2);
			for (k = 8; k > strlen(buf); k--)
				strcat(codes, "0");
			strcat(codes, buf);
		}
		for (i = 0; i < n; i++)
		{
			// 在codes中查找能使其前weight位和hfmnode.code相同的i值，weight即为codelength
			if (memcmp(hufTree[i].byte, codes, (unsigned int)hufTree[i].codeLength) == 0)
				break;
		}
		strcpy(codes, codes + hufTree[i].codeLength);
		buffer = hufTree[i].ch;
		fwrite(&buffer, 1, 1, output);

		filelength++;
		if (filelength == FileLength) break;
	}

	fclose(input);
	fclose(output);
	return true;
}
