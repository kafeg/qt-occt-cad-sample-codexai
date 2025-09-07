#include "Feature.h"

#include <sstream>
#include <string>
#include <utility>

IMPLEMENT_STANDARD_RTTIEXT(Feature, DocumentItem)
// Very simple key=value; encoding for base fields and params; not robust JSON.
// Keys: name, suppressed, p_<keyIndex> for numeric params (int/double), s_<keyIndex> for string params
static std::string escape(const std::string& s)
{
  std::string out; out.reserve(s.size());
  for (char c : s) { if (c == '\\' || c == '=') out.push_back('\\'); out.push_back(c); }
  return out;
}

static std::string toString(const TCollection_AsciiString& s)
{
  return std::string(s.ToCString());
}

static TCollection_AsciiString toAscii(const std::string& s)
{
  return TCollection_AsciiString(s.c_str());
}

std::string Feature::serialize() const
{
  std::ostringstream os;
  os << "name=" << escape(toString(m_name)) << "\n";
  os << "suppressed=" << (m_suppressed ? 1 : 0) << "\n";
  os << "datum_related=" << (m_isDatumRelated ? 1 : 0) << "\n";
  for (const auto& kv : m_params)
  {
    const int keyIdx = static_cast<int>(kv.first);
    if (std::holds_alternative<int>(kv.second))
    {
      os << "p_" << keyIdx << '=' << std::get<int>(kv.second) << "\n";
    }
    else if (std::holds_alternative<double>(kv.second))
    {
      os << "p_" << keyIdx << '=' << std::get<double>(kv.second) << "\n";
    }
    else
    {
      os << "s_" << keyIdx << '=' << escape(toString(std::get<TCollection_AsciiString>(kv.second))) << "\n";
    }
  }
  return os.str();
}

void Feature::deserialize(const std::string& data)
{
  m_params.clear();
  std::string key, val;
  std::size_t pos = 0;
  while (pos < data.size())
  {
    std::size_t eol = data.find('\n', pos);
    std::string line = data.substr(pos, eol == std::string::npos ? std::string::npos : eol - pos);
    pos = (eol == std::string::npos) ? data.size() : eol + 1;
    if (line.empty()) continue;
    std::size_t eq = std::string::npos;
    bool esc = false;
    for (std::size_t i = 0; i < line.size(); ++i)
    {
      if (!esc && line[i] == '=') { eq = i; break; }
      esc = (!esc && line[i] == '\\');
    }
    if (eq == std::string::npos) continue;
    key = line.substr(0, eq);
    val.clear();
    esc = false;
    for (std::size_t i = eq + 1; i < line.size(); ++i)
    {
      char c = line[i];
      if (!esc && c == '\\') { esc = true; continue; }
      val.push_back(c); esc = false;
    }

    if (key == "name") m_name = toAscii(val);
    else if (key == "suppressed") m_suppressed = (val == "1");
    else if (key == "datum_related") m_isDatumRelated = (val == "1");
    else if (key.rfind("p_", 0) == 0)
    {
      int idx = std::stoi(key.substr(2));
      // try parse as double, then as int
      try {
        double d = std::stod(val);
        m_params[static_cast<ParamKey>(idx)] = d;
      } catch (...) {
        try { int i = std::stoi(val); m_params[static_cast<ParamKey>(idx)] = i; } catch (...) {}
      }
    }
    else if (key.rfind("s_", 0) == 0)
    {
      int idx = std::stoi(key.substr(2));
      m_params[static_cast<ParamKey>(idx)] = toAscii(val);
    }
  }
}

double Feature::paramAsDouble(const ParamMap& pm, ParamKey key, double defVal)
{
  auto it = pm.find(key);
  if (it == pm.end())
    return defVal;
  const ParamValue& v = it->second;
  if (std::holds_alternative<double>(v))
    return std::get<double>(v);
  if (std::holds_alternative<int>(v))
    return static_cast<double>(std::get<int>(v));
  return defVal;
}
