#ifndef _XMLJSONPARSER_H_
#define _XMLJSONPARSER_H_

#include <json/json.h>
#include <tinyXml/tinyxml.h>
#include <QString>


/** XML and Json parser and coverter class
 *  Use Qt-based Json and TinyXML library to read/write json and xml file
 *  This class implements the conversion between Json and TinyXML's data structure
 *
 **/

class XmlJsonParser
{
public:
    //**********************************************************
    //                  Import file
    //**********************************************************
    //read json file to a QVariantMap
    static QVariant readJson(const QString in_file);

    //read xml file to TiXmlNode
    static TiXmlDocument readXml(const QString in_file);

    //**********************************************************
    //                    Convert
    //**********************************************************
    //convert a QVariant-based structure to write-ready TiXmlDocument structure
    static void convertJsonToXml(const QVariant& json_tree, TiXmlElement *xml_tree);

    //convert from xml structure to QVarant-based Json
    static void convertXmltoJson(TiXmlDocument& xml_doc, QVariant &json_tree);
    static void convertXmltoJson(const TiXmlHandle& xml_tree, QVariant& json_tree);

    //**********************************************************
    //                  Export file
    //**********************************************************
    //write json file from a QVariantMap
    static bool writeJson(const QString out_file, const QVariant json_tree);
    //write the content of TiXmlDocument to a Xml file
    static void writeXml(const QString out_file, TiXmlElement *xml_tree);
    //write the content of TiXmlDocument to a Xml file
    static void generateStringFromXml(QString& result_string, TiXmlElement *xml_tree);
};

#endif //_XMLJSONPARSER_H_
