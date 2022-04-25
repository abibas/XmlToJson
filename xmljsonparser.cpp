#include "xmljsonparser.h"

#include <QtDebug>
#include <QDateTime>
#include <QFile>
#include <QTextStream>

using namespace QtJson;

//**********************************************************
//                  Import file
//**********************************************************
//read Json file to QVariant
QVariant XmlJsonParser::readJson(const QString in_file)
{
    QVariant json_tree;
    QFile f(in_file);
    if (!f.open(QFile::ReadOnly | QFile::Text)) {
        qFatal("Cannot open the json file!");
        return json_tree;
    }

    QTextStream in(&f);
    QString json = in.readAll();

    if (json.size() == 0){ //empty string
        qDebug() << "Empty string. Returning...";
        return QVariant(); //return nothing
    }

    //parse the json sequence
    json_tree = Json::parse(json);

    return json_tree;
}

//read Xml file to TiXmlNode
TiXmlDocument XmlJsonParser::readXml(const QString in_file)
{
    TiXmlDocument doc(in_file.toStdString().c_str());

    if (!doc.LoadFile()) {
        qFatal("Couldn't load XML file!");
    }
    return doc;
}
//**********************************************************
//          END OF  Import file
//**********************************************************

//**********************************************************
//                  Export file
//**********************************************************
bool XmlJsonParser::writeJson(const QString out_file, const QVariant json_tree)
{
    QByteArray json_content = Json::serialize(json_tree);

    //write to file
    QFile json_out(out_file);
    if ( json_out.open(QIODevice::WriteOnly) ){
        json_out.write(json_content);
        json_out.close();
        return true;
    }
    return false;
}

//write the content of TiXmlDocument to a Xml file
void XmlJsonParser::writeXml(const QString out_file, TiXmlElement* xml_tree)
{
    //add xml file declaration
    TiXmlDocument doc;
    TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "", "" );
    doc.LinkEndChild( decl );

    //get generation's timestamp
    QString cur_time = QDateTime::currentDateTime().toLocalTime().toString();

    //add generatation timestamp as a comment
    TiXmlComment * comment = new TiXmlComment();
    comment->SetValue("Generated at " + cur_time.toStdString());
    doc.LinkEndChild(comment);

    //traverse through the root tree and append each element to the Doc
    TiXmlElement* child_element = xml_tree->FirstChildElement();
    while(child_element){
        //link to the clone of the node
        //since it does not allow multi-parents for a node (see the assert)
        //assert( node->parent == 0 || node->parent == this );
        doc.LinkEndChild(child_element->Clone());
        child_element = child_element->NextSiblingElement();
    }

    //save to file
    doc.SaveFile(out_file.toStdString());
}

void XmlJsonParser::generateStringFromXml(QString& result_string, TiXmlElement *xml_tree){
    TiXmlDocument doc;
    TiXmlElement* child_element = xml_tree->FirstChildElement();
    while(child_element){
        doc.LinkEndChild(child_element->Clone());
        child_element = child_element->NextSiblingElement();
    }

    TiXmlPrinter printer;
    doc.Accept(&printer);
    result_string = printer.CStr();
}
//**********************************************************
//        END OF    Export file
//**********************************************************

//**********************************************************
//                    Convert
//**********************************************************
void XmlJsonParser::convertXmltoJson(TiXmlDocument& xml_doc, QVariant &json_tree){
    TiXmlHandle xml_tree(&xml_doc);
    convertXmltoJson(xml_tree, json_tree);
}

