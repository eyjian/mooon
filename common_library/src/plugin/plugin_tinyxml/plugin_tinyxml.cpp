/**
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: JianYi, eyjian@qq.com
 */
#include <regex.h>
#include <strings.h>
#include <sys/types.h>
#include "tinyxml.h"
#include "util/token_list.h"
#include "util/string_util.h"
#include "plugin/plugin_tinyxml/plugin_tinyxml.h"
LIBPLUGIN_NAMESPACE_BEGIN

class CConfigReader;

class CConfigFile: public sys::IConfigFile
{
private:
	virtual bool open(const std::string& xmlfile);
    virtual void close();
    virtual sys::IConfigReader* get_config_reader();
    virtual void free_config_reader(sys::IConfigReader* config_reader);

    virtual int get_error_row() const;
    virtual int get_error_col() const;
    virtual std::string get_error_message() const;

private:
	TiXmlDocument _document;
};

class CConfigReader: public sys::IConfigReader
{
public:
    CConfigReader(TiXmlElement* root);

private:
    virtual bool path_exist(const std::string& path);
    virtual bool name_exist(const std::string& path, const std::string& name);

    virtual bool get_bool_value(const std::string& path, const std::string& name, bool& value);
    virtual bool get_string_value(const std::string& path, const std::string& name, std::string& value);
    virtual bool get_int16_value(const std::string& path, const std::string& name, int16_t& value);
    virtual bool get_int32_value(const std::string& path, const std::string& name, int32_t& value);
    virtual bool get_int64_value(const std::string& path, const std::string& name, int64_t& value);
    virtual bool get_uint16_value(const std::string& path, const std::string& name, uint16_t& value);
    virtual bool get_uint32_value(const std::string& path, const std::string& name, uint32_t& value);
    virtual bool get_uint64_value(const std::string&path, const std::string& name, uint64_t& value);

    virtual bool get_text(const std::string& path, std::string& text);

    virtual bool get_string_values(const std::string& path, const std::string& name, std::vector<std::string>& values);
    virtual bool get_int16_values(const std::string& path, const std::string& name, std::vector<int16_t>& values);
    virtual bool get_int32_values(const std::string& path, const std::string& name, std::vector<int32_t>& values);
    virtual bool get_int64_values(const std::string& path, const std::string& name, std::vector<int64_t>& values);
    virtual bool get_uint16_values(const std::string& path, const std::string& name, std::vector<uint16_t>& values);
    virtual bool get_uint32_values(const std::string& path, const std::string& name, std::vector<uint32_t>& values);
    virtual bool get_uint64_values(const std::string&path, const std::string& name, std::vector<uint64_t>& values);
    
    virtual bool get_sub_config(const std::string& path, std::vector<sys::IConfigReader*>& sub_config_array);

private:
    TiXmlElement* select_element(const std::string& path);
    TiXmlAttribute* select_attribute(const std::string& path, const std::string& name);

    typedef std::vector<TiXmlElement*> TTiXmlElementArray;
    bool select_elements(const std::string& path, TTiXmlElementArray& element_array);
    bool get_value(TiXmlElement* element, const std::string& name, std::string& value);

private:
    TiXmlElement* _root;
};

//////////////////////////////////////////////////////////////////////////
// CConfigFile

bool CConfigFile::open(const std::string& xmlfile)
{    
	return _document.LoadFile(xmlfile.c_str());
}

void CConfigFile::close()
{    
}

sys::IConfigReader* CConfigFile::get_config_reader()
{
    return new CConfigReader(_document.RootElement());
}

void CConfigFile::free_config_reader(sys::IConfigReader* config_reader)
{
    delete (CConfigReader *)config_reader;
}

int CConfigFile::get_error_row() const
{
    return _document.ErrorRow();
}

int CConfigFile::get_error_col() const
{
    return _document.ErrorCol();
}

std::string CConfigFile::get_error_message() const
{
    return _document.ErrorDesc();
}

//////////////////////////////////////////////////////////////////////////
// CConfigReader

CConfigReader::CConfigReader(TiXmlElement* root)
    :_root(root)
{
}

