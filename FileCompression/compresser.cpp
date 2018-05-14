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

	// ȡ���ļ���׺��
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

	//���ַ���ASCII��ֵ����±�
	long FileLength = 0;
	while (!feof(input)) // �ж��Ƿ����ļ�β��
	{
		fread(&buffer, 1, 1, input); // ��ȡ����
		hufTree[buffer].weight++; // ����Ȩֵ
		FileLength++;
	}

	FileLength--;
	hufTree[buffer].weight--;

	//�����ڵ㣬���Ӻ��Һ���ȫ����ʼ��Ϊ-1
	int n = 0;
	for (i = 0; i < 256; i++)
		if (hufTree[i].weight != 0)
		{
			hufTree[i].ch = (unsigned char)i;
			n++; // Ҷ����
			hufTree[i].lc = hufTree[i].rc = hufTree[i].parent = -1;
		}
	int m = 2 * n - 1;	//���������������
	j = 0;
	for (i = 0; i < 256; i++)
		if (hufTree[i].weight != 0)
		{
			hufTree[j] = hufTree[i];
			j++;
		}

	for (i = n; i < m; i++)	//��ʼ�����
	{
		hufTree[i].lc = hufTree[i].rc = -1;
		hufTree[i].parent = -1;
	}
	//������������
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
	//��������Ϣд��Ŀ���ļ�
	while (!feof(input))
	{
		fread(&buffer, 1, 1, input);
		filelength++;
		for (i = 0; i < n; i++)
			if (buffer == hufTree[i].ch)
				break;
		strcat(codes, hufTree[i].byte);   //����
		while (strlen(codes) >= 8)
		{
			// ��codes��ǰ8λ01�����ʾ���ַ�����buffer
			for (i = 0; i < 8; i++)
				buffer = codes[i] == '1' ? (buffer << 1) | 1 : buffer << 1;
			//���µ��ַ�д��Ŀ���ļ�
			fwrite(&buffer, 1, 1, output);
			sumlength++;
			strcpy(codes, codes + 8);
		}
		if (filelength == FileLength)
			break;
	}

	//�ٽ�ʣ��Ĳ���8λ��01���벹ȫ8λ������д��
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
	fwrite(&sumlength, 4, 1, output); //��sumlengthд��Ŀ���ļ��ĵ�5-8���ֽ���         
	fseek(output, sumlength, SEEK_SET);
	fwrite(&n, 4, 1, output);
	fwrite(profix, 1, 10, output);
	//��Ҷ����nд������κ����4���ֽڵ�λ��

	for (i = 0; i < n; i++)
	{
		fwrite(&(hufTree[i].ch), 1, 1, output);
		buffer = hufTree[i].codeLength;
		fwrite(&buffer, 1, 1, output);
		//д���ַ��ı���
		if (hufTree[i].codeLength % 8 != 0)
			//�ѱ��벻��8λ���ڵ�λ��0,��ֵ��C���ٰ�Cд��
			for (j = hufTree[i].codeLength % 8; j < 8; j++)
				strcat(hufTree[i].byte, "0");
		
		// ��ʼ������룬ÿ8λ������������һ���ֽ�
		while (hufTree[i].byte[0] != 0)
		{
			buffer = 0;
			for (j = 0; j < 8; j++)
				buffer = hufTree[i].byte[j] == '1' ? buffer = (buffer << 1) | 1 : buffer = buffer << 1;
			strcpy(hufTree[i].byte, hufTree[i].byte + 8);//����ǰ��8λ�������������
			count++;									 //����ռ���ֽ�������ֵ
			fwrite(&buffer, 1, 1, output);
		}
	}

	while (sumlength>100000)
	{
		sumlength /= 10;
		FileLength /= 10;
	}
	sumlength = sumlength + 4 + n * 2 + count; //����ѹ�����ļ��ĳ���

	double tmp1 = sumlength, tmp2 = FileLength;

	double ratio = tmp1 / tmp2;

	fclose(input);
	fclose(output);

	return ratio;
}

// ��Ҷ�������ÿ���ַ��Ĺ���������
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

// �ҵ���˫�׽�����С���
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

// ��ѹ�ļ�
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

	// ��ѹ���ļ�����FileLength��sumlength
	fread(&FileLength, 4, 1, input);
	fread(&sumlength, 4, 1, input);
	//����sumlength����n��ֵ
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

	// �������Ϣ
	for (i = 0; i < n; i++)
	{
		// �ַ�
		fread(&hufTree[i].ch, 1, 1, input);
		//���볤��
		fread(&buffer, 1, 1, input);
		hufTree[i].codeLength = buffer;
		hufTree[i].byte[0] = 0;
		if (hufTree[i].codeLength % 8 > 0) m = hufTree[i].codeLength / 8 + 1;// mΪ����ռ���ֽ���
		else m = hufTree[i].codeLength / 8;
		// �����ֽڳ���m��������
		for (j = 0; j < m; j++)
		{
			fread(&buffer, 1, 1, input);
			itoa(buffer, buf, 2);
			
			for (k = 8; k>strlen(buf); k--)
				strcat(hufTree[i].byte, "0");
			//�ٰѶ����Ʊ�����hfmnode.code��
			strcat(hufTree[i].byte, buf);
		}
		// ȥ�������ж����0  
		hufTree[i].byte[hufTree[i].codeLength] = 0;
	}
	//�ҳ����볤�ȵ����ֵ
	maxlength = 0;
	for (i = 0; i < n; i++)
		if (hufTree[i].codeLength>maxlength)
			maxlength = hufTree[i].codeLength;
	//��ʼд��Ŀ���ļ�
	fseek(input, 8, SEEK_SET);
	filelength = 0;
	codes[0] = 0;
	buf[0] = 0;
	while (1)
	{
		// codesС�ڱ��볤�ȵ����ֵʱ����������
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
			// ��codes�в�����ʹ��ǰweightλ��hfmnode.code��ͬ��iֵ��weight��Ϊcodelength
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
