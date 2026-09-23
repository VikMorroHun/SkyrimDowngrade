/***************************************************************************
 *   Copyright (C) 2024 by VikMorroHun                                     *
 *   luke2135@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 3 of the License.               *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses>.   *
 ***************************************************************************/

/*	SzőtsÁki: NEM A Qt A H****, HANEM A PROGRAMOZÓ.  "FALLOUT4" ESETE.  MEG ui->comboBoxGame->clear() + PrefetchAppName() == "complete nonsense"

*/
#include "skmainwindow.h"
#include "ui_skmainwindow.h"

SKMainWindow::SKMainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::SKMainWindow)
{
	int index;

	ui->setupUi(this);
	fileMenu = ui->menubar->addMenu( "&File" );					//https://stackoverflow.com/questions/41367027/qt-add-menubar-menus-and-sub-menus-to-qmainwindow
	//auto submenu = fileMenu->addMenu("Submenu");
	exitAction = browseActionGame = openXMLAction = NULL;		// otherwise crash at startup if not exists
	exitAction = new QAction( tr("E&xit"), this);			//newAct = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentNew), tr("&New"), this);
	browseActionGame = new QAction( tr("&Browse game"), this );
	browseActionDownload = new QAction( tr( "Browse &Download folder" ), this );
	ui->comboBoxGame->clear();ui->comboBoxVersion->clear();			// after connecting, it calls XMLReader::ReadXMLSL via on_comboBoxGameCurrentTextChanged!!!
	connect( ui->comboBoxVersion, &QComboBox::activated, this, &SKMainWindow::on_comboBoxVersionActivated );
	connect( ui->pushButtonAbort, &QPushButton::clicked, this, &SKMainWindow::on_pushButtonAbortClicked );
	connect( ui->pushButtonBrowse, &QPushButton::clicked, this, &SKMainWindow::on_pushButtonBrowseClicked );
	connect( ui->pushButtonBrowse2, &QPushButton::clicked, this, &SKMainWindow::on_pushButtonBrowse2Clicked );
	connect( ui->pushButtonDownload, &QPushButton::clicked, this, &SKMainWindow::on_pushButtonDownloadClicked );
	connect( ui->pushButtonSubwin, &QPushButton::clicked, this, &SKMainWindow::on_pushButtonSubwinClicked );
	if ( browseActionGame != NULL )
	{
		fileMenu->addAction( browseActionGame );
		connect( browseActionGame, &QAction::triggered, this, &SKMainWindow::on_pushButtonBrowseClicked );
		browseActionGame->setStatusTip( tr("Set game install location") );
	}
	if ( browseActionDownload != NULL )
	{
		fileMenu->addAction( browseActionDownload );
		connect( browseActionDownload, &QAction::triggered, this, &SKMainWindow::on_pushButtonBrowse2Clicked );
		browseActionDownload->setStatusTip( tr("Set download folder location") );
	}
	startDir = QDir::current();
	pProcessDL = NULL;										// to avoid possible segmentation fault
	if ( !TESTMODE )
		ui->pushButtonSubwin->setVisible( false );
	if ( TESTMODE == 1 )
		openXMLAction = new QAction( tr("Open XML"), this );
	if ( openXMLAction != NULL )
	{
		openXMLAction->setStatusTip( tr("Open XML file for testing") );
		fileMenu->addAction( openXMLAction );		// for testing
		connect( openXMLAction, &QAction::triggered, this, &SKMainWindow::on_OpenXMLMenuClicked );
	}
	if ( exitAction != NULL )
	{
		exitAction->setStatusTip( tr("Close the program") );
		fileMenu->addAction( exitAction );
		connect( exitAction, &QAction::triggered, this, &SKMainWindow::on_ExitMenuClicked );	//https://doc.qt.io/qt-6/qtwidgets-mainwindows-menus-example.html
	}
	pThreadControl = new Controller();
	if ( pThreadControl == NULL )
		ui->textEdit->append( tr( "Warning!  ThreadControl pointer problem at startup!" ) );
	else
	{
		connect( pThreadControl, &Controller::sendControllerStringBasic, this, &SKMainWindow::on_XMLReceiveStringBasic );
		connect( pThreadControl, &Controller::sendWorkerFinishedSignal, this, &SKMainWindow::FinalizeDowngrade2 );
		//pThreadControl->sendControllerStringBasic( "Controller Activated :)");
	}

	this->setWindowTitle("Skyrim Downgrade Utility");
	ui->pushButtonBrowse->setIcon( QIcon::fromTheme("system-file-manager") );
	ui->pushButtonBrowse2->setIcon( QIcon::fromTheme("system-file-manager") );
	pXMLReader = new XMLReader();
	if ( pXMLReader != NULL )
		connect( pXMLReader, &XMLReader::XMLSendStringBasicSignal, this, &SKMainWindow::on_XMLReceiveStringBasic );
	else ui->textEdit->append( tr("XMLReader() critical error at startup!") );
	pMainShared = new strucShared;
	if ( pMainShared == NULL )
		ui->textEdit->append( tr( "Critical ERROR!  Shared data pointer problem at startup, program won't work!" ) );
	else this->ResetSharedStruct();
	//pXMLReader->TestMsgSender();		// it works! :)
	QSettings WindowsRegSkyrimSE("HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Bethesda Softworks\\Skyrim Special Edition", QSettings::NativeFormat);
	sGamePathSkyrim = WindowsRegSkyrimSE.value("installed path", "").toString();								// here it can read it.  Later it can't.
	QSettings WindowsRegFallout4("HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Bethesda Softworks\\Fallout4", QSettings::NativeFormat);
	sGamePathFallout4 = WindowsRegFallout4.value("installed path", "").toString();
	this->PrefetchAppName();
	if ( ( index = ui->comboBoxGame->findText( "Skyrim SE/AE" ) ) > -1 )
		ui->comboBoxGame->setCurrentIndex( index );
	connect( ui->comboBoxGame, &QComboBox::currentTextChanged, this, &SKMainWindow::on_comboBoxGameCurrentTextChanged );		// must be after ui->comboBoxGame initialization!
	this->SetGameDefinitions();
	this->GameInstallLocationOutput();
	pProcessDL = new QProcess( this );
	if ( pProcessDL == NULL )
		ui->textEdit->append( tr( "Critical ERROR!  Can't start new process - can't download from Steam." ) );
	else
	{
		connect( pProcessDL, &QProcess::readyReadStandardOutput, this, &SKMainWindow::on_ReadStdOutputDL );
		connect( pProcessDL, &QProcess::finished, this, &SKMainWindow::on_processDLFinished );
		connect( pProcessDL, &QProcess::started, this, &SKMainWindow::on_processStarted );
	}
	pProcessCopyFast = new QProcess( this );
	if ( pProcessCopyFast != NULL )
	{
		connect( pProcessCopyFast, &QProcess::readyReadStandardOutput, this, &SKMainWindow::on_ReadStdOutputCopyFast );
		connect( pProcessCopyFast, &QProcess::finished, this, &SKMainWindow::on_processCopyFastFinished );
		connect( pProcessCopyFast, &QProcess::started, this, &SKMainWindow::on_processStarted );
	}
	if ( TESTMODE == 2 )		// public beta
	{
		ui->comboBoxGame->setVisible( false );ui->label_6->setVisible( false );
		ui->comboBoxVersion->setVisible( false );ui->label_7->setVisible( false );
	}
	ui->textEdit->append( tr( "YOU HAVE TO CLOSE STEAM BEFORE DOWNGRADING BECAUSE DEPOTDOWNLOADER WON'T WORK OTHERWISE.") );ui->lineEditPW->setEchoMode( QLineEdit::Password );
	ui->statusbar->showMessage( tr("Downgrade utility started." ), 2000);
/*#if OSTYPE == OSLINUX
	ui->textEdit->append( "\nTEST MODE STARTED!!\n");
	ui->lineEditGamePath->setText("/mnt/QVO 2TB_Games/SteamLibrary/steamapps/common/Skyrim Special Edition");
	ui->lineEditDownloadPath->setText( "/mnt/drive_d/Skyrim Downgrader");
#endif	*/
}

