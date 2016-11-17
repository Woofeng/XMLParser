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
*			Label 实现
*
*****************************************/

size_t CLabel::autoID = 1;
//构造函数
CLabel::CLabel() :m_strLabelName(""), m_strContent(" "), _uniqueID(CLabel::autoID++) {}

CLabel::~CLabel() {
}

//接口
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
		return nullSP; //是的 你没有看错 返回空shared_ptr
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
*			XML 实现
*
*****************************************/

CXML::CXML(const string& name) :fileName(name) {
	try {
		file.open(fileName, ios::in);
		if (!file) {
			throw runtime_error("文件打开失败！");
		}
	}
	catch (runtime_error& err) {
		cout << err.what() << endl;
	}
}

CXML::~CXML() {
}

//解析第一行
bool CXML::analyzeFirstLine() {
	string strFirstLine;
	getline(file, strFirstLine);
	//cout << "第一行1：" << strFirstLine << endl;
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
		//无BOM
	}

	//细节处理
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
*			FXML 实现
*
*****************************************/

std::size_t CFXML::LineNO = 2;
//构造函数
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
		//去掉行首空白字符
		string pattern = "(\\s*)(.*)";
		regex reg(pattern);
		string fmt("$2");
		string strAfterParse;
		regex_replace(std::back_inserter(strAfterParse), strPerLine.begin(), strPerLine.end(), reg, fmt);

		//去掉行尾空白字符
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

//处理开始标签
string CFXML::procBeginLabel(string& strContent, string& unProcPart) {
	try {
		//默认对标签
		bool bSingleLabel = false;
		string::size_type posEndSymbol = strContent.find(">");

		string strFirstLabel;
		if (strContent[posEndSymbol - 1] == '/') { //单标签
			bSingleLabel = true;
			strFirstLabel = strContent.substr(0, posEndSymbol) + ">";
		}
		else {
			strFirstLabel = strContent.substr(0, posEndSymbol + 1);
		}
		unProcPart = strContent.substr(posEndSymbol + 1);

		//bSingleLabel ? cout << "单" << endl : cout << "对" << endl;
		//cout << strFirstLabel << ":zzzzzz:" << unProcPart << endl;

		//对标签
		//if ( posEndSymbol != string::npos ){
		//	cout << "单标签：" << strFirstLabel << endl;
		//	bSingleLabel = false;
		//	posEndSymbol = strContent.find(">");
		//	strFirstLabel = strContent.substr(0, posEndSymbol + 1);
		//	unProcPart = strContent.substr(posEndSymbol + 1);
		//}




		//string strFirstLabel = strContent.substr(0, posEndSymbol + 1);
		//unProcPart = strContent.substr(posEndSymbol + 1);

		//cout << "strFirstLabel=" << strFirstLabel << endl;
		//cout << "unProcPart =" << unProcPart << endl;

		//键值对中的空格 即等号两边的空格
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

		//当前标签的信息
		string name = strFirstLabel.substr(1, posBlank - 1);
		//cout << "标签名："<< name << endl;
		sp->m_strLabelName = name;
		if (bSingleLabel) {
			sp->m_isClose = 0; //标签已关闭
		}
		else {
			sp->m_isClose = 1; //标签未关闭
		}

		sp->m_pLeft = nullptr;
		sp->m_pRight = nullptr;

		sp->m_pParent = this->m_stackLabelSPoint.top();
		//挂在父结点 begin
		if (sp->m_pParent->m_pLeft == nullptr) {
			sp->m_pParent->m_pLeft = sp;
		}
		else {
			shared_ptr<CLabel>* spRight = &(sp->m_pParent->m_pRight);
			//真TMD不容易
			while (*spRight) {
				spRight = &((*spRight)->m_pRight);
			}
			*spRight = sp;
		}
		//挂在父结点 end

		sp->m_strContent = "";
		sp->_parentID = this->m_stackLabelSPoint.top()->_uniqueID;


		//获取当前标签的属性和值 begin
		string key;
		string val;
		for (sregex_iterator it(strFirstLabel.begin(), strFirstLabel.end(), reg), end_it; it != end_it; ++it) {
			//cout << it->str() << " ";
			string::size_type pos = it->str().find("=");
			key = it->str().substr(0, pos);
			val = it->str().substr(pos + 2, it->str().size() - pos - 3);
			//cout << "key="<< key << " " << "val=" << val << endl;

			sp->m_pairKeyValue.push_back(make_pair(key, val)); //pair
			//UTF8编码
			std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
			wstring wKey = myconv.from_bytes(key);
			wstring wVal = myconv.from_bytes(val);
			pair<wstring, wstring> wStrPair(wKey,wVal);
			sp->m_pairKeyValue2.push_back( wStrPair );

		}
		//获取当前标签的属性和值 end

		//对标签压栈 单标签不用 挂在父结点就好
		if (!bSingleLabel) {
			this->m_stackLabelSPoint.push(sp);
			//cout << "压栈的标签" << this->m_stackLabelSPoint.top()->m_strLabelName << endl;
		}

		//cout << "" << endl;
		//cout << "top: " << this->m_stackLabelSPoint.top()->m_strLabelName << endl;


	}
	catch (regex_error& err) {
		cout << err.code() << err.what() << endl;
	}
	return "";
}

