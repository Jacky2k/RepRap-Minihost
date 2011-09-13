/*
 * ManualCommandFilter.h
 *
 *  Created on: 13.09.2011
 *      Author: fabian
 */

#ifndef MANUALCOMMANDFILTER_H_
#define MANUALCOMMANDFILTER_H_

#include <QObject>

class ManualCommandFilter : public QObject
{
	Q_OBJECT
public:
	ManualCommandFilter();
	virtual ~ManualCommandFilter();
	
	bool eventFilter(QObject* watched, QEvent* event);
	
signals:
	void returnHit();
};

#endif /* MANUALCOMMANDFILTER_H_ */
