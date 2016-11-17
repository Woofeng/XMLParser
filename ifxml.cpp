#include<iostream>
#include<memory>
#include<algorithm>
#include<regex>
#include<string>
#include<fstream>
#include<vector>
#include<stack>
#include<exception>
#include<locale>
#include<codecvt>

#include "ifxml.h"

using namespace std;

/****************************************
*
*			Label ʵ��
*
*****************************************/

size_t CLabel::autoID = 1;
//���캯��
CLabel::CLabel() :m_strLabelName(""), m_strContent(" "), _uniqueID(CLabel::autoID++) {}

CLabel::~CLabel() {
}

//�ӿ�
std::string CLabel::getNodeName()const {
	return this->m_strLabelName;
}

std::vector<std::pair<std::string, std::string>> CLabel::getAttributeVec()const {
	return this->m_pairKeyValue;
}

std::vector<std::pair<std::wstring, std::wstring>> CLabel::getAttributeVecUTF8()const {
	return this->m_pairKeyValue2;
}

std::shared_ptr<CNode> CLabel::getParentNode()const{
	return this->m_pParent;
}

std::shared_ptr<CNode> CLabel::getFirstChild()const {
	return this->m_pLeft;
}

//std::vector<std::shared_ptr<CLabel>> CLabel::getChildLabels()const{
//	std::vector<std::shared_ptr<CLabel>> vecSPLabel;
//	if (this->getFirstChild().lock() ) {
//		vecSPLabel.push_back(this->getFirstChild().lock());
//		
//		std::shared_ptr<CLabel> FirstRight = this->m_pParent->m_pRight;
//		while ( FirstRight ){
//			vecSPLabel.push_back(FirstRight);
//			FirstRight = FirstRight->m_pRight;
//		}
//	}
//	
//	return vecSPLabel;
//}
//

std::shared_ptr<CNode> CLabel::getLastChild()const{
	if (this->m_pRight == nullptr) {
		return this->m_pLeft;
	}else{
		shared_ptr<CLabel> spTmp(new CLabel(*this));
		while ( spTmp->m_pRight ){
			spTmp = spTmp->m_pRight;
		}
		return spTmp;
	}
}

std::shared_ptr<CNode> CLabel::getPreviousSiblingNode()const {
	if (this->m_pParent->m_pLeft.get() == this) {
		shared_ptr<CLabel> nullSP;
		return nullSP; //�ǵ� ��û�п��� ���ؿ�shared_ptr
	}else {
		return this->m_pParent;
	}
}

std::shared_ptr<CNode> CLabel::getNextSiblingNode()const {
	if (this->m_pParent->m_pLeft.get() == this) {
		return this->m_pParent->m_pRight;
	}
	else {
		return this->m_pRight;
	}
}

std::string CLabel::getTextContent()const {
	return this->m_strContent;
}

std::wstring CLabel::getTextContent(std::wstring& strContent, const std::string& CharSet)const {
	
	std::string strTmp = this->m_strContent;
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	strContent = myconv.from_bytes(strTmp);

	return strContent;
}


/****************************************
*
*			XML ʵ��
*
*****************************************/

CXML::CXML(const string& name) :fileName(name) {
	try {
		file.open(fileName, ios::in);
		if (!file) {
			throw runtime_error("�ļ���ʧ�ܣ�");
		}
	}
	catch (runtime_error& err) {
		cout << err.what() << endl;
	}
}

CXML::~CXML() {
}

