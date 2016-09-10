#pragma once

#include "RapidXML/rapidxml_utils.hpp"
#include "SimpleMD5.h"
#include <unordered_map>

typedef rapidxml::file<>				RapidXML_File;
typedef rapidxml::xml_document<>		RapidXML_Doc;
typedef rapidxml::xml_node<>			RapidXML_Node;

class XMLWrapper
{
public:
	static XMLWrapper& GetInstance() { static XMLWrapper INSTANCE; return INSTANCE; }

	const RapidXML_Doc* LoadXMLFile( const char* filename )
	{
		RapidXML_File* newFile = nullptr;
		auto hash = md5( std::string( filename ) );
		
		XMLListType::const_iterator findIter = m_LoadedXMLList.find( hash );
		if (findIter != m_LoadedXMLList.end()) newFile = (*findIter).second;
		else newFile = new RapidXML_File( filename );

		auto newDoc = new RapidXML_Doc;
		newDoc->parse<0>( newFile->data() );
		
		m_LoadedXMLList[hash] = newFile;
		return newDoc;
	}

	bool RemoveXMLFile( const char* filename )
	{
		auto hash = md5( std::string( filename ) );
		XMLListType::const_iterator findIter = m_LoadedXMLList.find( hash );
		if (findIter != m_LoadedXMLList.end())
		{
			delete (*findIter).second;
			m_LoadedXMLList.erase(findIter);
			return true;
		}

		return false;
	}

private:
	XMLWrapper()	{}
	~XMLWrapper()	{}

	typedef std::unordered_map< std::string, RapidXML_File* > XMLListType;
	XMLListType m_LoadedXMLList;
};

//  Instance to be utilized by anyone including this header
XMLWrapper& xmlWrapper = XMLWrapper::GetInstance();