#include "MiniXML/MiniXml.h"

#include <fstream>
#include <sstream>
#include <cctype>

namespace
{
    void SkipWhitespace(const std::string& s, size_t& i)
    {
        while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i])))
            ++i;
    }

    bool StartsWith(const std::string& s, size_t i, const char* lit)
    {
        size_t j = 0;
        while (lit[j] != '\0')
        {
            if (i + j >= s.size() || s[i + j] != lit[j])
                return false;
            ++j;
        }
        return true;
    }

    std::string ReadName(const std::string& s, size_t& i)
    {
        size_t start = i;
        while (i < s.size())
        {
            char c = s[i];
            if (std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-' || c == ':')
                ++i;
            else
                break;
        }
        return s.substr(start, i - start);
    }

    std::string ReadUntil(const std::string& s, size_t& i, char endChar)
    {
        size_t start = i;
        while (i < s.size() && s[i] != endChar)
            ++i;
        return s.substr(start, i - start);
    }

    bool ReadQuoted(const std::string& s, size_t& i, std::string& out)
    {
        if (i >= s.size())
            return false;
        char quote = s[i];
        if (quote != '"' && quote != '\'')
            return false;
        ++i;
        size_t start = i;
        while (i < s.size() && s[i] != quote)
            ++i;
        if (i >= s.size())
            return false;
        out = s.substr(start, i - start);
        ++i;
        return true;
    }

    bool ParseAttributes(const std::string& s, size_t& i, std::vector<XmlAttribute>& attrs)
    {
        while (true)
        {
            SkipWhitespace(s, i);
            if (i >= s.size())
                return false;
            char c = s[i];
            if (c == '/' || c == '>')
                break;
            std::string name = ReadName(s, i);
            if (name.empty())
                return false;
            SkipWhitespace(s, i);
            if (i >= s.size() || s[i] != '=')
                return false;
            ++i;
            SkipWhitespace(s, i);
            std::string value;
            if (!ReadQuoted(s, i, value))
                return false;
            XmlAttribute a;
            a.name = name;
            a.value = value;
            attrs.push_back(a);
        }
        return true;
    }

    bool ParseNodeInner(const std::string& s, size_t& i, XmlNode& outNode);

    bool ParseNode(const std::string& s, size_t& i, XmlNode& outNode)
    {
        if (i >= s.size() || s[i] != '<')
            return false;
        ++i;
        if (i < s.size() && s[i] == '/')
            return false;

        if (StartsWith(s, i, "?"))
        {
            ++i;
            size_t endPos = s.find("?>", i);
            if (endPos == std::string::npos)
                return false;
            i = endPos + 2;
            SkipWhitespace(s, i);
            return ParseNode(s, i, outNode);
        }

        if (StartsWith(s, i, "!--"))
        {
            i += 3;
            size_t endPos = s.find("-->", i);
            if (endPos == std::string::npos)
                return false;
            i = endPos + 3;
            SkipWhitespace(s, i);
            return ParseNode(s, i, outNode);
        }

        outNode.name = ReadName(s, i);
        if (outNode.name.empty())
            return false;

        SkipWhitespace(s, i);
        if (!ParseAttributes(s, i, outNode.attributes))
            return false;

        if (i >= s.size())
            return false;

        if (s[i] == '/')
        {
            ++i;
            if (i >= s.size() || s[i] != '>')
                return false;
            ++i;
            return true;
        }

        if (s[i] != '>')
            return false;
        ++i;

        return ParseNodeInner(s, i, outNode);
    }

    bool ParseNodeInner(const std::string& s, size_t& i, XmlNode& outNode)
    {
        std::string collectedText;

        while (i < s.size())
        {
            if (s[i] == '<')
            {
                if (StartsWith(s, i, "</"))
                {
                    i += 2;
                    std::string closeName = ReadName(s, i);
                    SkipWhitespace(s, i);
                    if (i >= s.size() || s[i] != '>')
                        return false;
                    ++i;
                    if (closeName != outNode.name)
                        return false;
                    if (!collectedText.empty())
                    {
                        size_t start = 0;
                        size_t end = collectedText.size();
                        while (start < end && std::isspace(static_cast<unsigned char>(collectedText[start])))
                            ++start;
                        while (end > start && std::isspace(static_cast<unsigned char>(collectedText[end - 1])))
                            --end;
                        if (end > start)
                            outNode.text = collectedText.substr(start, end - start);
                    }
                    return true;
                }
                else if (StartsWith(s, i, "!--"))
                {
                    i += 4;
                    size_t endPos = s.find("-->", i);
                    if (endPos == std::string::npos)
                        return false;
                    i = endPos + 3;
                    continue;
                }
                else if (StartsWith(s, i, "?"))
                {
                    ++i;
                    size_t endPos = s.find("?>", i);
                    if (endPos == std::string::npos)
                        return false;
                    i = endPos + 2;
                    continue;
                }
                else
                {
                    XmlNode child;
                    if (!ParseNode(s, i, child))
                        return false;
                    if (!collectedText.empty())
                    {
                        size_t start = 0;
                        size_t end = collectedText.size();
                        while (start < end && std::isspace(static_cast<unsigned char>(collectedText[start])))
                            ++start;
                        while (end > start && std::isspace(static_cast<unsigned char>(collectedText[end - 1])))
                            --end;
                        if (end > start && outNode.text.empty())
                            outNode.text = collectedText.substr(start, end - start);
                        collectedText.clear();
                    }
                    outNode.children.push_back(child);
                }
            }
            else
            {
                collectedText.push_back(s[i]);
                ++i;
            }
        }

        return false;
    }

    void SerializeNode(const XmlNode& node, std::string& out, int indentSpaces, int depth)
    {
        std::string indent(depth * indentSpaces, ' ');

        out += indent;
        out += "<";
        out += node.name;

        for (const auto& a : node.attributes)
        {
            out += " ";
            out += a.name;
            out += "=\"";
            out += a.value;
            out += "\"";
        }

        bool hasChildren = !node.children.empty();
        bool hasText = !node.text.empty();

        if (!hasChildren && !hasText)
        {
            out += "/>\n";
            return;
        }

        out += ">";

        if (hasText && !hasChildren)
        {
            out += node.text;
            out += "</";
            out += node.name;
            out += ">\n";
            return;
        }

        out += "\n";

        if (hasText)
        {
            std::string tIndent((depth + 1) * indentSpaces, ' ');
            out += tIndent;
            out += node.text;
            out += "\n";
        }

        for (const auto& c : node.children)
            SerializeNode(c, out, indentSpaces, depth + 1);

        out += indent;
        out += "</";
        out += node.name;
        out += ">\n";
    }
}

bool XmlParseString(const std::string& text, XmlNode& outRoot)
{
    size_t i = 0;
    SkipWhitespace(text, i);
    if (i >= text.size())
        return false;

    return ParseNode(text, i, outRoot);
}

std::string XmlToString(const XmlNode& root, int indentSpaces)
{
    std::string out;
    SerializeNode(root, out, indentSpaces, 0);
    return out;
}

bool XmlLoadFromFile(const std::string& path, XmlNode& outRoot)
{
    std::ifstream ifs(path);
    if (!ifs.is_open())
        return false;
    std::stringstream ss;
    ss << ifs.rdbuf();
    return XmlParseString(ss.str(), outRoot);
}

bool XmlSaveToFile(const std::string& path, const XmlNode& root)
{
    std::ofstream ofs(path);
    if (!ofs.is_open())
        return false;
    ofs << XmlToString(root, 2);
    return true;
}