//������һ��
bool CXML::analyzeFirstLine() {
	string strFirstLine;
	getline(file, strFirstLine);
	//cout << "��һ��1��" << strFirstLine << endl;
	unsigned char c1 = strFirstLine.at(0);
	unsigned char c2 = strFirstLine.at(1);
	unsigned char c3 = strFirstLine.at(2);

	if (c1 == 0XEF && c2 == 0XBB && c3 == 0XBF) {
		this->m_strCharSet = "utf-8";
		strFirstLine = strFirstLine.substr(3);
	}
	else if ((c1 == 0XFE && c2 == 0XFF) || (c1 == 0XFF) && (c2 == 0XFE)) {
		this->m_strCharSet = "gb2312";
		strFirstLine = strFirstLine.substr(2);
		cout << strFirstLine << endl;
	}
	else {
		//��BOM
	}

	//ϸ�ڴ���
	replace(strFirstLine.begin(), strFirstLine.end(), '\'', '\"');
	transform(strFirstLine.begin(), strFirstLine.end(), strFirstLine.begin(), tolower);

	try {
		string pattern = "(\\s*)(\\<\\?xml)([\\s|\\t])+(version)([\\s|\\t])*(=)([\\s|\\t])*";
		pattern += "(\")(1.0|2.0)(\")";
		pattern += "([\\s|\\t])+(encoding)([\\s|\\t])*(=)([\\s|\\t])*(\")(gb2312|utf-8)+(\")(\\s|\\t)*(\\?\\>)$";

		regex reg(pattern, regex::icase);
		string fmt = "$2 $4$6$8$9$10 $12$14$16$17$18 $20";
		strFirstLine = regex_replace(strFirstLine, reg, fmt);
		//cout << strFirstLine << endl;
	}
	catch (regex_error& err) {
		cout << err.code() << err.what() << endl;
	}

	std::size_t beg = strFirstLine.find("version=");
	beg += sizeof("version=");
	std::size_t end = strFirstLine.find("\"", beg); //find(s2,pos=0)
	this->m_xmlVersion = strFirstLine.substr(beg, end - beg);

	if (string::npos == strFirstLine.find("utf-8")) {
		this->m_strCharSet.empty() ? this->m_strCharSet = "gb2312" : this->m_strCharSet;
	}
	else {
		this->m_strCharSet.empty() ? this->m_strCharSet = "utf-8" : this->m_strCharSet;
	}

	return true;
}


/****************************************
*
*			FXML ʵ��
*
*****************************************/

std::size_t CFXML::LineNO = 2;
//���캯��
CFXML::CFXML(const string& fileName) :CXML(fileName){
}

bool CFXML::exe() {
	this->analyzeFirstLine();
	this->getRootLabel();
	this->getContent();
	return true;
}

bool CFXML::getRootLabel() {
	string strRoot;
	getline(file, strRoot);
	replace(strRoot.begin(), strRoot.end(), '\'', '\"');

	string::size_type pos = strRoot.find('>');
	strRoot.insert(pos, " ");
	//cout << strRoot << endl;

	string pattern("(\\s)*(<)(\\w+)([\\s|\\t])*(>)$");
	regex reg(pattern);
	string fmt = "$3";

	this->m_strRootLabel = regex_replace(strRoot, reg, fmt);
	//cout << "this->m_strRootLabel=" << this->m_strRootLabel << endl;

	shared_ptr<CLabel> sp(new CLabel);

	sp->m_strLabelName = this->m_strRootLabel;
	sp->m_pLeft = sp->m_pRight = nullptr;
	this->m_stackLabelSPoint.push(sp);
	this->m_spRoot = sp;
	CFXML::LineNO++;

	return true;
}

bool CFXML::getContent() {
	string strPerLine;
	while (getline(file, strPerLine)) {
		//ȥ�����׿հ��ַ�
		string pattern = "(\\s*)(.*)";
		regex reg(pattern);
		string fmt("$2");
		string strAfterParse;
		regex_replace(std::back_inserter(strAfterParse), strPerLine.begin(), strPerLine.end(), reg, fmt);

		//ȥ����β�հ��ַ�
		size_t i = 0;
		if (!strAfterParse.empty()) {
			i = strAfterParse.size() - 1;
			while (iswspace(strAfterParse.at(i))) {
				--i;
			}
			strAfterParse = strAfterParse.substr(0, i + 2);
		}

		pattern = "(<)(\\w+)([\\s|\\t])*(.*)";
		reg = pattern;
		fmt = "$1$2 $4";
		string strAfterParsePart;
		strAfterParsePart = regex_replace(strAfterParse, reg, fmt);

		if (!strAfterParsePart.empty()) {
			parsePerLine(strAfterParsePart);
		}
		CFXML::LineNO++;
	}

	return true;
}

