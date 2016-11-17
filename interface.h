//
//
//	Copyright(c) by xiaobai. All rights reserved.
//
//
#ifndef _INTER_FACE_H_
#define _INTER_FACE_H_
#include<memory>
#include<vector>

//供用户使用的接口
class CNode{
public:
	virtual std::string getNodeName()const = 0;
	virtual std::vector<std::pair<std::string, std::string>> getAttributeVec()const = 0;
	virtual std::vector<std::pair<std::wstring, std::wstring>> getAttributeVecUTF8()const = 0;
	virtual std::shared_ptr<CNode> getParentNode()const = 0;
	virtual std::shared_ptr<CNode> getFirstChild()const = 0;
	virtual std::shared_ptr<CNode> getLastChild()const = 0;
	virtual std::shared_ptr<CNode> getPreviousSiblingNode()const = 0;
	virtual std::shared_ptr<CNode> getNextSiblingNode()const = 0;
	virtual std::string getTextContent()const = 0;
	virtual std::wstring getTextContent(std::wstring& strContent, const std::string& CharSet = "utf-8")const = 0;

}; //End class

#endif // !_INTER_FACE_H_