//+------------------------------------------------------------------+
//| Destructor                                                       |
//| INPUT: none                                                      |
//| OUTPUT: none                                                     |
//| REMARK: cleanup everything                                       |
//+------------------------------------------------------------------+
SKMainWindow::~SKMainWindow()
{
	if ( exitAction != NULL )
		delete exitAction;
	if ( browseActionGame != NULL )
		delete browseActionGame;
	if ( browseActionDownload != NULL )
		delete browseActionDownload;
	if ( pXMLReader != NULL )
		delete pXMLReader;
	if ( openXMLAction != NULL )
		delete openXMLAction;
	if ( pMainShared != NULL )
		delete pMainShared;
	if ( pProcessDL != NULL )
	{
		if ( pProcessDL->state() == QProcess::Running )
			pProcessDL->kill();
		delete pProcessDL;
	}
	if ( pThreadControl != NULL )
	{
		if ( pThreadControl->bIsWorkerRunning() )
			pThreadControl->sendWorkerInterruptSignal();
		delete pThreadControl;
	}
	if ( pProcessCopyFast != NULL )
	{
		if ( pProcessCopyFast->state() == QProcess::Running )
			pProcessCopyFast->kill();
		delete pProcessCopyFast;
	}
	if ( pSubwindow )
		delete pSubwindow;
	delete ui;
}

//+------------------------------------------------------------------+
//| Copy downloaded files to game folder                             |
//| INPUT: none                                                      |
//| OUTPUT: none                                                     |
//| REMARK: after download completed, old way                        |
//+------------------------------------------------------------------+
void SKMainWindow::CopyFiles()
{
	QStringList slFilters;
	QFileInfoList fiList;
	qint32 i, j;
	QString sFileNameSource, sFileNameTarget;
	QFile * pFileSource, * pFileTarget;

	if ( pMainShared == NULL )
		return;
	QDir dirSource( ui->lineEditDownloadPath->text() );//QDir dirDest( ui->lineEditGamePath->text() );
	QDir dirSourceData( ui->lineEditDownloadPath->text() + "/Data" );
	slFilters << "*";		// "*.*" works but don't find all entries!
	fiList = dirSource.entryInfoList( slFilters, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot );
	if ( fiList.isEmpty() )
	{
		//qDebug() << "fiList is empty.";
		ui->textEdit->append( tr( "ERROR: No downloaded files found in %1.\n").arg( dirSource.absolutePath() ) );
		return;
	}
	fiList.append( dirSourceData.entryInfoList( slFilters, QDir::Files | QDir::NoDotAndDotDot ) );
	for ( i = fiList.size() - 1; i >= 0; i-- )
	{
		if ( fiList.at( i ).filePath().contains( ".DepotDownloader", Qt::CaseInsensitive ) )
			fiList.removeAt( i );
		if ( fiList.at( i ).filePath() == ui->lineEditDownloadPath->text() + "/Data" )
			fiList.removeAt( i );
	}
	ui->textEdit->append( tr( "\nCopying files...") );
	for ( i = 0; i < fiList.size(); i++ )
	{
		sFileNameSource = fiList.at( i ).filePath();
		pFileSource = new QFile( sFileNameSource );
		if ( pFileSource == NULL )
		{
			ui->textEdit->append( tr( "Cannot copy file %1.").arg( sFileNameSource ) );
			continue;
		}
		sFileNameTarget = ui->lineEditGamePath->text();
		if ( !sFileNameTarget.endsWith( '/' ) )
			sFileNameTarget.append( '/' );
		j = sFileNameSource.lastIndexOf( "/Data", Qt::CaseInsensitive );
		if ( j > -1 )
			sFileNameTarget += "Data/";
		sFileNameTarget += fiList.at( i ).fileName();
		pFileTarget = new QFile( sFileNameTarget );
		if ( pFileTarget == NULL )
		{
			ui->textEdit->append( tr( "Cannot copy file %1.").arg( sFileNameTarget ) );
			if ( pFileSource != NULL )
				delete pFileSource;
			continue;
		}
		if ( pFileTarget->exists() )
			pFileTarget->remove();
		pFileSource->copy( pFileTarget->fileName() );
		//qDebug() << "copied to: " << pFileTarget->fileName();
		ui->textEdit->append( sFileNameTarget );		//sFileNameSource + "->" +
		if ( pFileSource != NULL )
			delete pFileSource;
		if ( pFileTarget != NULL )
			delete pFileTarget;
	}
}

