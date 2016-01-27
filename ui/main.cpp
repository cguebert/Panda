#include <QApplication>

#include <ui/MainWindow.h>

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
	QApplication app(argc, argv);
	MainWindow mainWin;
	mainWin.show();
	return app.exec();
}