//����ʼ��ǩ
string CFXML::procBeginLabel(string& strContent, string& unProcPart) {
	try {
		//Ĭ�϶Ա�ǩ
		bool bSingleLabel = false;
		string::size_type posEndSymbol = strContent.find(">");

		string strFirstLabel;
		if (strContent[posEndSymbol - 1] == '/') { //����ǩ
			bSingleLabel = true;
			strFirstLabel = strContent.substr(0, posEndSymbol) + ">";
		}
		else {
			strFirstLabel = strContent.substr(0, posEndSymbol + 1);
		}
		unProcPart = strContent.substr(posEndSymbol + 1);

		//bSingleLabel ? cout << "��" << endl : cout << "��" << endl;
		//cout << strFirstLabel << ":zzzzzz:" << unProcPart << endl;

		//�Ա�ǩ
		//if ( posEndSymbol != string::npos ){
		//	cout << "����ǩ��" << strFirstLabel << endl;
		//	bSingleLabel = false;
		//	posEndSymbol = strContent.find(">");
		//	strFirstLabel = strContent.substr(0, posEndSymbol + 1);
		//	unProcPart = strContent.substr(posEndSymbol + 1);
		//}




		//string strFirstLabel = strContent.substr(0, posEndSymbol + 1);
		//unProcPart = strContent.substr(posEndSymbol + 1);

		//cout << "strFirstLabel=" << strFirstLabel << endl;
		//cout << "unProcPart =" << unProcPart << endl;

		//��ֵ���еĿո� ���Ⱥ����ߵĿո�
		auto itNext = strFirstLabel.begin() + 1;
		auto it = strFirstLabel.begin();
		for (; *it != '>'; ) {
			itNext = it + 1;
			if (*it == ' ' && *itNext == '=') {
				it = strFirstLabel.erase(it);
			}
			else if (*it == '=' && *itNext == ' ') {
				it = strFirstLabel.erase(itNext);
			}
			else if (*it == ' '  && *itNext == ' ') {
				it = strFirstLabel.erase(it);
			}
			else {
				++it;
			}
			itNext = it + 1;
		}


		string pattern = "\\w+=[\"|\']\\w+[\"|\']";
		regex reg(pattern);
		string fmt = "";
		shared_ptr<CLabel> sp(new CLabel);

		string::size_type posBlank = strFirstLabel.find(" ");
		if (posBlank == string::npos) {
			strFirstLabel.insert(strFirstLabel.end() - 1, ' ');
			posBlank = strFirstLabel.find(' ');
		}

		//��ǰ��ǩ����Ϣ
		string name = strFirstLabel.substr(1, posBlank - 1);
		//cout << "��ǩ����"<< name << endl;
		sp->m_strLabelName = name;
		if (bSingleLabel) {
			sp->m_isClose = 0; //��ǩ�ѹر�
		}
		else {
			sp->m_isClose = 1; //��ǩδ�ر�
		}

		sp->m_pLeft = nullptr;
		sp->m_pRight = nullptr;

		sp->m_pParent = this->m_stackLabelSPoint.top();
		//���ڸ���� begin
		if (sp->m_pParent->m_pLeft == nullptr) {
			sp->m_pParent->m_pLeft = sp;
		}
		else {
			shared_ptr<CLabel>* spRight = &(sp->m_pParent->m_pRight);
			//��TMD������
			while (*spRight) {
				spRight = &((*spRight)->m_pRight);
			}
			*spRight = sp;
		}
		//���ڸ���� end

		sp->m_strContent = "";
		sp->_parentID = this->m_stackLabelSPoint.top()->_uniqueID;


		//��ȡ��ǰ��ǩ�����Ժ�ֵ begin
		string key;
		string val;
		for (sregex_iterator it(strFirstLabel.begin(), strFirstLabel.end(), reg), end_it; it != end_it; ++it) {
			//cout << it->str() << " ";
			string::size_type pos = it->str().find("=");
			key = it->str().substr(0, pos);
			val = it->str().substr(pos + 2, it->str().size() - pos - 3);
			//cout << "key="<< key << " " << "val=" << val << endl;

			sp->m_pairKeyValue.push_back(make_pair(key, val)); //pair
			//UTF8����
			std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
			wstring wKey = myconv.from_bytes(key);
			wstring wVal = myconv.from_bytes(val);
			pair<wstring, wstring> wStrPair(wKey,wVal);
			sp->m_pairKeyValue2.push_back( wStrPair );

		}
		//��ȡ��ǰ��ǩ�����Ժ�ֵ end

		//�Ա�ǩѹջ ����ǩ���� ���ڸ����ͺ�
		if (!bSingleLabel) {
			this->m_stackLabelSPoint.push(sp);
			//cout << "ѹջ�ı�ǩ" << this->m_stackLabelSPoint.top()->m_strLabelName << endl;
		}

		//cout << "" << endl;
		//cout << "top: " << this->m_stackLabelSPoint.top()->m_strLabelName << endl;


	}
	catch (regex_error& err) {
		cout << err.code() << err.what() << endl;
	}
	return "";
}