//+------------------------------------------------------------------+
//| Delete unnecessary files from target folder                      |
//| INPUT: none                                                      |
//| OUTPUT: none                                                     |
//| REMARK: after download completed                                 |
//+------------------------------------------------------------------+
void SKMainWindow::DeleteFiles()
{
	QString sFileName;
	QFile * pFile;

	if ( pMainShared == NULL )
		return;
	if ( !pMainShared->slDeleteFiles.size() )
	{
		ui->textEdit->append( tr( "It seems there is no need to delete files from the game folder." ) );
		return;
	}
	sGamePath = ui->lineEditGamePath->text();
	if ( !sGamePath.endsWith('/') )
		sGamePath.append('/');
	for ( int i = 0; i < pMainShared->slDeleteFiles.size(); i++ )
	{
		sFileName = sGamePath + pMainShared->slDeleteFiles.at(i);
		pFile = new QFile( sFileName );
		if ( pFile != NULL )
		{
			if ( pFile->exists() )
			{
				if ( pFile->remove() )
					ui->textEdit->append( tr( "%1 has been deleted.").arg( pFile->fileName() ) );
				else ui->textEdit->append( tr( "Cannot delete %1.").arg( pFile->fileName() ) );
			}
			delete pFile;
		}
	}
}

//+------------------------------------------------------------------+
//| Set game install location                                        |
//| INPUT: none                                                      |
//| OUTPUT: path to line and text edit fields                        |
//| REMARK: none                                                     |
//+------------------------------------------------------------------+
void SKMainWindow::GameInstallLocationOutput()
{
	if ( ui->comboBoxGame->currentText() == "Skyrim SE/AE" )
		sGamePath = sGamePathSkyrim;
	else if ( ui->comboBoxGame->currentText() == "Fallout 4" )
		sGamePath = sGamePathFallout4;
	if ( ui->comboBoxGame->currentText().size() )
		this->setWindowTitle( ui->comboBoxGame->currentText() + tr( " Downgrade Utility" ) );
	if ( !sGamePath.isEmpty() )
	{
		sGamePath.replace( '\\', '/' );
		ui->lineEditGamePath->setText( sGamePath );
		ui->textEdit->append( tr( "Game install location found at %1" ).arg( sGamePath ) );
	}
	else ui->textEdit->append( tr( "Game install location not found." ) );
	ui->lineEditDownloadPath->setText( startDir.absolutePath() + "/Download" );
}

//+------------------------------------------------------------------+
//| Get depotIDs and manifestIDs                                     |
//| INPUT: none                                                      |
//| OUTPUT: depotIDn, manifestIDn                                    |
//| REMARK: none                                                     |
//+------------------------------------------------------------------+
void SKMainWindow::GetDepotAndManifestIDs()
{
	qint32 cik, i, j;
	QString str, elementName, elementText;

	if ( pMainShared == NULL )
		return;
	if ( !pMainShared->sList.size() )
		return;
	if ( pXMLReader == NULL )
		return;
	for ( cik = 0; cik < pMainShared->sList.size(); cik++ )
	{
		str = pMainShared->sList.at( cik );
		if ( str.contains("<!--") )
			continue;
		if ( str.contains( "AppID", Qt::CaseInsensitive) )
		{
			elementText = pXMLReader->getsElementText( str );
			pMainShared->sAppID = elementText;
		}
		if ( str.contains( "AppName", Qt::CaseInsensitive) )
		{
			elementText = pXMLReader->getsElementText( str );
			pMainShared->sAppName = elementText;
		}
		if ( !str.contains("Version ID", Qt::CaseInsensitive) )
			continue;
		i = str.indexOf( "\"" );
		j = str.indexOf( "\"", i + 1 );
		if ( i == -1 || j == -1 )
			continue;
		i++;j--;
		if ( ui->comboBoxVersion->currentText() == str.mid( i, j - i + 1 ) )		// attrib/ID found
			break;
	}
	if ( cik == pMainShared->sList.size() )
	{
		ui->textEdit->append( tr("Error! Version ID '%1' not found.").arg(ui->comboBoxVersion->currentText()) );
		return;
	}
	for ( cik++; !str.contains("</Version>", Qt::CaseInsensitive); cik++ )
	{
		str = pMainShared->sList.at( cik );
		if ( str.contains("<!--") )
			continue;
		elementName = pXMLReader->getsElementID( str );
		elementText = pXMLReader->getsElementText( str );
		if ( elementName.contains( "Depot", Qt::CaseInsensitive ) )
			pMainShared->slDepotIDs.append( elementText );
		if ( elementName.contains( "Manifest", Qt::CaseInsensitive ) )
			pMainShared->slManifestIDs.append( elementText );
		if ( elementName.contains( "Delete", Qt::CaseInsensitive ) )
			pMainShared->slDeleteFiles.append( elementText );
	}
}

//+------------------------------------------------------------------+
//| User clicked on Exit menu                                        |
//| INPUT: none                                                      |
//| OUTPUT: none                                                     |
//| REMARK: none                                                     |
//+------------------------------------------------------------------+
void SKMainWindow::on_ExitMenuClicked()
{
	qint32 iRet;

	//ui->statusbar->showMessage("Exit menu clicked.");
	if ( pThreadControl != NULL )
		if ( pThreadControl->bIsWorkerRunning() )
		{
			msgBox.setWindowTitle(tr("Question"));
			msgBox.setText( tr("Work is in progress.") );
			msgBox.setInformativeText( tr( "Are you sure you want to interrupt?" ) );
			msgBox.setStandardButtons( QMessageBox::Ok | QMessageBox::Cancel );
			msgBox.setDefaultButton( QMessageBox::Cancel );
			iRet = msgBox.exec();
			if ( iRet == QMessageBox::Ok )
			{
				pThreadControl->sendWorkerInterruptSignal();
				return;											// let it finish
				//ui->textEdit->append( tr("Uh-oh.  Thread is still running!") );
			}
			if ( iRet == QMessageBox::Cancel )
				return;
		}
	if ( pProcessDL != NULL )
	{
		if ( pProcessDL->state() == QProcess::Running )
		{
			msgBox.setWindowTitle(tr("Early exit"));
			msgBox.setText(tr("Are you sure you want to quit?"));
			msgBox.setDetailedText( tr( "Download process is still running." ) );
			msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
			msgBox.setDefaultButton(QMessageBox::Cancel);
			iRet = msgBox.exec();
			if ( iRet == QMessageBox::Ok )
			{
				if ( pProcessDL->state() == QProcess::Running )
					pProcessDL->close();				// close() and kill()
					//pProcess->write( "0x03");		// Ctrl+C doesn't work
				return;
			}
			else return;
		}
	}
	if ( pProcessCopyFast != NULL )
	{
		if ( pProcessCopyFast->state() == QProcess::Running )
		{
			msgBox.setWindowTitle(tr("Early exit"));
			msgBox.setText(tr("Are you sure you want to quit?"));
			msgBox.setDetailedText( tr( "Copy process is still running." ) );
			msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
			msgBox.setDefaultButton(QMessageBox::Cancel);
			iRet = msgBox.exec();
			if ( iRet == QMessageBox::Ok )
			{
				if ( pProcessCopyFast->state() == QProcess::Running )
					pProcessCopyFast->close();				// close() and kill()
				return;
			}
			else return;
		}
	}
	if ( pSubwindow )
		pSubwindow->close();
	close();
}