TiXmlElement* CConfigReader::select_element(const std::string& path)
{
	util::CTokenList::TTokenList token_list;
	util::CTokenList::parse(token_list, path, "/");
	
	// 用以支持：get_string_value("/", "name", value);
	if (token_list.empty()) return _root;
	
	TiXmlElement* element = _root;	
	while (!token_list.empty())
	{
		std::string token = token_list.front();
		token_list.pop_front();

		while (true)
		{
			if (NULL == element) return NULL;

			const char* element_name = element->Value();
			if (NULL == element_name) return NULL;			

			if (0 == strcasecmp(token.c_str(), element_name))
			{
				if (token_list.empty()) return element;
				
				element = element->FirstChildElement();
				break;
			}
			else
			{
				element = element->NextSiblingElement();
			}
		}
	}

	return NULL;
}

TiXmlAttribute* CConfigReader::select_attribute(const std::string& path, const std::string& name)
{
	TiXmlElement* element = this->select_element(path);
	if (NULL == element) return NULL;
	
	TiXmlAttribute* attribute = element->FirstAttribute();
	while (attribute != NULL)
	{		
		const char* name_tmp = attribute->Name();
		
		if (NULL == name_tmp) return NULL;
        if (0 == strcasecmp(name.c_str(), name_tmp)) return attribute;
		
		attribute = attribute->Next();
	}

	return NULL;
}

bool CConfigReader::select_elements(const std::string& path, TTiXmlElementArray& element_array)
{	
    regex_t reg;
    size_t i = 0;
    std::string token;
    const char* element_name = NULL;
    
    TiXmlElement* element = NULL;
    TTiXmlElementArray child_element_array;
	TTiXmlElementArray parent_element_array;
    util::CTokenList::TTokenList token_list;

    util::CTokenList::parse(token_list, path, "/");
    parent_element_array.push_back(_root);

	while (!token_list.empty())
	{
		token = token_list.front();
		token_list.pop_front();
        
        // returns zero for a successful compilation or an error code for failure
        int errcode = regcomp(&reg, token.c_str(), REG_EXTENDED);
        if (errcode != 0)
        {
            return false;
        }
            
        child_element_array.clear();
        for (i=0; i<parent_element_array.size(); ++i)
        {
			element_name =  parent_element_array[i]->Value();
			if (NULL == element_name) continue;			

            // returns zero for a successful match or REG_NOMATCH for failure
			if (0 == regexec(&reg, element_name, 0, NULL, 0))
            {
                if (token_list.empty())
                {     
                    element_array.push_back(parent_element_array[i]);
                }
                else
                {
                    element = parent_element_array[i]->FirstChildElement();
                    while (element != NULL)
                    {
                        child_element_array.push_back(element);
                        element = element->NextSiblingElement();
                    }
                }
            }        
        }

        if (token_list.empty()) break;

        parent_element_array.clear();
        for (i=0; i<child_element_array.size(); ++i)
        {
            parent_element_array.push_back(child_element_array[i]);
        }

        regfree(&reg);
	}

	return !element_array.empty();
}

bool CConfigReader::path_exist(const std::string& path)
{
	TiXmlElement* element = this->select_element(path);
	return (NULL == element)? false: true;
}

bool CConfigReader::name_exist(const std::string& path, const std::string& name)
{
	TiXmlAttribute* attribute = this->select_attribute(path, name);
	return (NULL == attribute)? false: true;
}

//////////////////////////////////////////////////////////////////////////

bool CConfigReader::get_bool_value(const std::string& path, const std::string& name, bool& value)
{
    std::string string_value;
    if (!CConfigReader::get_string_value(path, name, string_value)) return false;

    if (0 == strcasecmp(string_value.c_str(), "true"))
    {
        value = true;
        return true;
    }
    if (0 == strcasecmp(string_value.c_str(), "false"))
    {
        value = false;
        return true;
    }

    return false;
}

bool CConfigReader::get_string_value(const std::string& path, const std::string& name, std::string& value)
{
	TiXmlAttribute* attribute = this->select_attribute(path, name);
	if (NULL == attribute) return false;

	const char* value_tmp = attribute->Value();
	if (NULL == value_tmp) return false;

	value = value_tmp;
	return true;
}

