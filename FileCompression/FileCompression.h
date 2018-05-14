#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_FileCompression.h"

class FileCompression : public QMainWindow
{
	Q_OBJECT

public:
	FileCompression(QWidget *parent = Q_NULLPTR);

private:
	Ui::FileCompressionClass ui;

private slots:
	void compressBtnClicked();
	void decompressBtnClicked();
};