void XmlJsonParser::convertXmltoJson(const TiXmlHandle& xml_tree, QVariant &json_tree)
{
    //loop over all branches of xml_tree and append it to QVariantMap
    TiXmlHandle cur_handle = xml_tree.FirstChildElement();
    TiXmlElement* cur_element = cur_handle.ToElement();

    //check if this branch xml's child element has the same tag name or not
    //if it has, use QList and use QVariant otherwise
    bool use_QList = false, use_QMap = false;
    TiXmlElement* next_element = cur_element->NextSiblingElement();
    //if there is no next element: use QVariantMap for sure
    if (!next_element)
        use_QMap = true;
    else if (QString(cur_element->Value())!= ""){ //at least it should be a valid tag name
        if (QString(cur_element->Value()) == QString(next_element->Value()))
            //same tag name: use list since QMap does not support multiple values for same key
            use_QList= true;
        else
            use_QMap = true;
    }

    QList<QVariant> cur_list;
    QVariantMap cur_map;

    //loop through all child element of this XML branch
    while(cur_element){
        QString element_tag = QString(cur_element->Value());
        if (element_tag != ""){ //ignore invalid or empty tag name
            //cout << element_tag.toStdString() << endl;
            //check if it's lowest level
            QVariant child_map;
            TiXmlHandle child_xml_handle = TiXmlHandle(cur_element);
            if (!child_xml_handle.FirstChildElement().ToElement()){
                child_map = QVariant(cur_element->GetText());
                //std::cout << child_map.toString().toStdString() << std::endl;
            } else {
                //recursively collect the tree structure
                convertXmltoJson(child_xml_handle, child_map);
            }
            if (use_QMap){
                //map the found branch to the current tag
                cur_map[element_tag] = child_map;
            }else {
                //construct a QMap for each element then warp it with a QVariant
                QVariantMap ele_map;
                ele_map[element_tag] = child_map;
                cur_list.append(QVariant(ele_map));
            }
        }
        //go to next tag
        cur_element = cur_element->NextSiblingElement();
    }
    if (use_QMap){
        //warp the current map to be QVariant
        json_tree = QVariant(cur_map);
    } else {
        json_tree = QVariant(cur_list);
    }
}

//convert a QVariant-based structure to write-ready TiXmlDocument structure
void XmlJsonParser::convertJsonToXml(const QVariant& json_tree, TiXmlElement* xml_tree)
{
    bool is_QMap = false, is_QList = false;
    QVariantMap cur_map;
    QVariantList cur_list;
    //check if the QVariant is a QList or QMap
    if (json_tree.userType() == QVariant::Map){ //It's a QVariantMap
        is_QMap = true;
        cur_map = json_tree.toMap();
    } else if (json_tree.userType() == QVariant::List){ //It's a QVariantList
        is_QList = true;
        cur_list = json_tree.toList();
    } else {
        qFatal("Invalid QVariant type");
        return;
    }

    //travel through the QMap or QList
    if (is_QMap) {
        for(QMap<QString, QVariant>::iterator iter = cur_map.begin(); iter!= cur_map.end(); ++iter){
            //check if this is only a string -> last level
            std::string cur_key = iter.key().toStdString();
            QVariant cur_value = iter.value();

            //form the TiXMLElement: convert the QString key to const char*
            TiXmlElement* child_element = new TiXmlElement(cur_key);
            //link the newly formed element to its parent
            xml_tree->LinkEndChild(child_element);
            if (cur_value.userType() == QVariant::String){
                std::string attri_value = cur_value.toString().toStdString();
                TiXmlText * text = new TiXmlText( attri_value);
                child_element->LinkEndChild( text);
            } else {
                //find the child element
                convertJsonToXml(cur_value, child_element);
            }
        }
    } else if (is_QList) { //is_QList
        for(QVariantList::iterator iter = cur_list.begin(); iter!= cur_list.end(); ++iter){
            //get the list element and convert it to QMap
            QVariantMap cur_map_element = (*iter).toMap();
            //get first element of the QMap
            QVariantMap::iterator iter_map = cur_map_element.begin();
            std::string cur_key = iter_map.key().toStdString();
            QVariant cur_value = iter_map.value();

            //form the TiXMLElement: convert the QString key to const char*
            TiXmlElement* child_element = new TiXmlElement(cur_key);
            //link the newly formed element to its parent
            xml_tree->LinkEndChild(child_element);
            if (cur_value.userType() == QVariant::String){
                std::string attri_value = cur_value.toString().toStdString();
                TiXmlText * text = new TiXmlText( attri_value);
                child_element->LinkEndChild( text);
            } else {
                //find the child element
                convertJsonToXml(cur_value, child_element);
            }
        }
    } else {
        //the code should never go here
        return;
    }
}
//**********************************************************
//           END OF    Convert
//**********************************************************