bool CConfigReader::get_int16_value(const std::string& path, const std::string& name, int16_t& value)
{
    TiXmlAttribute* attribute = this->select_attribute(path, name);
	if (NULL == attribute) return false;

    const char* value_tmp = attribute->Value();
    if (NULL == value_tmp) return false;

    int16_t result = 0;
    if (!util::CStringUtil::string2int16(value_tmp, result)) return false;

    value = result;
    return true;
}

bool CConfigReader::get_int32_value(const std::string& path, const std::string& name, int32_t& value)
{
    TiXmlAttribute* attribute = this->select_attribute(path, name);
	if (NULL == attribute) return false;

    const char* value_tmp = attribute->Value();
    if (NULL == value_tmp) return false;

    int32_t result = 0;
    if (!util::CStringUtil::string2int32(value_tmp, result)) return false;

    value = result;
    return true;
}

bool CConfigReader::get_int64_value(const std::string& path, const std::string& name, int64_t& value)
{
    TiXmlAttribute* attribute = this->select_attribute(path, name);
	if (NULL == attribute) return false;

    const char* value_tmp = attribute->Value();
    if (NULL == value_tmp) return false;

    int64_t result = 0;
    if (!util::CStringUtil::string2int64(value_tmp, result)) return false;

    value = result;
    return true;
}

bool CConfigReader::get_uint16_value(const std::string& path, const std::string& name, uint16_t& value)
{
    TiXmlAttribute* attribute = this->select_attribute(path, name);
	if (NULL == attribute) return false;

    const char* value_tmp = attribute->Value();
    if (NULL == value_tmp) return false;

    uint16_t result = 0;
    if (!util::CStringUtil::string2uint16(value_tmp, result)) return false;

    value = result;
    return true;
}

bool CConfigReader::get_uint32_value(const std::string& path, const std::string& name, uint32_t& value)
{
    TiXmlAttribute* attribute = this->select_attribute(path, name);
	if (NULL == attribute) return false;

    const char* value_tmp = attribute->Value();
    if (NULL == value_tmp) return false;

    uint32_t result = 0;
    if (!util::CStringUtil::string2uint32(value_tmp, result)) return false;

    value = result;
    return true;
}

bool CConfigReader::get_uint64_value(const std::string& path, const std::string& name, uint64_t& value)
{
    TiXmlAttribute* attribute = this->select_attribute(path, name);
	if (NULL == attribute) return false;
	
    const char* value_tmp = attribute->Value();
    if (NULL == value_tmp) return false;

    uint64_t result = 0;
    if (!util::CStringUtil::string2uint64(value_tmp, result)) return false;

    value = result;
    return true;
}

bool CConfigReader::get_text(const std::string& path, std::string& text)
{
    TiXmlElement* elem = this->select_element(path);
    if (NULL == elem) return false;

    const char* text_tmp = elem->GetText();
    if (NULL == text_tmp) return false;

    text = text_tmp;
    return true;
}

bool CConfigReader::get_string_values(const std::string& path, const std::string& name, std::vector<std::string>& values)
{
    TTiXmlElementArray element_array;
    if (!this->select_elements(path, element_array)) return false;
    
    for (TTiXmlElementArray::size_type i=0; i<element_array.size(); ++i)
    {
        std::string value;
        if (this->get_value(element_array[i], name, value))
            values.push_back(value);
    }

    return !values.empty();
}

bool CConfigReader::get_int16_values(const std::string& path, const std::string& name, std::vector<int16_t>& values)
{
    TTiXmlElementArray element_array;
    if (!this->select_elements(path, element_array)) return false;
    
    for (TTiXmlElementArray::size_type i=0; i<element_array.size(); ++i)
    {
        std::string value;
        int16_t value_int16 = 0;
        if (this->get_value(element_array[i], name, value))
        {
            if (!util::CStringUtil::string2int16(value.c_str(), value_int16)) return false;
            values.push_back(value_int16);
        }
    }

    return !values.empty();
}

