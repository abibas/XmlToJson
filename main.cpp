#include <QCoreApplication>
#include "xmljsonparser.h"

void convertXmlToJsonAndSave(const QString& inputXmlFilePath, const QString& outputJsonFilePath);
void convertJsonToXmlAndSave(const QString& inputJsonFilePath, const QString& outputXmlFilePath);
void convertJsonToXmlAndReturnString(const QString& inputJsonFilePath);

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

//    convertXmlToJsonAndSave("./test.xml", "test_XML_to_JSON.json");
//    convertJsonToXmlAndSave("./test_XML_to_JSON.json", "test_JSON_to_XML.xml");
    convertJsonToXmlAndReturnString("./test_XML_to_JSON.json");

    return a.exec();
}


void convertXmlToJsonAndSave(const QString& inputXmlFilePath, const QString& outputJsonFilePath){
    TiXmlDocument xml_document = XmlJsonParser::readXml(inputXmlFilePath);

    QVariant json_tree;
    XmlJsonParser::convertXmltoJson(xml_document, json_tree);

    qDebug() << "Genenrating " << outputJsonFilePath <<  " ...";
    XmlJsonParser::writeJson(outputJsonFilePath, json_tree);
}

void convertJsonToXmlAndSave(const QString& inputJsonFilePath, const QString& outputXmlFilePath){
    QVariant json_tree = XmlJsonParser::readJson(inputJsonFilePath);

    //convert the tree-like structure of QVariantMap to TiXmlElement
    TiXmlElement* xml_tree = new TiXmlElement("root");
    XmlJsonParser::convertJsonToXml(json_tree, xml_tree);

    //write to xml file
    std::cout << "Genenrating " << outputXmlFilePath.toStdString() << " ..." << std::endl;
    XmlJsonParser::writeXml(outputXmlFilePath, xml_tree);
}

void convertJsonToXmlAndReturnString(const QString& inputJsonFilePath){
    QVariant json_tree = XmlJsonParser::readJson(inputJsonFilePath);

    //convert the tree-like structure of QVariantMap to TiXmlElement
    TiXmlElement* xml_tree = new TiXmlElement("root");
    XmlJsonParser::convertJsonToXml(json_tree, xml_tree);

    //write to xml file
    QString result;
    XmlJsonParser::generateStringFromXml(result, xml_tree);
    qDebug() << result;
}