void SKMainWindow::on_XMLReceiveStringBasic( QString s )
{
	ui->textEdit->append( s );
}

void SKMainWindow::on_OpenXMLMenuClicked()
{
	QString sFileName;
	QFile fileCheck;

	sFileName = QFileDialog::getOpenFileName(this, tr("Open XML file to load"), "", "*.xml");
	if ( sFileName.isEmpty() )
		return;
	fileCheck.setFileName(sFileName);
	if ( !fileCheck.exists() )
	{
		ui->textEdit->append( tr( "%1 does not exist.").arg(sFileName) );
		return;
	}
	if ( pMainShared != NULL )
		pXMLReader->ReadXMLSLTest(sFileName, pMainShared );		//, TESTMODE not needed because conditional QAction/menu item
}

//+------------------------------------------------------------------+
//| Version information changed in ComboBox                          |
//| INPUT: index of selected item                                    |
//| OUTPUT: none                                                     |
//| REMARK: none                                                     |
//+------------------------------------------------------------------+
void SKMainWindow::on_comboBoxVersionActivated(int index)
{
	if ( TESTMODE == 1 )
		ui->textEdit->append( tr( "Version information selected: %1, index: %2").arg( ui->comboBoxVersion->currentText() ).arg( index ) );
}

//+------------------------------------------------------------------+
//| Abort button clicked                                             |
//| INPUT: none                                                      |
//| OUTPUT: none                                                     |
//| REMARK: emergency stop process                                   |
//+------------------------------------------------------------------+
void SKMainWindow::on_pushButtonAbortClicked()
{
	qint32 iRet;
	bool bProcessRunning = false;

	if ( pThreadControl != NULL )
		if ( pThreadControl->bIsWorkerRunning() )
		{
			msgBox.setWindowTitle(tr("Question"));
			msgBox.setText( tr("Work is in progress.") );
			msgBox.setInformativeText( tr( "Are you sure you want to interrupt?" ) );
			msgBox.setStandardButtons( QMessageBox::Ok | QMessageBox::Cancel );
			msgBox.setDefaultButton( QMessageBox::Cancel );
			iRet = msgBox.exec();bProcessRunning = true;
			if ( iRet == QMessageBox::Ok )
			{
				pThreadControl->sendWorkerInterruptSignal();
				//ui->pushButtonAbort->setEnabled( false );
				return;
			}
			if ( iRet == QMessageBox::Cancel )
				return;
		}
	if ( pProcessDL != NULL )
		if ( pProcessDL->state() == QProcess::Running )
		{
			msgBox.setWindowTitle(tr("Early exit"));
			msgBox.setText(tr("Are you sure you want to quit?"));
			msgBox.setDetailedText( tr( "Download process is still running." ) );
			msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
			msgBox.setDefaultButton(QMessageBox::Cancel);
			iRet = msgBox.exec();bProcessRunning = true;
			if ( iRet == QMessageBox::Ok )
				pProcessDL->close();				// close() and kill()
		}
	if ( pProcessCopyFast != NULL )
		if ( pProcessCopyFast->state() == QProcess::Running )
		{
			msgBox.setWindowTitle(tr("Early exit"));
			msgBox.setText(tr("Are you sure you want to quit?"));
			msgBox.setDetailedText( tr( "Copy process is still running." ) );
			msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
			msgBox.setDefaultButton(QMessageBox::Cancel);
			iRet = msgBox.exec();bProcessRunning = true;
			if ( iRet == QMessageBox::Ok )
				pProcessCopyFast->close();				// close() and kill()
		}
	if ( !bProcessRunning )
		ui->pushButtonAbort->setEnabled( false );
}

//+------------------------------------------------------------------+
//| Browse button clicked (for game install location)                |
//| INPUT: none                                                      |
//| OUTPUT: path in sGamePath                                        |
//| REMARK: none                                                     |
//+------------------------------------------------------------------+
void SKMainWindow::on_pushButtonBrowseClicked()
{
	QString sFileName;
	qint32 i;

	if ( pThreadControl != NULL )
		if ( pThreadControl->bIsWorkerRunning() )
			return;
	if ( !sGamePath.isEmpty() )
		QDir::setCurrent( sGamePath );
	if ( ui->comboBoxGame->currentText() == "Skyrim SE/AE" )
		sFileName = QFileDialog::getOpenFileName(this, tr("Open Skyrim folder"), "", "SkyrimSE.exe");
	else if ( ui->comboBoxGame->currentText() == "Fallout 4" )
		sFileName = QFileDialog::getOpenFileName(this, tr("Open Fallout 4 folder"), "", "Fallout4.exe");
	else sFileName = QFileDialog::getOpenFileName(this, tr("Open game folder"), "", "*.exe");
	if ( sFileName.isEmpty() )
		return;
	if ( !sFileName.endsWith('/') )
	{
		i = sFileName.lastIndexOf( "/" );
		if ( i < 0 )
		{
			ui->textEdit->append( tr( "Something is wrong with the selected path." ) );
			return;
		}
		sFileName.truncate( i + 1 );
	}
	sGamePath = sFileName;
	ui->lineEditGamePath->setText( sGamePath );
}

//+------------------------------------------------------------------+
//| Browse button clicked (for download location)                    |
//| INPUT: none                                                      |
//| OUTPUT: none                                                     |
//| REMARK: none                                                     |
//+------------------------------------------------------------------+
void SKMainWindow::on_pushButtonBrowse2Clicked()
{
	QString sDLFolderName;

	if ( pThreadControl != NULL )
		if ( pThreadControl->bIsWorkerRunning() )
			return;
	sDLFolderName = QFileDialog::getExistingDirectory( this, tr( "Open download folder" ), "", QFileDialog::ShowDirsOnly );
	if ( sDLFolderName.isEmpty() )
		return;
	ui->lineEditDownloadPath->setText( sDLFolderName );
}