//���������ǩ
string CFXML::procEndLabel(string& strContent, string& unProcPart) {

	string::size_type posEnd = strContent.find(">");
	string strNeedProc;
	strNeedProc = strContent.substr(0, posEnd + 1);
	unProcPart = strContent.substr(posEnd + 1);

	//string pattern = "(\\s*)(</)(\\S*)(>)";
	string pattern = "(\\s*)(</)([^>\\s]+)(>)(.*)";
	string fmt = "$3";
	regex reg(pattern);
	string strEndLabelName = regex_replace(strNeedProc, reg, fmt);


	//cout << "strEndLabelName:" << strEndLabelName << endl;
	//unProcPart = strContent.substr(strContent.find(">")+1);

	//m_isClose == 1δ�ر�
	if ((this->m_stackLabelSPoint.top()->m_strLabelName == strEndLabelName) && (this->m_stackLabelSPoint.top()->m_isClose == 1)) {
		this->m_stackLabelSPoint.top()->m_isClose = 0;
		shared_ptr<CLabel> spCurr = this->m_stackLabelSPoint.top(); //��ǰ���
		this->m_stackLabelSPoint.pop();

		//�����û�и���� ����Ǹ����
		if (!this->m_stackLabelSPoint.empty()) {
			spCurr->_parentID = this->m_stackLabelSPoint.top()->_uniqueID;
			shared_ptr<CLabel> spParent = this->m_stackLabelSPoint.top(); //�����
			if (spParent != this->m_spRoot) {
				//����㲻Ϊ�����
				//����ǩ�ı�ǩ�������� ҲΪ����������
				//spParent->m_pairKeyValue.push_back( make_pair(spCurr->m_strLabelName, spCurr->m_strContent) );

				//����ǩ�����Ժ�ֵ ҲΪ����������
				//for (auto e:(spCurr->m_pairKeyValue) ){
				//	spParent->m_pairKeyValue.push_back( e );
				//}
			}
		}
	}
	else {
		//cout << "state: " << this->m_stackLabelSPoint.top()->m_isClose << endl;
		if (1 == this->m_stackLabelSPoint.top()->m_isClose) {
			// ȱ����ԵĽ�����ǩ����   ��ǰ������ǩ��ջ��Ԫ�ز�ͬ
			string strLineNO = to_string(CFXML::LineNO);
			try {
				throw runtime_error(strLineNO + "��ȱ��ƥ��ı�ǩ��");
			}
			catch (runtime_error& err) {
				cout << err.what() << endl;
				unexpected();
			}
		}
	}

	return "";
}

