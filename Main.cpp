#include "interface.h"
#include "ifxml.h"
#include <iostream>

using namespace std;

int main() {
	
	CParseXML file("Ҫ�򿪵��ļ�.xml");
	file.exe();
	vector<PNode> vec = file.getNodeByAttributeAndValue("������","����ֵ");
	
	if (vec.empty() ) {
		cout << "�� " << endl;
	}else {
		wcout.imbue(locale(".936"));
		for ( auto& e:vec){
			cout << "��ǩ��:" << e->getNodeName() << endl;

			for (auto& ee : e->getAttributeVec()) {
				cout << "��������" << ee.first << " " << "����ֵ��" << ee.second << endl;
			}
			//��
			for ( auto& ee:e->getAttributeVecUTF8() ){			
				wcout << L"��������" << ee.first << " " << L"����ֵ��" <<ee.second << endl;
			}
		}
	}//End else

	if (vec.at(0)) {
		PNode v1 = vec.at(0);
		PNode p2 = v1->getNextSiblingNode();
		if (p2) {
			cout << p2->getNodeName() << endl;
		}else {
			cout << "���¸��ֵܽ��" << endl;
		}
		wcout.imbue(locale(".936"));
		wstring wstrTmp;
		wcout << v1->getNextSiblingNode()->getTextContent(wstrTmp) << endl;
		//��
		wcout << v1->getNextSiblingNode()->getTextContent(wstrTmp, "utf-8") << endl;
	}//End if

	return 0;
}