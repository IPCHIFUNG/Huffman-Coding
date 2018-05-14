#include "FileCompression.h"
#include "compresser.h"

#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>

FileCompression::FileCompression(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	connect(ui.compressionBtn, SIGNAL(clicked(bool)), this, SLOT(compressBtnClicked()));
	connect(ui.decompressionBtn, SIGNAL(clicked(bool)), this, SLOT(decompressBtnClicked()));
}

void FileCompression::compressBtnClicked()
{
	// 打开文件选择器
	QFileDialog fd;
	fd.setAcceptMode(QFileDialog::AcceptSave);
	fd.setViewMode(QFileDialog::Detail);
	fd.setFileMode(QFileDialog::ExistingFile);
	fd.setWindowTitle(QString::fromLocal8Bit("选择需要压缩的文件"));
	QStringList filters;
	filters << QString::fromLocal8Bit("所有文件 (*.*)");
	fd.setNameFilters(filters);
	if (fd.exec() != QFileDialog::Accepted)
		return;

	string inPath = fd.selectedFiles()[0].toStdString();
	int i = inPath.size();
	while (inPath[i] != '.' && i >= 0)
		i--;
	string outPath = inPath.substr(0, i + 1) + "huffman";

	double ratio = Compresser::compress(inPath, outPath);
	if (ratio >= 0)
	{
		QString msg = QString::fromLocal8Bit("压缩成功，压缩率为：") + QString::number(ratio, 5, 10);
		QMessageBox::information(NULL, QString::fromLocal8Bit("压缩"), msg, QMessageBox::Yes, QMessageBox::Yes);
	}
	else
		QMessageBox::information(NULL, QString::fromLocal8Bit("压缩"), QString::fromLocal8Bit("压缩失败"), QMessageBox::Yes, QMessageBox::Yes);
}

void FileCompression::decompressBtnClicked()
{
	// 选择压缩文件位置
	QFileDialog fd;
	fd.setAcceptMode(QFileDialog::AcceptOpen);
	fd.setViewMode(QFileDialog::Detail);
	fd.setFileMode(QFileDialog::ExistingFile);
	fd.setWindowTitle(QString::fromLocal8Bit("选择文件位置"));
	QStringList filters;
	filters << QString::fromLocal8Bit("哈夫曼压缩格式 (*.huffman)");
	fd.setNameFilters(filters);
	if (fd.exec() != QFileDialog::Accepted)
		return;

	string path = fd.selectedFiles()[0].toStdString();

	if (Compresser::decompress(path))
		QMessageBox::information(NULL, QString::fromLocal8Bit("解压"), QString::fromLocal8Bit("解压成功"), QMessageBox::Yes, QMessageBox::Yes);
	else
		QMessageBox::information(NULL, QString::fromLocal8Bit("解压"), QString::fromLocal8Bit("解压失败"), QMessageBox::Yes, QMessageBox::Yes);
}