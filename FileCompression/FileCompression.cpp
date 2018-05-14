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
	// ���ļ�ѡ����
	QFileDialog fd;
	fd.setAcceptMode(QFileDialog::AcceptSave);
	fd.setViewMode(QFileDialog::Detail);
	fd.setFileMode(QFileDialog::ExistingFile);
	fd.setWindowTitle(QString::fromLocal8Bit("ѡ����Ҫѹ�����ļ�"));
	QStringList filters;
	filters << QString::fromLocal8Bit("�����ļ� (*.*)");
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
		QString msg = QString::fromLocal8Bit("ѹ���ɹ���ѹ����Ϊ��") + QString::number(ratio, 5, 10);
		QMessageBox::information(NULL, QString::fromLocal8Bit("ѹ��"), msg, QMessageBox::Yes, QMessageBox::Yes);
	}
	else
		QMessageBox::information(NULL, QString::fromLocal8Bit("ѹ��"), QString::fromLocal8Bit("ѹ��ʧ��"), QMessageBox::Yes, QMessageBox::Yes);
}

void FileCompression::decompressBtnClicked()
{
	// ѡ��ѹ���ļ�λ��
	QFileDialog fd;
	fd.setAcceptMode(QFileDialog::AcceptOpen);
	fd.setViewMode(QFileDialog::Detail);
	fd.setFileMode(QFileDialog::ExistingFile);
	fd.setWindowTitle(QString::fromLocal8Bit("ѡ���ļ�λ��"));
	QStringList filters;
	filters << QString::fromLocal8Bit("������ѹ����ʽ (*.huffman)");
	fd.setNameFilters(filters);
	if (fd.exec() != QFileDialog::Accepted)
		return;

	string path = fd.selectedFiles()[0].toStdString();

	if (Compresser::decompress(path))
		QMessageBox::information(NULL, QString::fromLocal8Bit("��ѹ"), QString::fromLocal8Bit("��ѹ�ɹ�"), QMessageBox::Yes, QMessageBox::Yes);
	else
		QMessageBox::information(NULL, QString::fromLocal8Bit("��ѹ"), QString::fromLocal8Bit("��ѹʧ��"), QMessageBox::Yes, QMessageBox::Yes);
}