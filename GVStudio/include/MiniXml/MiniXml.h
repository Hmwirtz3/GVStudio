#pragma once

#include <string>
#include <vector>

struct XmlAttribute
{
    std::string name;
    std::string value;
};

struct XmlNode
{
    std::string name;
    std::string text;
    std::vector<XmlAttribute> attributes;
    std::vector<XmlNode> children;


};

bool XmlLoadFromFile(const std::string& path, XmlNode& outRoot);
bool XmlSaveToFile(const std::string& path, const XmlNode& root);

std::string XmlToString(const XmlNode& root, int indentSpaces = 2);
bool XmlParseString(const std::string& text, XmlNode& outRoot);
