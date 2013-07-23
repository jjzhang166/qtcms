#ifndef QPREVIEWACTIVITY_H
#define QPREVIEWACTIVITY_H

#include <QObject>
#include <QWebFrame>
#include "IActiveFrame.h"

class QPreviewActivity : public QObject,
        public IActiveFrame
{
    Q_OBJECT
public:
    explicit QPreviewActivity(QObject *parent = 0);

	virtual void Active( QWebFrame *Frame );

signals:
    
public slots:
	void OnJavaScriptWindowObjectCleared();
	void OnTopActDbClick();

private:
	QWebFrame * m_Frame;
    
};

#endif // QPREVIEWACTIVITY_H
