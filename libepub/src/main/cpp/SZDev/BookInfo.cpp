#include "StdAfx.h"
#include "BookInfo.h"


CBookInfo::CBookInfo(void)
{
	m_title=m_creator=m_subject=m_description=m_contributor
		= m_date=m_type= m_format= m_identifier=m_source
		= m_language=m_relation=m_coverage =m_rights = m_publisher = NULL;
}

CBookInfo::~CBookInfo(void)
{
	Clear();
// 	if (m_title != NULL)
// 	{
// 		delete[] m_title;
// 	}
// 	if (m_creator != NULL)
// 	{
// 		delete[] m_creator;
// 	}
// 	if (m_subject != NULL)
// 	{
// 		delete[] m_subject;
// 	}
// 	if (m_description != NULL)
// 	{
// 		delete[] m_description;
// 	}
// 	if (m_contributor != NULL)
// 	{
// 		delete[] m_contributor;
// 	}
// 	if (m_date != NULL)
// 	{
// 		delete[] m_date;
// 	}
// 	if (m_type != NULL)
// 	{
// 		delete[] m_type;
// 	}
// 	if (m_format != NULL)
// 	{
// 		delete[] m_format;
// 	}
// 	if (m_identifier != NULL)
// 	{
// 		delete[] m_identifier;
// 	}
// 	if (m_source != NULL)
// 	{
// 		delete[] m_source;
// 	}
// 	if (m_language != NULL)
// 	{
// 		delete[] m_language;
// 	}
// 	if (m_relation != NULL)
// 	{
// 		delete[] m_relation;
// 	}
// 	if (m_coverage != NULL)
// 	{
// 		delete[] m_coverage;
// 	}
// 	if (m_rights != NULL)
// 	{
// 		delete[] m_rights;
// 	}
}
void CBookInfo::PutTitle(const char* title )
{
	if (title != NULL)
	{
		ConverEncode(title,m_title);
	}
}

myWcharT* CBookInfo::GetTitle()
{
	return m_title;
}

void CBookInfo::PutCreator(const char* creator )
{
	if (creator != NULL)
	{
		ConverEncode(creator,m_creator);
	}
}

myWcharT* CBookInfo::GetCreator()
{
	return m_creator;
}

void CBookInfo::PutSubject(const char* subject )
{
	if (subject != NULL )
	{
		ConverEncode(subject,m_subject);
	}
}

myWcharT* CBookInfo::GetSubject()
{
	return m_subject;
}

void CBookInfo::Putdescription(const char* description )
{
	if (description != NULL )
	{
		ConverEncode(description,m_description);
	}
}

myWcharT* CBookInfo::Getdescription()
{
	return m_description;
}

void CBookInfo::Putcontributor(const char* contributor )
{
	if (contributor != NULL)
	{
		ConverEncode(contributor,m_contributor);
	}
}

myWcharT* CBookInfo::Getcontributor()
{
	return m_contributor;
}

void CBookInfo::Putdate(const char* date )
{	
	if (date != NULL)
	{
		ConverEncode(date,m_date);
	}
}	

myWcharT* CBookInfo::Getdate()
{
	return m_date;
}

void CBookInfo::Puttype(const char* type )
{
	if (type != NULL)
	{
		ConverEncode(type,m_type);
	}
}

myWcharT* CBookInfo::Gettype()
{
	return m_type;
}

void CBookInfo::Putformat(const char* format )
{
	if (format != NULL)
	{
		ConverEncode(format,m_format);
	}
}

myWcharT* CBookInfo::Getformat()
{
	return m_format;
}

void CBookInfo::Putidentifier(const char* identifier )
{
	if (identifier != NULL )
	{
		ConverEncode(identifier,m_identifier);
	}
}

myWcharT* CBookInfo::Getidentifier()
{
	return m_identifier;
}

void CBookInfo::Putsource(const char* source )
{
	if (source != NULL )
	{
		ConverEncode(source,m_source);
	}
}

myWcharT* CBookInfo::Getsource()
{
	return m_source;
}

void CBookInfo::Putlanguage(const char* language )
{
	if (language != NULL)
	{
		ConverEncode(language,m_language);
	}
}

myWcharT* CBookInfo::Getlanguage()
{
	return m_language;
}

void CBookInfo::Putrelation(const char* relation )
{
	if (relation != NULL)
	{
		ConverEncode(relation,m_relation);
	}
}

myWcharT* CBookInfo::Getrelation()
{
	return m_relation;
}

void CBookInfo::Putcoverage(const char* coverage )
{
	if (coverage != NULL)
	{
		ConverEncode(coverage,m_coverage);
	}
}

myWcharT* CBookInfo::Getcoverage()
{
	return m_coverage;
}

void CBookInfo::Putrights(const char* rights )
{
	if (rights != NULL)
	{
		ConverEncode(rights,m_rights);
	}
}

myWcharT* CBookInfo::Getrights()
{
	return m_rights;
}

void CBookInfo::PutEncoding( int encodeType )
{
	if (encodeType >= 0 && encodeType < 4)
	{
		encode = encodeType;
	}
	else
		encode = 1;
}

int CBookInfo::GetEncodeing()
{
	return encode;
}

void CBookInfo::ConverEncode( const char* input,myWcharT*& out )
{
	if (out != NULL)
	{
		delete[] out;
		out = NULL;
	}
	int length = strlen(input);
	out = new myWcharT[length + 1];
	memset(out,0,length*sizeof(myWcharT)+sizeof(myWcharT));
	switch(encode)
	{
	case 0:
		m_conver.GbkToUnicodeBig((char*)input,length,out,length*sizeof(myWcharT));
		break;
	case 1:
		m_conver.Utf8ToUnicodeBig((char*)input,length,out,length*sizeof(myWcharT));
		break;
	case 2:
		m_conver.Gb18030ToUnicodeBig((char*)input,length,out,length*sizeof(myWcharT));
		break;
	default:
		memcpy(out,input,length);
		break;
	}
}

void CBookInfo::Clear(void)
{
	if ( m_title)
	{
		delete[] m_title;
		m_title = NULL;
	}
	if ( m_creator)
	{
		delete[] m_creator;
		m_creator = NULL;
	}
	if ( m_subject)
	{
		delete[] m_subject;
		m_subject = NULL;
	}
	if ( m_description)
	{
		delete[] m_description;
		m_description = NULL;
	}
	if ( m_contributor)
	{
		delete[] m_contributor;
		m_contributor = NULL;
	}
	if ( m_date)
	{
		delete[] m_date;
		m_date = NULL;
	}
	if ( m_type)
	{
		delete[] m_type;
		m_type = NULL;
	}
	if ( m_format)
	{
		delete[] m_format;
		m_format = NULL;
	}
	if ( m_identifier)
	{
		delete[] m_identifier;
		m_identifier = NULL;
	}
	if ( m_source)
	{
		delete[] m_source;
		m_source = NULL;
	}
	if ( m_language)
	{
		delete[] m_language;
		m_language = NULL;
	}
	if ( m_relation)
	{
		delete[] m_relation;
		m_relation = NULL;
	}
	if ( m_coverage)
	{
		delete[] m_coverage;
		m_coverage = NULL;
	}
	if ( m_rights)
	{
		delete[] m_rights;
		m_rights = NULL;
	}
}

void CBookInfo::PutPublisher( const char* publisher )
{
	if (publisher != NULL)
	{
		ConverEncode(publisher,m_publisher);
	}
}

myWcharT* CBookInfo::GetPublisher()
{
	return m_publisher;
}
