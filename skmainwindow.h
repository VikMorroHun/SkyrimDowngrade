#ifndef SKMAINWINDOW_H
#define SKMAINWINDOW_H
#define TESTMODE 0			// 0 - release, 1 - test mode, 2 - public beta test, 3 - copy files only
#define OSLINUX 1
#define OSWINDOWS 2
#define OSTYPE OSLINUX

#include "XMLReader.h"
#include "subwindow.h"
#include "workerthread.h"
#include <QMainWindow>
#include <QMenuBar>
#include <QAction>
#include <QProcess>
#include <QDir>
#include <QSettings>
#include <QScrollBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QMdiSubWindow>
#include <QPointer>

QT_BEGIN_NAMESPACE
namespace Ui {
class SKMainWindow;
}
QT_END_NAMESPACE

class SKMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	SKMainWindow(QWidget *parent = nullptr);
	~SKMainWindow();
	void DeleteFiles();
	void CopyFiles();
	XMLReader * pXMLReader;
	strucShared * pMainShared;
	Controller * pThreadControl;

private:
	Ui::SKMainWindow *ui;
	QMenu * fileMenu;
	QMenuBar * menuBar;
	QAction * exitAction;		// Esc key press?
	QAction * browseActionGame, *browseActionDownload;
	QAction * openXMLAction;
	QProcess * pProcessDL;
	QString sDefXMLDir, sGamePath, sGamePathSkyrim, sGamePathFallout4;
	QSettings WindowsRegSkyrimSE, WindowsRegFallout4;
	QMessageBox msgBox;
	QDir startDir;
	QPointer <Subwindow> pSubwindow;
	bool bAbortClickedOnce;
	void SetVersionCombobox();
	void GetDepotAndManifestIDs();
	void ResetDepotManifestIDs();
	void SetGameDefinitions();
	void GameInstallLocationOutput();
	void PrefetchAppName();
	QString sDLParamConstruct( int );
	QStringList slDLParamConstruct( int );
	void FinalizeDowngrade();
	void FinalizeDowngrade2();

private slots:
	void on_ExitMenuClicked();

	void on_XMLReceiveStringBasic( QString );

	void on_OpenXMLMenuClicked();

	void on_comboBoxVersionActivated(int index);

	void on_pushButtonAbortClicked();

	void on_pushButtonBrowseClicked();

	void on_pushButtonDownloadClicked();

	void on_comboBoxGameCurrentTextChanged(const QString &arg1);

	void on_readyReadStd();

	void on_processFinished(int, QProcess::ExitStatus);

	void on_pushButtonBrowse2Clicked();

	void on_pushButtonSubwinClicked();

	void on_subwinOKButtonClicked( QString );

	void on_subwinCancelButtonClicked();

	void on_processStarted();

signals:
	void StartSubwinSignal();
};
#endif // SKMAINWINDOW_H
