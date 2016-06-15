#include <QApplication>
#include <QFile>
#include <QTime>

#include "serialconnector.h"
#include "gui.h"

int main(int argc, char *argv[])
{
	qsrand(QTime::currentTime().msec());

	QApplication app{argc, argv};

	{
		// Load QDarkStyleSheet
		QFile style(":qdarkstyle/style.qss");
		Q_ASSERT(style.exists());
		style.open(QFile::ReadOnly | QFile::Text);
		app.setStyleSheet(style.readAll());
	}

	SerialConnector connector;

	GUI gui{&connector};

	gui.show();
	return app.exec();
}
