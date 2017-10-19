/* -*-c++-*- IFC++ www.ifcquery.com
*
MIT License

Copyright (c) 2017 Fabian Gerold

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <iomanip>
#include <locale.h>
#include <boost/fusion/container/map.hpp>
#include "ifcpp/model/IfcPPBasicTypes.h"
#include "ifcpp/model/IfcPPObject.h"
#include "ifcpp/model/IfcPPModel.h"
#include "ifcpp/writer/WriterUtil.h"
#include "ifcpp/IFC4/include/IfcProduct.h"
#include "ifcpp/IFC4/include/IfcProject.h"
#include "ifcpp/writer/IfcPPWriterSTEP.h"

IfcPPWriterSTEP::IfcPPWriterSTEP()
{
}
IfcPPWriterSTEP::~IfcPPWriterSTEP()
{
}

void IfcPPWriterSTEP::writeModelToStream( std::stringstream& stream, shared_ptr<IfcPPModel> model )
{
	std::string current_numeric_locale(setlocale(LC_NUMERIC, nullptr));
	setlocale(LC_NUMERIC,"C");
	
	const std::wstring& file_header_wstr = model->getFileHeader();
	std::string file_header_str = encodeStepString( file_header_wstr );
	stream << file_header_str.c_str();
	stream << "DATA;\n";
	stream << std::setprecision( 15 );
	stream << std::setiosflags( std::ios::showpoint );
	stream << std::fixed;
	const std::map<int,shared_ptr<IfcPPEntity> >& map = model->getMapIfcEntities();
	std::map<int, shared_ptr<IfcPPEntity> > map_ordered( map.begin(), map.end() );
	size_t i = 0;
	double last_progress = 0.0;
	double num_objects = double(map_ordered.size());
	for( auto it=map_ordered.begin(); it!=map_ordered.end(); ++it )
	{
		shared_ptr<IfcPPEntity> obj = it->second;

		if( obj.use_count() < 2 )
		{
			// entity is referenced only in model map, not by other entities
			if( !dynamic_pointer_cast<IfcProduct>(obj) && !dynamic_pointer_cast<IfcProject>(obj) )
			{
				continue;
			}
		}
		obj->getStepLine( stream );
		stream << std::endl;

		if( i % 10 == 0 )
		{
			double progress = double( i ) / num_objects;
			if( progress - last_progress > 0.03 )
			{
				progressValueCallback( progress, "write" );
				last_progress = progress;
			}
		}
		++i;
	}

	stream << "ENDSEC;\nEND-ISO-10303-21;\n";

	setlocale(LC_NUMERIC, current_numeric_locale.c_str());
}
