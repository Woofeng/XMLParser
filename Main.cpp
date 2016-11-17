#include "interface.h"
#include "ifxml.h"
#include <iostream>

using namespace std;

int main() {
	
	CParseXML file("要打开的文件.xml");
	file.exe();
	vector<PNode> vec = file.getNodeByAttributeAndValue("属性名","属性值");
	
	if (vec.empty() ) {
		cout << "空 " << endl;
	}else {
		wcout.imbue(locale(".936"));
		for ( auto& e:vec){
			cout << "标签名:" << e->getNodeName() << endl;

			for (auto& ee : e->getAttributeVec()) {
				cout << "属性名：" << ee.first << " " << "属性值：" << ee.second << endl;
			}
			//或
			for ( auto& ee:e->getAttributeVecUTF8() ){			
				wcout << L"属性名：" << ee.first << " " << L"属性值：" <<ee.second << endl;
			}
		}
	}//End else

	if (vec.at(0)) {
		PNode v1 = vec.at(0);
		PNode p2 = v1->getNextSiblingNode();
		if (p2) {
			cout << p2->getNodeName() << endl;
		}else {
			cout << "无下个兄弟结点" << endl;
		}
		wcout.imbue(locale(".936"));
		wstring wstrTmp;
		wcout << v1->getNextSiblingNode()->getTextContent(wstrTmp) << endl;
		//或
		wcout << v1->getNextSiblingNode()->getTextContent(wstrTmp, "utf-8") << endl;
	}//End if

	return 0;
}