//+------------------------------------------------------------------+
//| Start downgrade                                                  |
//| INPUT: index of selected item                                    |
//| OUTPUT: depotIDn, manifestIDn                                    |
//| REMARK: none                                                     |
//+------------------------------------------------------------------+
void SKMainWindow::on_pushButtonDownloadClicked()
{
	QString sProcessCommand;
	QFileInfoList fiList;				// synonym for QList<QFileInfo>
	QStringList filters, slArguments;

	if ( pMainShared == NULL )
	{
		ui->textEdit->append( tr( "Critical error!  Shared data pointer problem, program won't work!" ) );
		return;
	}
	if ( pMainShared->iState )
		return;
	if ( pThreadControl != NULL )
		if ( pThreadControl->bIsWorkerRunning() )
			return;
	if ( ui->comboBoxGame->currentText().isEmpty() )
	{
		ui->textEdit->append( tr( "You have to select a game to downgrade." ) );
		return;
	}
	if ( ui->comboBoxVersion->currentText().isEmpty() )
	{
		ui->textEdit->append( tr( "You have to select a game version to downgrade." ) );
		return;
	}
	if ( ui->lineEditGamePath->text().isEmpty() )
	{
		ui->textEdit->append( tr( "Game path is empty, can't downgrade %1." ).arg( pMainShared->sAppName ) );
		return;
	}
	if ( pProcessDL == NULL )
	{
		ui->textEdit->append( tr( "Critical error!  Can't start new process - can't download from Steam." ) );
		return;
	}
	if ( ui->lineEditUser->text().isEmpty() )
	{
		ui->textEdit->append( tr( "Username or password missing!") );
		return;
	}
	if ( ui->lineEditPW->text().isEmpty() )
	{
		ui->textEdit->append( tr( "Username or password missing!") );
		return;
	}
	this->ResetSharedStruct();
	this->GetDepotAndManifestIDs();
	if ( pMainShared->slDepotIDs.isEmpty() || pMainShared->slManifestIDs.isEmpty() )
	{
		ui->textEdit->append( tr( "Error: Depot and/or manifest ID list is empty!") );
		return;
	}
	ui->textEdit->append( tr("Target directory for downloaded files: %1\n").arg( ui->lineEditDownloadPath->text()) );
	QDir dirDownload( ui->lineEditDownloadPath->text() );
	if ( !dirDownload.exists() )
		dirDownload.mkdir( dirDownload.absolutePath() );
	else
	{
		filters << "*";		// "*.*" doesn't work?!
		fiList = dirDownload.entryInfoList( filters, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot );
		if ( !fiList.isEmpty() )
		{
			if ( TESTMODE == 3 )
			{
				this->DisableControls();
				this->FinalizeDowngrade();
				//this->FinalizeDowngrade2();
				return;
			}
			msgBox.setWindowTitle(tr("Warning!  Download directory is not empty!"));
			msgBox.setText( tr( "Would you like to delete the contents of directory %1?" ).arg( dirDownload.absolutePath() ) );
			msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
			msgBox.setDefaultButton(QMessageBox::Cancel);
			int ret = msgBox.exec();
			if ( ret == QMessageBox::Ok )
			{
				if ( !dirDownload.removeRecursively() )
				{
					ui->textEdit->append( tr( "Something went wrong.  Cannot delete directory %1. ").arg( dirDownload.absolutePath() ) );
					return;									// https://stackoverflow.com/questions/27758573/deleting-a-folder-and-all-its-contents-with-qt is better
				}
				if ( !dirDownload.exists() )
					dirDownload.mkdir( dirDownload.absolutePath() );
				ui->textEdit->append( tr("Directory %1 is now empty.").arg( dirDownload.absolutePath() ) );
			}
			if ( ret == QMessageBox::Cancel )
			{
				ui->textEdit->append( tr("Download canceled.") );
				return;
			}
		}
	}
#if OSTYPE == OSWINDOWS
	sProcessCommand = startDir.absolutePath() + "/DepotDownloader/DepotDownloader.exe";
#elif OSTYPE == OSLINUX
	sProcessCommand = startDir.absolutePath() + "/DepotDownloader/DepotDownloader";
#endif
	if ( TESTMODE == 1 )
		qDebug() << "Downloader path: " << sProcessCommand;
	if ( !QFile::exists( sProcessCommand ) )
	{
		ui->textEdit->append( tr("Can't find DepotDownloader at %1!").arg( sProcessCommand ) );
		return;
	}
	ui->textEdit->append("Starting DepotDownloader...\n");
	slArguments.clear();

	//sProcessCommand = "ls";					// thanks to https://forum.qt.io/topic/113592/run-command-line-from-qt-app-in-linux/5 grullo wrote at 2020. ápr. 13. 6:55
	//slArguments << "-l" << "~";
	slArguments = this->slDLParamConstruct( pMainShared->iState++ );
	pProcessDL->start( sProcessCommand, slArguments );

}

//+------------------------------------------------------------------+
//| Testing a subwindow                                              |
//| INPUT: none                                                      |
//| OUTPUT: subwindow with buttons and text edit field               |
//| REMARK: none                                                     |
//+------------------------------------------------------------------+
void SKMainWindow::on_pushButtonSubwinClicked()
{
	if ( pSubwindow == NULL )
	{
		pSubwindow = new Subwindow();
		if ( pSubwindow != NULL )
		{
			connect( pSubwindow, &Subwindow::SubwinSendTextSignal, this, &SKMainWindow::on_subwinOKButtonClicked);
			connect( pSubwindow, &Subwindow::SubwinCancelSignal, this, &SKMainWindow::on_subwinCancelButtonClicked );
			//qDebug() << "Subwindow exists now.";
		}
	}
	if ( pSubwindow != NULL )
		pSubwindow->show();
	/*pMdiArea = new QMdiArea();						// it works but destroys UI
	*/
}

//+------------------------------------------------------------------+
//| OK button clicked at 2FA authenticaton                           |
//| INPUT: 2FA code as QString                                       |
//| OUTPUT: message                                                  |
//| REMARK: none                                                     |
//+------------------------------------------------------------------+
void SKMainWindow::on_subwinOKButtonClicked( QString s )
{
	if ( s.isEmpty() )
	{
		ui->statusbar->showMessage( tr("2FA code is missing!"), 2000 );
		ui->textEdit->append( tr("2FA code is missing!") );
		if ( pProcessDL->state() == QProcess::Running )
			pProcessDL->close();				// close() and kill()
		return;
	}
	if ( TESTMODE )
		ui->statusbar->showMessage( tr( "2FA code is %1.").arg(s), 2000 );
	if ( pProcessDL->state() == QProcess::Running )
	{
		s.append("\n");
		pProcessDL->write( s.toLatin1().constData() );
	}
}