//处理结束标签
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

	//m_isClose == 1未关闭
	if ((this->m_stackLabelSPoint.top()->m_strLabelName == strEndLabelName) && (this->m_stackLabelSPoint.top()->m_isClose == 1)) {
		this->m_stackLabelSPoint.top()->m_isClose = 0;
		shared_ptr<CLabel> spCurr = this->m_stackLabelSPoint.top(); //当前结点
		this->m_stackLabelSPoint.pop();

		//根结点没有父结点 处理非根结点
		if (!this->m_stackLabelSPoint.empty()) {
			spCurr->_parentID = this->m_stackLabelSPoint.top()->_uniqueID;
			shared_ptr<CLabel> spParent = this->m_stackLabelSPoint.top(); //父结点
			if (spParent != this->m_spRoot) {
				//父结点不为根结点
				//本标签的标签名和内容 也为父结点的属性
				//spParent->m_pairKeyValue.push_back( make_pair(spCurr->m_strLabelName, spCurr->m_strContent) );

				//本标签的属性和值 也为父结点的属性
				//for (auto e:(spCurr->m_pairKeyValue) ){
				//	spParent->m_pairKeyValue.push_back( e );
				//}
			}
		}
	}
	else {
		//cout << "state: " << this->m_stackLabelSPoint.top()->m_isClose << endl;
		if (1 == this->m_stackLabelSPoint.top()->m_isClose) {
			// 缺少配对的结束标签导致   当前结束标签与栈顶元素不同
			string strLineNO = to_string(CFXML::LineNO);
			try {
				throw runtime_error(strLineNO + "：缺少匹配的标签！");
			}
			catch (runtime_error& err) {
				cout << err.what() << endl;
				unexpected();
			}
		}
	}

	return "";
}

//处理内容+结束标签
string CFXML::procContentAndEndLabel(string& strContent, string& unProcPart) {
	size_t Len = strContent.size();
	//cout << "其后是内容加上结束标签" << endl;
	string::size_type posBeg = strContent.find("</");
	string strOnlyContent = strContent.substr(0, posBeg);
	//假设该处已删除 内容前部多余的空格
	//cout << "strOnlyContent =" << strOnlyContent << endl;
	this->procOnlyContent(strOnlyContent);
	//parsePerLine(strOnlyContent);
	//处理第一个结束标签
	string::size_type posBeg2 = strContent.find(">");
	unProcPart = strContent.substr(posBeg2 + 1);

	return "";
}

//处理注释
string CFXML::procCommentBegin(string& strContent, string& unProcPart) {
	string::size_type posEndSymbol = strContent.find("-->");
	if (posEndSymbol == string::npos) {
		//处理多行注释
		shared_ptr<CLabel> sp(new CLabel);
		sp->m_strLabelName = "-->";
		sp->m_isClose = 1;
		sp->_parentID = -1;//，没有父结点
		this->m_stackLabelSPoint.push(sp);
		this->procOnlyContent(strContent.substr(3));
	}
	else {
		//在一行内完整的注释 忽略不处理
		unProcPart = strContent.substr(posEndSymbol + 3);
	}
	return "";
}

string CFXML::procCommentEnd(string& strContent) {
	//处理注释跨越多行 即此时注释标签未关闭
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
		//跨越多行的原样输出
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
	if (posEndSymbol == string::npos) { //未找到结束标签
		this->procOnlyContent(strContent);
	}
	else {
		strContent.substr(0, posEndSymbol);
		this->procOnlyContent(strContent);
	}

	return strContent;
}

//处理原样输出 多行注释 多行内容
string CFXML::procOnlyContent(string& strContent) {

	//cout << "处理纯内容" << strContent << endl;
	if (strContent.empty()) return "";
	//原样输出 标签未关闭
	if ((this->m_stackLabelSPoint.top()->m_isClose == 1) && (this->m_stackLabelSPoint.top()->m_strLabelName != "<![CDATA[")) {
		this->m_stackLabelSPoint.top()->m_strContent += strContent;
	}
	else if ((this->m_stackLabelSPoint.top()->m_isClose == 1) && (this->m_stackLabelSPoint.top()->m_strLabelName == "-->")) {
		//多行注释 当前标签未关闭 不处理
	}
	else {
		try {
			throw runtime_error(": 格式有误！");
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

	//删除前置和后置的空白字符
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
		//开始标签 不处理跨越多行的开始标签
		this->procBeginLabel(strPerContent, unProcPart);
	}
	else if (strFront_2 == "</") {
		//结束标签 不处理跨越多行的结束标签
		this->procEndLabel(strPerContent, unProcPart);
	}
	else if (strFront_4 == "<!--") {
		//注释标签 需处理多行注释
		this->procCommentBegin(strPerContent, unProcPart);
	}
	else if (strFront_9 == "<![CDATA[") {
		//原文输出 需处理多行原文输出
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
*			ParseXML 实现
*
*****************************************/

CParseXML::CParseXML(const std::string& strFileName):CFXML(strFileName) {
	
}

//接口 //遍历
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