bool CConfigReader::get_int32_values(const std::string& path, const std::string& name, std::vector<int32_t>& values)
{
    TTiXmlElementArray element_array;
    if (!this->select_elements(path, element_array)) return false;
    
    for (TTiXmlElementArray::size_type i=0; i<element_array.size(); ++i)
    {
        std::string value;
        int32_t value_int = 0;
        if (this->get_value(element_array[i], name, value))
        {
            if (!util::CStringUtil::string2int32(value.c_str(), value_int)) return false;
            values.push_back(value_int);
        }
    }

    return !values.empty();
}

bool CConfigReader::get_int64_values(const std::string& path, const std::string& name, std::vector<int64_t>& values)
{
    TTiXmlElementArray element_array;
    if (!this->select_elements(path, element_array)) return false;
    
    for (TTiXmlElementArray::size_type i=0; i<element_array.size(); ++i)
    {
        std::string value;
        int64_t value_int = 0;
        if (this->get_value(element_array[i], name, value))
        {
            if (!util::CStringUtil::string2int64(value.c_str(), value_int)) return false;
            values.push_back(value_int);
        }
    }

    return !values.empty();
}

bool CConfigReader::get_uint16_values(const std::string& path, const std::string& name, std::vector<uint16_t>& values)
{
    TTiXmlElementArray element_array;
    if (!this->select_elements(path, element_array)) return false;
    
    for (TTiXmlElementArray::size_type i=0; i<element_array.size(); ++i)
    {
        std::string value;
        uint16_t value_int = 0;
        if (this->get_value(element_array[i], name, value))
        {
            if (!util::CStringUtil::string2uint16(value.c_str(), value_int)) return false;
            values.push_back(value_int);
        }
    }

    return !values.empty();
}

bool CConfigReader::get_uint32_values(const std::string& path, const std::string& name, std::vector<uint32_t>& values)
{
    TTiXmlElementArray element_array;
    if (!this->select_elements(path, element_array)) return false;
    
    for (TTiXmlElementArray::size_type i=0; i<element_array.size(); ++i)
    {
        std::string value;
        uint32_t value_int = 0;
        if (this->get_value(element_array[i], name, value))
        {
            if (!util::CStringUtil::string2uint32(value.c_str(), value_int)) return false;
            values.push_back(value_int);
        }
    }

    return !values.empty();
}

bool CConfigReader::get_uint64_values(const std::string&path, const std::string& name, std::vector<uint64_t>& values)
{
    TTiXmlElementArray element_array;
    if (!this->select_elements(path, element_array)) return false;
    
    for (TTiXmlElementArray::size_type i=0; i<element_array.size(); ++i)
    {
        std::string value;
        uint64_t value_int = 0;
        if (this->get_value(element_array[i], name, value))
        {
            if (!util::CStringUtil::string2uint64(value.c_str(), value_int)) return false;
            values.push_back(value_int);
        }
    }

    return !values.empty();
}

bool CConfigReader::get_sub_config(const std::string& path, std::vector<sys::IConfigReader*>& sub_config_array)
{
    TTiXmlElementArray element_array;
    if (!this->select_elements(path, element_array)) return false;

    for (size_t i=0; i<element_array.size(); ++i)
    {
        sub_config_array.push_back(new CConfigReader(element_array[i]));
    }

    return true;
}

bool CConfigReader::get_value(TiXmlElement* element, const std::string& name, std::string& value)
{
    TiXmlAttribute* attribute = element->FirstAttribute();
    while (attribute != NULL)
    {
        const char* name_tmp = attribute->Name();
        if (name_tmp != NULL)
        {
            if (name == name_tmp)
            {
                const char* value_tmp = attribute->Value();
                if (NULL == value_tmp) return false;

                value = value_tmp;
                return true;
            }
        }        

        attribute = attribute->Next();
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////
// 出口函数

sys::IConfigFile* create_config_file()
{
	return new CConfigFile;
}

void destroy_config_file(sys::IConfigFile* config_file)
{
	delete (CConfigFile*)config_file;
}

LIBPLUGIN_NAMESPACE_END