//+------------------------------------------------------------------+
//| Cancel button clicked at 2FA authenticaton                       |
//| INPUT: none                                                      |
//| OUTPUT: message                                                  |
//| REMARK: none                                                     |
//+------------------------------------------------------------------+
void SKMainWindow::on_subwinCancelButtonClicked()
{
	ui->statusbar->showMessage( tr("2FA canceled."), 1000 );
	if ( pProcessDL->state() == QProcess::Running )
		pProcessDL->close();				// close() and kill()
	/*if ( TESTMODE )
		if ( pMainShared != NULL )
		{
			qDebug() << "MainShared::iState is " << pMainShared->iState++;			// 0
			this->CopyFastPreparation();
		}*/
}

//+------------------------------------------------------------------+
//| ComboBox text changed, update data                               |
//| INPUT: none                                                      |
//| OUTPUT: yes                                                      |
//| REMARK: none                                                     |
//+------------------------------------------------------------------+
void SKMainWindow::on_comboBoxGameCurrentTextChanged(const QString &arg1)
{
	QString s = arg1;							// just to stop complaining about unused parameter

	this->SetGameDefinitions();
	this->GameInstallLocationOutput();s.clear();
}

//+------------------------------------------------------------------+
//| Process has STD output                                           |
//| INPUT: none                                                      |
//| OUTPUT: data as byte array into text edit field                  |
//| REMARK: none                                                     |
//+------------------------------------------------------------------+
void SKMainWindow::on_ReadStdOutputDL()
{
	QByteArray baData;
	QString sProcessOutput;

	baData.clear();
	if ( pProcessDL != NULL )
		baData = pProcessDL->readAllStandardOutput();
	if ( !baData.size() )
		return;
	if ( baData.endsWith( '\n' ) )			// or removeLast()
		baData.chop( 1 );
	if ( baData.endsWith( '\r' ) )
		baData.chop( 1 );
	sProcessOutput = baData.constData();
	if ( sProcessOutput.contains( "Logging") && sProcessOutput.contains( "into Steam") )
	{
		ui->textEdit->append( tr( "If STEAM GUARD is activated on your account (it should be) use the Steam Mobile App to confirm your sign in...\n" ) );
		//ui->textEdit->append( tr( "Otherwise report the error and hope that I can fix it.") );		// subwin - 2FA code?
	}
	if ( sProcessOutput.contains( "2FA") )
		this->on_pushButtonSubwinClicked();
	if ( sProcessOutput.contains( "username", Qt::CaseInsensitive ) && sProcessOutput.contains( "password", Qt::CaseInsensitive ) )
		return;
	ui->textEdit->append( baData.data() );
	ui->textEdit->verticalScrollBar()->setValue( ui->textEdit->verticalScrollBar()->maximum() );
}

//+------------------------------------------------------------------+
//| Process has STD output                                           |
//| INPUT: none                                                      |
//| OUTPUT: data as byte array into text edit field                  |
//| REMARK: had to separate them to avoid "device not open" warnings.|
//+------------------------------------------------------------------+
void SKMainWindow::on_ReadStdOutputCopyFast()
{
	QByteArray baData;

	baData.clear();
	if ( pProcessCopyFast != NULL )
	{
		baData = pProcessCopyFast->readAllStandardOutput();
		//if ( baData.size() )
			//qDebug() << "'Copy fast' process has standard output.";
	}
	if ( !baData.size() )
		return;
	if ( baData.endsWith( '\n' ) )			// or removeLast()
		baData.chop( 1 );
	if ( baData.endsWith( '\r' ) )
		baData.chop( 1 );
	ui->textEdit->append( baData.data() );
	ui->textEdit->verticalScrollBar()->setValue( ui->textEdit->verticalScrollBar()->maximum() );
}

//+------------------------------------------------------------------+
//| Process finished, should we continue?                            |
//| INPUT: none                                                      |
//| OUTPUT: message box in case sg went wrong                        |
//| REMARK: none                                                     |
//+------------------------------------------------------------------+
void SKMainWindow::on_processDLFinished(int exitCode, QProcess::ExitStatus exitState)
{
	QString sProcess;
	QStringList slArguments;
	int iRet;

	sProcess = tr( "Process exited with code %1." ).arg(exitCode);		//+ QString::number(exitCode)
	if ( exitState == QProcess::CrashExit )
		sProcess += "  Process crashed.";
	if ( exitCode != 0 )
	{
		ui->textEdit->append( sProcess );
		msgBox.setText( tr("Something went wrong.") );
		msgBox.setDetailedText( tr("There has been a problem.  Process exited with error code %1.").arg(exitCode) );
		msgBox.setWindowTitle( tr("Warning") );
		msgBox.setStandardButtons( QMessageBox::Ok );		//| QMessageBox::Cancel
		msgBox.setDefaultButton( QMessageBox::Ok );
		iRet = msgBox.exec();
		if ( iRet == QMessageBox::Ok )
		{
			this->EnableControls();
			return;
		}
	}
	else if ( pProcessDL != NULL )
	{
		if ( pMainShared->iState < pMainShared->slDepotIDs.size() )
		{
			//qDebug() << "current iteration: " << pMainShared->slDepotIDs[pMainShared->iState];
#if OSTYPE == OSWINDOWS
			sProcess = startDir.absolutePath() + "/DepotDownloader/DepotDownloader.exe";				// start process
#elif OSTYPE == OSLINUX
			sProcess = startDir.absolutePath() + "/DepotDownloader/DepotDownloader";				// start process
#endif
			slArguments.clear();
			slArguments = this->slDLParamConstruct( pMainShared->iState++ );
			pProcessDL->start( sProcess, slArguments );
			return;
		}
		//if ( pMainShared->iState > pMainShared->slDepotIDs.size() )				// bug :)
			//ui->textEdit->append( tr( "Process finished." ) );
		if ( pMainShared->iState == pMainShared->slDepotIDs.size() )
			this->FinalizeDowngrade();
	}
}

//+------------------------------------------------------------------+
//| Process started                                                  |
//| INPUT: none                                                      |
//| OUTPUT: none                                                     |
//| REMARK: none                                                     |
//+------------------------------------------------------------------+
void SKMainWindow::on_processStarted()
{
	if ( pMainShared != NULL )
		if ( pMainShared->iState == 1 )
			this->DisableControls();
}

