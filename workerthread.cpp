#include "workerthread.h"
#include <QDebug>

Worker::Worker(
	QObject *parent)
	: QObject{parent}
{}

Controller::Controller()
{
	pWorker = new Worker;
	if ( pWorker == NULL )		//sendMessage, problem
	{
		emit sendControllerStringBasic( tr( "Error!  Worker thread pointer is NULL." ) );
		return;
	}
	//qDebug() << "Controller constructor";
	pWorker->moveToThread( &workerThread );
	pWorker->setbRunning( false );
	connect( &workerThread, &QThread::finished, pWorker, &QObject::deleteLater );
	connect( this, &Controller::sendWorkerStartSignal, pWorker, &Worker::doCopyFiles );
	connect( pWorker, &Worker::resultReady, this, &Controller::handleResults );
	connect( pWorker, &Worker::sendWorkerStringBasic, this, &Controller::receiveControllerStringBasic );
	connect( this, &Controller::sendWorkerInterruptSignal, &Controller::InterruptWorker );
	workerThread.start();
}

Controller::~Controller()
{
	if ( pWorker == NULL )
		return;
	workerThread.quit();
	workerThread.wait();
}

bool Controller::bIsWorkerRunning()
{
	if ( pWorker == NULL )
		return false;
	if ( pWorker->bIsRunning() )
		return true;
	return false;
}

void Controller::handleResults( const QString s )
{
	if ( pWorker == NULL )		// not possible?
		return;
	if ( !pWorker->bIsRunning() )		// interrupted
		return;
	pWorker->setbRunning(false);
	if ( s.length() )
		emit sendControllerStringBasic( s );
	emit sendWorkerFinishedSignal();
	//qDebug() << "Controller::Result handler called.";
}

void Controller::receiveControllerStringBasic( QString s )
{
	emit sendControllerStringBasic( s );
}

void Controller::InterruptWorker()
{
	if ( pWorker == NULL )
		return;
	pWorker->setbRunning( false );
	emit sendControllerStringBasic( tr("Copying interrupted, aborting..." ) );
}

void Worker::doCopyFiles( QString sDLPath, QString sGamePath )
{
	QStringList slFilters;
	QFileInfoList fiList;
	qint32 i, j;
	QString sFileNameSource, sFileNameTarget;
	QFile * pFileSource, * pFileTarget;

	this->setbRunning( true );
	QDir dirSource( sDLPath );
	QDir dirSourceData( sDLPath + "/Data" );
	slFilters << "*";		// "*.*" works but don't find all entries!
	fiList = dirSource.entryInfoList( slFilters, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot );
	if ( fiList.isEmpty() )
	{
		//qDebug() << "fiList is empty.";
		emit sendWorkerStringBasic( tr( "ERROR: No downloaded files found in %1.\n").arg( dirSource.absolutePath() ) );
		return;
	}
	fiList.append( dirSourceData.entryInfoList( slFilters, QDir::Files | QDir::NoDotAndDotDot ) );
	for ( i = fiList.size() - 1; i >= 0; i-- )
	{
		if ( fiList.at( i ).filePath().contains( ".DepotDownloader", Qt::CaseInsensitive ) )
			fiList.removeAt( i );
		if ( fiList.at( i ).filePath() == sDLPath + "/Data" )
			fiList.removeAt( i );
	}
	emit sendWorkerStringBasic( tr( "Copying files..." ) );
	for ( i = 0; i < fiList.size(); i++ )
	{
		if ( !this->bIsRunning() )						// interrupted
		{
			emit sendWorkerStringBasic( tr("Copying aborted." ) );
			break;
		}
		sFileNameSource = fiList.at( i ).filePath();
		pFileSource = new QFile( sFileNameSource );
		if ( pFileSource == NULL )
			continue;
		sFileNameTarget = sGamePath;
		if ( !sFileNameTarget.endsWith( '/' ) )
			sFileNameTarget.append( '/' );
		j = sFileNameSource.lastIndexOf( "/Data", Qt::CaseInsensitive );
		if ( j > -1 )
			sFileNameTarget += "Data/";
		sFileNameTarget += fiList.at( i ).fileName();	//pFileSource->fileName();
		pFileTarget = new QFile( sFileNameTarget );
		if ( pFileTarget == NULL )
		{
			if ( pFileSource != NULL )
				delete pFileSource;
			continue;
		}
		if ( pFileTarget->exists() )
			pFileTarget->remove();
		pFileSource->copy( pFileTarget->fileName() );				// basic, slow
		//qDebug() << "copied to: " << pFileTarget->fileName();
		emit sendWorkerStringBasic( sFileNameTarget );		//sFileNameSource + "->" +
		if ( pFileSource != NULL )
			delete pFileSource;
		if ( pFileTarget != NULL )
			delete pFileTarget;
	}
	emit resultReady( tr( "Copying files finished." ) );
}
