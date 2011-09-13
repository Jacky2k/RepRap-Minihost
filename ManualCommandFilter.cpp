/*
 * ManualCommandFilter.cpp
 *
 *  Created on: 13.09.2011
 *      Author: fabian
 */

#include "ManualCommandFilter.h"

#include <iostream>
#include <QEvent>
#include <QKeyEvent>

using namespace std;

ManualCommandFilter::ManualCommandFilter()
{
	
}

ManualCommandFilter::~ManualCommandFilter()
{
	
}

bool ManualCommandFilter::eventFilter(QObject*, QEvent* event)
{
	if(event->type() == QEvent::KeyPress)
	{
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return)
		{
			emit returnHit();
			cout<<"emmitted..."<<endl;
			return true;
		}
		else
		{
			return false;
		}
    }
	return false;
}