//+------------------------------------------------------------------+
//| Process finished, should we continue?                            |
//| INPUT: none                                                      |
//| OUTPUT: message box in case sg went wrong                        |
//| REMARK: none                                                     |
//+------------------------------------------------------------------+
void SKMainWindow::on_processCopyFastFinished(int exitCode, QProcess::ExitStatus exitState)
{
	QString sProcess;
	QStringList slArguments;
	int iRet;

	sProcess = tr( "Process exited with code %1." ).arg(exitCode);		//+ QString::number(exitCode)
	if ( exitState == QProcess::CrashExit )
		sProcess += "  Process crashed.";
	if ( exitCode != 0 )
	{
		ui->textEdit->append( sProcess );
		msgBox.setText( tr("Something went wrong.") );		//Do you wish to continue?
		msgBox.setDetailedText( tr("There has been a problem.  Process exited with error code %1.").arg(exitCode) );
		msgBox.setWindowTitle( tr("Warning") );
		msgBox.setStandardButtons( QMessageBox::Ok );		//| QMessageBox::Cancel
		msgBox.setDefaultButton( QMessageBox::Ok );
		iRet = msgBox.exec();
		if ( iRet == QMessageBox::Ok )
		{
			this->EnableControls();
			return;
		}
	}
	this->FinalizeDowngrade2();
}

//+------------------------------------------------------------------+
//| Reset shared data structure                                      |
//| INPUT: none                                                      |
//| OUTPUT: yes                                                      |
//| REMARK: none                                                     |
//+------------------------------------------------------------------+
void SKMainWindow::ResetSharedStruct()
{
	if ( pMainShared == NULL )
		return;
	pMainShared->iState = 0;pMainShared->slDepotIDs.clear();pMainShared->sAppID.clear();pMainShared->bInterrupt = false;
	pMainShared->slManifestIDs.clear();pMainShared->slDeleteFiles.clear();pMainShared->sAppName.clear();//pMainShared->sList.clear();	// allowed only in XMLReader::ReadXMLSL()!
}

//+------------------------------------------------------------------+
//| Set game directory and settings                                  |
//| INPUT: none                                                      |
//| OUTPUT: none                                                     |
//| REMARK: none                                                     |
//+------------------------------------------------------------------+
void SKMainWindow::SetGameDefinitions()
{
	QString str, sSearchFor, sGameNameEmpty = "none";
	qint32 i;
	bool bFound = false;

	ui->lineEditGamePath->clear();sGamePath.clear();
	if ( pXMLReader != NULL && pMainShared != NULL )
	{
		str = ui->comboBoxGame->currentText();
		i = str.indexOf( ' ' );
		if ( i > 1 )
			str = str.first( i );
		//qDebug() << "SetGameDefinitions fileName substring == " << str;
		sSearchFor = "/steam_manifests_" + str;
		for ( i = 0; i < pMainShared->slDefinitionFiles.size(); i++ )
			if ( pMainShared->slDefinitionFiles.at( i ).contains( sSearchFor, Qt::CaseInsensitive ) )
			{
				str = pMainShared->slDefinitionFiles.at( i );bFound = true;
				break;
			}
		if ( !bFound && !TESTMODE )
		{
			if ( ui->comboBoxGame->currentText().isEmpty() )
				ui->textEdit->append( tr( "Error!  No match found for selected game '%1' in game definition files!").arg( sGameNameEmpty ) );
			else ui->textEdit->append( tr( "Error!  No match found for selected game '%1' in game definition files!").arg( ui->comboBoxGame->currentText() ) );
			return;
		}
		pXMLReader->ReadXMLSL( str, pMainShared, true );
		SetVersionCombobox();
	}
}

//+------------------------------------------------------------------+
//| Fill version information into ComboBox                           |
//| INPUT: pMainShared.sList                                         |
//| OUTPUT: data to ui->ComboBoxVersion                              |
//| REMARK: none                                                     |
//+------------------------------------------------------------------+
void SKMainWindow::SetVersionCombobox()
{
	qint32 cik, i, j;
	QString sAttribName, str;

	if ( pMainShared == NULL )
		return;
	if ( !pMainShared->sList.size() )
		return;
	ui->comboBoxVersion->clear();
	for ( cik = 0; cik < pMainShared->sList.size(); cik++ )				// getAttribute()
	{
		str = pMainShared->sList.at( cik );
		if ( str.contains("<!--") )
			continue;
		if ( !str.contains("Version ID", Qt::CaseInsensitive) )
			continue;
		i = str.indexOf( "\"" );
		j = str.indexOf( "\"", i + 1 );
		if ( i == -1 || j == -1 )
			continue;
		i++;j--;
		sAttribName = str.mid( i, j - i + 1 );
		if ( TESTMODE == 1 )
			ui->textEdit->append( tr( "Attrib name: ") + sAttribName );
		ui->comboBoxVersion->addItem( sAttribName );
	}
}

//+------------------------------------------------------------------+
//| Set combobox list                                                |
//| INPUT: none                                                      |
//| OUTPUT: ui->comboboxGame items, game definitions directory       |
//| REMARK: run only at startup!                                     |
//+------------------------------------------------------------------+
void SKMainWindow::PrefetchAppName()
{
	QStringList slFilters;
	QFileInfoList fiList;
	qint32 i, j;
	QString str, elementText;
	bool bFound;

	if ( pXMLReader == NULL )
		return;
	if ( pMainShared == NULL )
		return;
	sDefXMLDir = startDir.absolutePath() + "/GameDefinitions";pMainShared->slDefinitionFiles.clear();
	QDir dirPrefetch( sDefXMLDir );
	slFilters << "*.xml";		// "*.*" does work!
	fiList = dirPrefetch.entryInfoList( slFilters, QDir::Files );
	if ( fiList.isEmpty() )
	{
		//qDebug() << "fiList is empty.";
		ui->textEdit->append( tr( "ERROR: Cannot prefetch app name(s) from %1.\n").arg(sDefXMLDir) );
		return;
	}
	for ( i = 0; i < fiList.size(); i++ )				// https://stackoverflow.com/questions/27758573/deleting-a-folder-and-all-its-contents-with-qt is better
	{
		//qDebug() << "dirPrefetch entry: " << fiList.at(i).absoluteFilePath();
		pXMLReader->ReadXMLSL( fiList.at(i).absoluteFilePath(), pMainShared, false );bFound = false;
		if ( !pMainShared->sList.size() )
		{
			ui->textEdit->append( tr( "ERROR: Cannot prefetch app name from %1.\n" ).arg( fiList.at(i).absoluteFilePath() ) );
			continue;
		}
		for ( j = 0; j < pMainShared->sList.size(); j++ )
		{
			str = pMainShared->sList.at( j );
			if ( str.contains( "AppName", Qt::CaseInsensitive) )
			{
				elementText = pXMLReader->getsElementText( str );bFound = true;
				ui->comboBoxGame->addItem( elementText );
				pMainShared->slDefinitionFiles.append( fiList.at(i).absoluteFilePath() );
				break;
			}
		}
		if ( !bFound )
			ui->textEdit->append( tr( "ERROR: Cannot prefetch app name from %1.\n" ).arg( fiList.at(i).absoluteFilePath() ) );
	}
	pMainShared->sList.clear();
}

