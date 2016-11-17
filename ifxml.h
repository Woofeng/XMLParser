//
//
//	Copyright(c) by xiaobai. All rights reserved.
//
//
#ifndef _CF_XML_H_
#define _CF_XML_H_

#include<fstream>
#include<memory>
#include<stack>
#include<vector>
#include<string>
#include"interface.h"

//�����ǩ
class CLabel:public CNode{
public:
	CLabel();
	virtual ~CLabel();
public:
	std::string getNodeName()const;
	std::vector<std::pair<std::string, std::string>> getAttributeVec()const;
	std::vector<std::pair<std::wstring, std::wstring>> getAttributeVecUTF8()const;
	std::shared_ptr<CNode> getParentNode()const;
	std::shared_ptr<CNode> getFirstChild()const;
	std::shared_ptr<CNode> getLastChild()const;
	std::shared_ptr<CNode> getPreviousSiblingNode()const;
	std::shared_ptr<CNode> getNextSiblingNode()const;
	std::string getTextContent()const;
	std::wstring getTextContent(std::wstring& strContent, const std::string& CharSet = "utf-8")const;

	//�ӿ�
	//std::string getLabelName()const;
	//std::weak_ptr<CLabel> getParentLabel()const;
	//std::vector<std::shared_ptr<CLabel>> getChildLabels()const; //����Ϊ�ӱ�ǩ�ĸ���
	//std::weak_ptr<CLabel> getFirstChild()const;
	//std::weak_ptr<CLabel> getLastChild()const;
	//std::weak_ptr<CLabel> getPreviousSibling()const;
	//std::weak_ptr<CLabel> getNextSibling()const;
	//std::string getTextContent()const;
public:
	std::string m_strLabelName;
	std::string m_strContent;
	std::vector<std::pair<std::string, std::string>> m_pairKeyValue;
	std::vector<std::pair<std::wstring, std::wstring>> m_pairKeyValue2;
	std::shared_ptr<CLabel> m_pParent;
	std::shared_ptr<CLabel> m_pLeft;
	std::shared_ptr<CLabel> m_pRight;
	bool m_isClose;
	std::size_t _uniqueID;
	std::size_t _parentID;
	static std::size_t autoID;
};


//�����ļ�����ش���
class CXML {
public:
	CXML() = delete;
	CXML(const std::string& name);
	virtual ~CXML();
protected:
	//������һ��
	bool analyzeFirstLine();
protected:
	std::ifstream file;
	std::string fileName;
	std::string m_strCharSet;
	std::string m_xmlVersion;
};


//�����ļ�����
class CFXML :public CLabel,public CXML{
public:
	CFXML() = delete;
	CFXML(const std::string& name);
	//virtual std::vector<std::shared_ptr<CNode>> getNodeByAttributeAndValue(std::string&, std::string&);
	//virtual std::vector<std::shared_ptr<CNode>> getNodeByAttributeAndValue(std::pair<std::string, std::string>&);
	virtual bool exe();
	virtual ~CFXML();

private:
	//��ȡ�����ĵ�
	bool getContent();
	//��ȡ����ǩ������
	bool getRootLabel();
	std::string parsePerLine(std::string& strpercontent);
	//����ʼ��ǩ
	std::string procBeginLabel(std::string& strcontent,std::string& unprocpart);
	//�������
	std::string procEndLabel(std::string& strcontent,std::string& unprocpart);
	//��������+������ǩ
	std::string procContentAndEndLabel(std::string& strcontent, std::string& unprocpart);
	//����ԭ������ʹ�����
	std::string procOnlyContent(std::string& strcontent);
	//����ע�͡�ԭ�����
	std::string procCommentBegin(std::string& strpercontent,std::string& unprocpart);
	std::string procCommentEnd(std::string& strpercontent);
	std::string procOriginalBegin(std::string& strpercontent, std::string& unprocpart);
	std::string procOriginalEnd(std::string& strpercontent);
protected:
	std::vector<std::string> m_strVecContent;
	std::size_t m_fileLength;
	std::string m_strRootLabel;
	std::shared_ptr<CLabel> m_spRoot;
	std::vector<std::shared_ptr<CLabel>> m_vecLabel;
	std::stack<std::shared_ptr<CLabel>> m_stackLabelSPoint;
	static std::size_t LineNO;
};


class CParseXML : private CFXML{
public:
	CParseXML() = delete;
	CParseXML(const std::string& strFileName);
	virtual bool exe() { return this->CFXML::exe(); }
	std::vector<std::shared_ptr<CNode>> getNodeByAttributeAndValue(const std::string&,const std::string&);
	std::vector<std::shared_ptr<CNode>> getNodeByAttributeAndValue(const std::pair<std::string, std::string>&);
protected:
	void traverse(std::weak_ptr<CLabel> wpLabel, std::pair<std::string, std::string> attr, std::vector<std::shared_ptr<CNode>>& vecSPLabelArr);
};//End class

typedef std::shared_ptr<CNode> PNode;
#endif
