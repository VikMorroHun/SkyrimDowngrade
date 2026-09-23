#ifndef XMLREADER_H
#define XMLREADER_H

#include <QString>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QStringList>
#include <QTextStream>
#include <QMap>

struct strucShared
{
	int iState;
	QStringList sList;
	QStringList slDepotIDs, slManifestIDs, slDeleteFiles;
	QStringList slDefinitionFiles;
	QString sAppID, sAppName;
	bool bInterrupt;
};

class XMLReader : public QObject
{
	Q_OBJECT
public:
	XMLReader();
	~XMLReader();
	void ReadXMLSLTest(QString, strucShared * );
	void ReadXMLSL(QString, strucShared *, bool );
	void TestMsgSender();
	QString getsElementID( QString );
	QString getsElementText( QString );

private:
	QString str;
	QByteArray baData;
	QTextStream *pTsXML, *pTsEngXML, *pTsTrXML;
	QString getElementName(int index, strucShared *pShared);
	bool getbIsEmptyElement(QString * );
	//QString getsUUIDEng( QString );		-> getsAttribName
	QString getsTextEng( QString );

signals:
	void XMLSendStringBasicSignal( QString );
	//void XMLStringPointerSender( QString * );		// can produce garbage/lose data

private slots:

};
#endif // XMLREADER_H