//+------------------------------------------------------------------+
//| Construct parameter string list for DL process                   |
//| INPUT: phase (iState)                                            |
//| OUTPUT: argument string list                                     |
//| REMARK: none                                                     |
//+------------------------------------------------------------------+
QStringList SKMainWindow::slDLParamConstruct( int phase )
{
	QStringList slArguments;

	slArguments.clear();
	if ( pMainShared == NULL )				// should not be possible
		return slArguments;
	slArguments.append( "-app");
	slArguments.append( pMainShared->sAppID );
	if ( TESTMODE )
	{
		slArguments << "-depot" << "489833";									//TEST
		slArguments << "-manifest" << "2442187225363891157";					//TEST
		pMainShared->slDepotIDs.clear();
	}
	else
	{
		slArguments << "-depot" << pMainShared->slDepotIDs.at( phase );
		slArguments << "-manifest" << pMainShared->slManifestIDs.at( phase );
	}
	slArguments << "-username" << ui->lineEditUser->text();
	slArguments << "-password" << ui->lineEditPW->text();
	slArguments << "-remember-password";
	slArguments << "-dir" << ui->lineEditDownloadPath->text();
	return slArguments;
}

//+------------------------------------------------------------------+
//| Downgrade cleanup, finishing moves                               |
//| INPUT: none                                                      |
//| OUTPUT: none                                                     |
//| REMARK: none                                                     |
//+------------------------------------------------------------------+
void SKMainWindow::FinalizeDowngrade()
{
	//qDebug() << "FinalizeDowngrade started.";
	if ( pThreadControl == NULL )
	{
		ui->textEdit->append( tr( "Warning!  ThreadControl pointer is NULL!  Copying files the old way (program will become unresponsive for a while)." ) );
		this->CopyFiles();
		this->DeleteFiles();
		ui->textEdit->append( tr( "Downgrading finished.") );
		return;
	}
	pMainShared->iState++;
	//emit pThreadControl->sendWorkerStartSignal( ui->lineEditDownloadPath->text(), ui->lineEditGamePath->text() );
	this->CopyFastPreparation();
}

//+------------------------------------------------------------------+
//| Downgrade cleanup, finishing moves part 2                        |
//| INPUT: none                                                      |
//| OUTPUT: none                                                     |
//| REMARK: none                                                     |
//+------------------------------------------------------------------+
void SKMainWindow::FinalizeDowngrade2()
{
	if ( pThreadControl == NULL )				// already finished in this case
		return;
	this->DeleteFiles();
	this->EnableControls();pMainShared->iState = 0;
	ui->textEdit->append( tr( "Downgrading finished.") );
}

//+------------------------------------------------------------------+
//| Disable controls when worker process starts                      |
//| INPUT: none                                                      |
//| OUTPUT: none                                                     |
//| REMARK: none                                                     |
//+------------------------------------------------------------------+
void SKMainWindow::DisableControls()
{
	ui->pushButtonAbort->setEnabled( true );
	ui->comboBoxGame->setEnabled( false );ui->comboBoxVersion->setEnabled( false );
	ui->pushButtonBrowse->setEnabled( false );ui->pushButtonBrowse2->setEnabled( false );
}

//+------------------------------------------------------------------+
//| Enable controls after worker process is finished                 |
//| INPUT: none                                                      |
//| OUTPUT: none                                                     |
//| REMARK: none                                                     |
//+------------------------------------------------------------------+
void SKMainWindow::EnableControls()
{
	ui->comboBoxGame->setEnabled( true );ui->comboBoxVersion->setEnabled( true );
	ui->pushButtonBrowse->setEnabled( true );ui->pushButtonBrowse2->setEnabled( true );
}

//+------------------------------------------------------------------+
//| Prepare file copying (fast method)                               |
//| INPUT: download path (source), game path (dest)                  |
//| OUTPUT: none                                                     |
//| REMARK: separate process, no worker thread                       |
//+------------------------------------------------------------------+
void SKMainWindow::CopyFastPreparation()
{
	QStringList slArguments;
	//qint32 i, j;
	QString sFileNameSource, sFileNameTarget, sProcessCommand;

	if ( pMainShared == NULL )
	{
		ui->textEdit->append( tr( "Critical ERROR!  Shared data pointer problem.  Fast copy would work but download didn't so..." ) );
		return;
	}
	ui->textEdit->append( tr( "\nCopying files..." ) );pMainShared->iState = 0;
#if OSTYPE == OSLINUX
	sProcessCommand = "rsync";
#elif OSTYPE == OSWINDOWS
	sProcessCommand = "robocopy";
#endif
	slArguments = this->slCopyFastParamConstruct( pMainShared->iState++ );
	pProcessCopyFast->start( sProcessCommand, slArguments );
}

//+------------------------------------------------------------------+
//| Construct parameter string list for fast copy process            |
//| INPUT: phase (iState)                                            |
//| OUTPUT: argument string list                                     |
//| REMARK: none                                                     |
//+------------------------------------------------------------------+
QStringList SKMainWindow::slCopyFastParamConstruct( int iPhase )
{
	QStringList slArguments;
	QString sSourceDir, sTargetDir;

	slArguments.clear();
	if ( !iPhase )							// unnecessary but prevents warning (int paramenter is necessary for on_processStarted() )
		;
	if ( pMainShared == NULL )				// should not be possible
		return slArguments;
#if OSTYPE == OSLINUX
	slArguments.append( "-av");
	slArguments.append( "-r" );
	slArguments << "--exclude=.DepotDownloader/*";
	sSourceDir = ui->lineEditDownloadPath->text();
	if ( !sSourceDir.endsWith( '/' ) )
		sSourceDir.append( '/' );
	sTargetDir = ui->lineEditGamePath->text();
	if ( !sTargetDir.endsWith( '/' ) )
		sTargetDir.append( '/' );
	slArguments << sSourceDir << sTargetDir;
#elif OSTYPE == OSWINDOWS
	sSourceDir = ui->lineEditDownloadPath->text();
	sTargetDir = ui->lineEditGamePath->text();
	slArguments << sSourceDir << sTargetDir;
	slArguments << "/s" << "/mt" << "/j";
	slArguments.append( "/xd .DepotDownloader" );
#endif
	return slArguments;
}
