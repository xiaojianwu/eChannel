#ifndef ECHANNEL_H
#define ECHANNEL_H

#include <QtWidgets/QMainWindow>
#include "ui_echannel.h"

class eChannel : public QMainWindow
{
	Q_OBJECT

public:
	eChannel(QWidget *parent = 0);
	~eChannel();

private:
	Ui::eChannelClass ui;
};

#endif // ECHANNEL_H
