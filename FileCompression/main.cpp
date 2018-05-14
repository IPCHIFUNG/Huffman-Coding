#include "FileCompression.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	FileCompression w;
	w.show();
	return a.exec();
}
