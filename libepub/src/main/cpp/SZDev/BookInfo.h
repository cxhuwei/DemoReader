#ifndef _BOOKINFO_H
#define _BOOKINFO_H

#include <string>
#include "StringConver.h"

class CBookInfo
{
public:
	CBookInfo(void);
	~CBookInfo(void);
public:
	void PutTitle(const char* title);
	myWcharT* GetTitle();
	void PutCreator(const char* creator);
	myWcharT* GetCreator();
	void PutSubject(const char* subject);
	myWcharT* GetSubject();
	void Putdescription(const char* description);
	myWcharT* Getdescription();
	void Putcontributor(const char* contributor);
	myWcharT* Getcontributor();
	void Putdate(const char* date);
	myWcharT* Getdate();
	void Puttype(const char* type);
	myWcharT* Gettype();
	void Putformat(const char* format);
	myWcharT* Getformat();
	void Putidentifier(const char* identifier);
	myWcharT* Getidentifier();
	void Putsource(const char* source);
	myWcharT* Getsource();
	void Putlanguage(const char* language);
	myWcharT* Getlanguage();
	void Putrelation(const char* relation);
	myWcharT* Getrelation();
	void Putcoverage(const char* coverage);
	myWcharT* Getcoverage();
	void Putrights(const char* rights);
	myWcharT* Getrights();
	void PutEncoding(int encodeType);
	int GetEncodeing();
	void PutPublisher(const char* publisher);
	myWcharT* GetPublisher();
	void ConverEncode(const char* input,myWcharT*& out);

private:
	myWcharT* m_title;
	myWcharT* m_creator;
	myWcharT* m_subject;
	myWcharT* m_description;
	myWcharT* m_contributor;
	myWcharT* m_date;
	myWcharT* m_type;
	myWcharT* m_format;
	myWcharT* m_identifier;
	myWcharT* m_source;
	myWcharT* m_language;
	myWcharT* m_relation;
	myWcharT* m_coverage;
	myWcharT* m_rights;
	myWcharT* m_publisher;
	
	CStringConver m_conver;
	int encode;
public:
	void Clear(void);
};
#endif