//��������+������ǩ
string CFXML::procContentAndEndLabel(string& strContent, string& unProcPart) {
	size_t Len = strContent.size();
	//cout << "��������ݼ��Ͻ�����ǩ" << endl;
	string::size_type posBeg = strContent.find("</");
	string strOnlyContent = strContent.substr(0, posBeg);
	//����ô���ɾ�� ����ǰ������Ŀո�
	//cout << "strOnlyContent =" << strOnlyContent << endl;
	this->procOnlyContent(strOnlyContent);
	//parsePerLine(strOnlyContent);
	//�����һ��������ǩ
	string::size_type posBeg2 = strContent.find(">");
	unProcPart = strContent.substr(posBeg2 + 1);

	return "";
}

//����ע��
string CFXML::procCommentBegin(string& strContent, string& unProcPart) {
	string::size_type posEndSymbol = strContent.find("-->");
	if (posEndSymbol == string::npos) {
		//�������ע��
		shared_ptr<CLabel> sp(new CLabel);
		sp->m_strLabelName = "-->";
		sp->m_isClose = 1;
		sp->_parentID = -1;//��û�и����
		this->m_stackLabelSPoint.push(sp);
		this->procOnlyContent(strContent.substr(3));
	}
	else {
		//��һ����������ע�� ���Բ�����
		unProcPart = strContent.substr(posEndSymbol + 3);
	}
	return "";
}

string CFXML::procCommentEnd(string& strContent) {
	//����ע�Ϳ�Խ���� ����ʱע�ͱ�ǩδ�ر�
	if (this->m_stackLabelSPoint.top()->m_isClose == 1) {
		if (this->m_stackLabelSPoint.top()->m_strLabelName == "-->") {
			this->m_stackLabelSPoint.top()->m_strContent += " " + strContent.substr(0, strContent.find("-->"));
			this->m_stackLabelSPoint.top()->m_isClose = 0;
			this->m_stackLabelSPoint.pop();
		}
	}
	return "";
}

string CFXML::procOriginalBegin(string& strContent, string& unProcPart) {
	if (strContent.empty()) {
		return "";
	}

	strContent = strContent.substr(9);
	string::size_type posEndSymbol = strContent.find("]]>");
	if (posEndSymbol == string::npos) {
		//��Խ���е�ԭ�����
		this->m_stackLabelSPoint.top()->m_isClose = 1;
		this->procOnlyContent(strContent);
	}
	else {
		strContent.substr(0, posEndSymbol);
		this->procOnlyContent(strContent);
		this->m_stackLabelSPoint.top()->m_isClose = 0;
		unProcPart = strContent.substr(posEndSymbol + 3);
	}

	return strContent;
}
string CFXML::procOriginalEnd(string& strContent) {
	if (strContent.empty()) {
		return "";
	}

	string::size_type posEndSymbol = strContent.find("]]>");
	if (posEndSymbol == string::npos) { //δ�ҵ�������ǩ
		this->procOnlyContent(strContent);
	}
	else {
		strContent.substr(0, posEndSymbol);
		this->procOnlyContent(strContent);
	}

	return strContent;
}

//����ԭ����� ����ע�� ��������
string CFXML::procOnlyContent(string& strContent) {

	//cout << "��������" << strContent << endl;
	if (strContent.empty()) return "";
	//ԭ����� ��ǩδ�ر�
	if ((this->m_stackLabelSPoint.top()->m_isClose == 1) && (this->m_stackLabelSPoint.top()->m_strLabelName != "<![CDATA[")) {
		this->m_stackLabelSPoint.top()->m_strContent += strContent;
	}
	else if ((this->m_stackLabelSPoint.top()->m_isClose == 1) && (this->m_stackLabelSPoint.top()->m_strLabelName == "-->")) {
		//����ע�� ��ǰ��ǩδ�ر� ������
	}
	else {
		try {
			throw runtime_error(": ��ʽ����");
		}
		catch (const std::runtime_error& err) {
			cout << to_string(CFXML::LineNO) << err.what() << endl;
			std::unexpected();
		}
	}
	return "";
}

