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
#include <XMLReader.h>
#include <QDebug>
#include <QObject>

/****************************************************************************************************
 * An Element is something that does or could have attributes of its own or contain other elements. *
 * An Attribute is something that is self-contained, i.e., a color, an ID, a name.                  *
 * An element can contain: text, attributes, other elements, or a mix of the above.                 *
 * Metadata (data about data) should be stored as attributes, and the data itself should be stored  *
 * as elements.  https://www.w3schools.com/xml/xml_attributes.asp                                   *
 ****************************************************************************************************/

XMLReader::XMLReader()
{
	/*if ( document == NULL )
	{
		str = "XMLReader constructor without document pointer.  Exiting.";
		//emit XMLSendStringBasic( str );		// emit inside constructor has no effect
		return;
	}	*/
	//pWorkerShared = &Shared;
}

// https://stackoverflow.com/questions/49188597/how-to-parse-an-unknown-xml-in-qt-and-get-all-the-attributes-in-it

XMLReader::~XMLReader()
{

}

//+------------------------------------------------------------------+
//| Read XML file into QStringList - test mode                       |
//| INPUT: fileName, shared data pointer                             |
//| OUTPUT: QStringList                                              |
//| REMARK: none                                                     |
//+------------------------------------------------------------------+
void XMLReader::ReadXMLSLTest( QString fileName, strucShared * pShared )
{
	qint32 i;
	QString elementName, elementText, strIN, temp;
	QFile file(fileName);
	//QMessageBox msgBox;

	if ( !file.open( QIODevice::ReadOnly ) )
	{
		str = QObject::tr("Opening file %1 failed.").arg(fileName);
		emit XMLSendStringBasicSignal( str );
		//msgBox.setText( QObject::tr("Opening file %1 failed.").arg(fileName) );
		//msgBox.exec();
		return;
	}
	str = "ReadXMLSL test - file opened.  Element names:\n";
	emit XMLSendStringBasicSignal( str );
	pShared->sList.clear();str.clear();
	pTsXML = new QTextStream( &file );
	while ( !pTsXML->atEnd() )
	{
		strIN = pTsXML->readLine();//strIN = strIN.trimmed();		// breaks multiple line string
		if ( strIN.endsWith( ">" ) )
		{
			str += strIN;
			pShared->sList.append( str );str.clear();
		}
		else
		{
			str += strIN;str += "\n";
			continue;
		}
	}
	file.close();delete pTsXML;
	if ( pShared->sList.size() > 0 )
	{
		for ( i = 0; i < pShared->sList.size(); i++ )
		{
			temp = pShared->sList.at( i );
			elementName = this->getsElementID( temp );
			elementText = this->getsElementText( temp );
			if ( elementName.size() && elementText.size() )		// elementName != "", elementText == "" -> node
			{
				str = "element name: " + elementName + ", element text: " + elementText;
				emit XMLSendStringBasicSignal( str );		//pShared->sList.at(i)
			}
		}
	}
}

//+------------------------------------------------------------------+
//| Read XML file into QStringList                                   |
//| INPUT: fileName, shared data pointer, emit string?               |
//| OUTPUT: QStringList                                              |
//| REMARK: none                                                     |
//+------------------------------------------------------------------+
void XMLReader::ReadXMLSL( QString fileName, strucShared * pShared, bool bEchoMode = true )
{
	QString strIN;
	QFile file(fileName);

	pShared->sList.clear();
	if ( !file.open( QIODevice::ReadOnly ) )
	{
		str = QObject::tr("Opening file %1 failed.").arg(fileName);
		emit XMLSendStringBasicSignal( str );
		return;
	}
	str.clear();
	pTsXML = new QTextStream( &file );
	while ( !pTsXML->atEnd() )
	{
		strIN = pTsXML->readLine();//strIN = strIN.trimmed();		// breaks multiple line string
		if ( strIN.endsWith( ">" ) )
		{
			str += strIN;
			pShared->sList.append( str );str.clear();
		}
		else
		{
			str += strIN;str += "\n";
			continue;
		}
	}
	file.close();delete pTsXML;
	str = QObject::tr("File containing Steam depot information %1 has been loaded.").arg(fileName);
	if ( bEchoMode )
		emit XMLSendStringBasicSignal( str );
}

void XMLReader::TestMsgSender()
{
	emit XMLSendStringBasicSignal( "KUKUCS" );
}

//+------------------------------------------------------------------+
//| Get element ID                                                   |
//| INPUT: string                                                    |
//| OUTPUT: element ID, or empty string if error occured             |
//| REMARK: none                                                     |
//+------------------------------------------------------------------+
QString XMLReader::getsElementID( QString s )
{
	qint32 i, j;

	i = s.indexOf( "<" );
	j = s.indexOf( ">", i + 1 );
	if ( i == -1 || j == -1 )
		return "";
	s.truncate( j );
	i = s.indexOf( "<" );
	s.remove( 0, i + 1 );
	if ( s.endsWith("/") )     // empty element
		s.truncate( s.size() - 1 );
	return s;
}

//+------------------------------------------------------------------+
//| Check if empty element                                           |
//| INPUT: string                                                    |
//| OUTPUT: true - element is empty, false - element contains data   |
//| REMARK: none                                                     |
//+------------------------------------------------------------------+
bool XMLReader::getbIsEmptyElement( QString * s )
{
	bool bRet = false;

	if ( s->endsWith("/>") )
		bRet = true;
	return bRet;
}

//+------------------------------------------------------------------+
//| Get element text                                                 |
//| INPUT: string                                                    |
//| OUTPUT: element text (or empty string if empty)                  |
//| REMARK: none                                                     |
//+------------------------------------------------------------------+
QString XMLReader::getsElementText( QString s )
{
	qint32 i, j;

	if ( this->getbIsEmptyElement( &s ) )
		return "";
	i = s.indexOf( "<" );
	j = s.indexOf( ">", i + 1 );
	s.remove(0, j + 1 );
	i = s.indexOf( "<" );
	s.truncate( i );
	return s;
}
