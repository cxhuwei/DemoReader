#ifndef _BOOKSPINE_H
#define _BOOKSPINE_H

#include <vector>
#include <string>
using namespace std;

class CBookSpine
{
public:
	CBookSpine(void);
	~CBookSpine(void);
	void PushBackSpine(string item);
	string ReadItem(int index);
	vector<string> GetVSpine();
	void Clear();
	void DeleteItem(int index);
private:
	vector<string> m_vSpine;
};
#endif