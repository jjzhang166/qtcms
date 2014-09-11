#ifndef MDWORKOBJECT_H
#define MDWORKOBJECT_H

#include <QObject>

class MDWorkObject : public QObject
{
	Q_OBJECT

public:
	MDWorkObject(QObject *parent);
	~MDWorkObject();

private:

public slots:
	void motionDetectionEcho();
	
};

#endif // MDWORKOBJECT_H