string deleteFrontAndLastBlank(string& strContent) {
	if (strContent.empty()) return "";
	string::size_type Len = strContent.size();
	size_t i = 0;

	//ɾ��ǰ�úͺ��õĿհ��ַ�
	for (; (i<Len) && (iswspace(strContent[i])); ++i) {
	}
	if (i > 1) {
		strContent = strContent.substr(i);
		Len = strContent.size();
	}

	for (i = Len - 1; (i >= 0) && (iswspace(strContent[i])); --i) {
	}
	strContent = strContent.substr(0, i + 1);
	return strContent;
}

string CFXML::parsePerLine(string& strPerContent) {

	if (strPerContent.empty()) {
		return "";
	}
	string::size_type Len = strPerContent.size();
	size_t i = 0;
	deleteFrontAndLastBlank(strPerContent);
	Len = strPerContent.size();

	//cout << "LEN = " << Len << endl;

	string strFront_2 = strPerContent.size() >= 2 ? strPerContent.substr(0, 2) : "";
	string strFront_4 = strPerContent.size() >= 4 ? strPerContent.substr(0, 4) : "";
	string strFront_9 = strPerContent.size() >= 9 ? strPerContent.substr(0, 9) : "";

	//	]]>
	string unProcPart;
	if (strPerContent.at(0) == '<' && strPerContent.at(1) != '/' && strPerContent.at(1) != '!') {
		//��ʼ��ǩ �������Խ���еĿ�ʼ��ǩ
		this->procBeginLabel(strPerContent, unProcPart);
	}
	else if (strFront_2 == "</") {
		//������ǩ �������Խ���еĽ�����ǩ
		this->procEndLabel(strPerContent, unProcPart);
	}
	else if (strFront_4 == "<!--") {
		//ע�ͱ�ǩ �账�����ע��
		this->procCommentBegin(strPerContent, unProcPart);
	}
	else if (strFront_9 == "<![CDATA[") {
		//ԭ����� �账�����ԭ�����
		this->procOriginalBegin(strPerContent, unProcPart);
	}
	else {
		string::size_type posSymbol = strPerContent.find_first_of("<]-");
		this->procOnlyContent(strPerContent.substr(0, posSymbol));
		unProcPart = strPerContent.substr(posSymbol);

	}// else end

	if (!unProcPart.empty()) {
		parsePerLine(unProcPart);
	}

	return "";
}

CFXML::~CFXML() {
}

/****************************************
*
*			ParseXML ʵ��
*
*****************************************/

CParseXML::CParseXML(const std::string& strFileName):CFXML(strFileName) {
	
}

//�ӿ� //����
void CParseXML::traverse(std::weak_ptr<CLabel> wpLabel, std::pair<std::string, std::string> attr, std::vector<std::shared_ptr<CNode>>& vecSPLabelArr) {
	if (wpLabel.lock() == nullptr) return;

	for (auto e : (wpLabel.lock()->m_pairKeyValue)) {
		if (e == attr) {
			vecSPLabelArr.push_back(wpLabel.lock());
			break;
		}
	}

	traverse(wpLabel.lock()->m_pLeft, attr, vecSPLabelArr);
	traverse(wpLabel.lock()->m_pRight, attr, vecSPLabelArr);
}

std::vector<std::shared_ptr<CNode>> CParseXML::getNodeByAttributeAndValue(const std::string& key,const std::string& val) {
	std::vector<std::shared_ptr<CNode>> vecSPNode;
	traverse(this->m_spRoot, make_pair(key, val), vecSPNode);
	
	return vecSPNode;
}

std::vector<std::shared_ptr<CNode>> CParseXML::getNodeByAttributeAndValue(const std::pair<std::string,std::string>& pairStr) {
	return this->getNodeByAttributeAndValue(pairStr.first, pairStr.second);
}