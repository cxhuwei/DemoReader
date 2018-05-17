#include "StdAfx.h"
#include "BookSpine.h"

CBookSpine::CBookSpine(void)
{
}

CBookSpine::~CBookSpine(void)
{
	Clear();
}

void CBookSpine::PushBackSpine(string item )
{
	if (!item.empty())
	{
		m_vSpine.push_back(item);
	}
}

string CBookSpine::ReadItem( int index )
{
	if (index > 0 && index <= m_vSpine.size())
	{
		return m_vSpine[index - 1];
	}
	return NULL;
	
}

vector<string> CBookSpine::GetVSpine()
{
	return m_vSpine;
}

void CBookSpine::Clear()
{
// 	for (int i = 0; i< m_vSpine.size(); i++)
// 	{
// 		m_vSpine[i]
// 	}
	m_vSpine.clear();
}

void CBookSpine::DeleteItem(int index)
{
	if (index > 0 && index <= m_vSpine.size())
	{
		m_vSpine.erase(m_vSpine.begin()+index);
	}
}
