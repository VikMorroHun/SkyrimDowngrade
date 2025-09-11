#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include <QObject>
#include <QThread>
#include <QDir>

/* ***************************************************************************************************************************************** *
 * It is important to remember that a QThread instance lives in the old thread that instantiated it, not in the new thread that calls run(). *
 * This means that all of QThread's queued slots and invoked methods will execute in the old thread. Thus, a developer who wishes to invoke  *
 * slots in the new thread must use the worker-object approach; new slots should not be implemented directly into a subclassed QThread.      *
 * ***************************************************************************************************************************************** */

/* **************************************************************************************************************************************** *
 * Note: If a QObject has no thread affinity (that is, if thread() returns zero), or if it lives in a thread that has no running event      *
 * loop, then it cannot receive queued signals or posted events.																		    *
 * **************************************************************************************************************************************** */

class Worker : public QObject
{
	Q_OBJECT
public:
	explicit Worker(QObject *parent = nullptr);
	bool bIsRunning() { return bRunning; }
	void setbRunning( bool b ) { bRunning = b; }

public slots:
	void doCopyFiles(QString, QString );					/* ... here is the expensive or blocking operation ... */

private:
	bool bRunning;

signals:
	void resultReady(const QString &result);
	void sendWorkerStringBasic( QString );
	/*void sendWorkerProgressReport(int);
	void sendWorkerStringPointer( QString * );
	void workerFinished( strucShared *);
		*/

};

//*********************************

class Controller : public QObject
{
	Q_OBJECT
	QThread workerThread;
public:
	Controller();
	~Controller();
	bool bIsWorkerRunning();
	Worker * pWorker;

public slots:
	void handleResults(const QString );
	void receiveControllerStringBasic( QString );
	void InterruptWorker();

signals:
	//void operate(const QString &);
	void sendControllerStringBasic( const QString );
	void sendWorkerStartSignal( QString, QString );
	void sendWorkerInterruptSignal();
	void sendWorkerFinishedSignal();
};
#endif // WORKERTHREAD_H
